/*
 * Copyright (c) 2014 Graham Edgecombe <graham@grahamedgecombe.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _GNU_SOURCE /* for asprintf() */
#include <ldap.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * The LDAP filter used to find an entry which is a POSIX account with the
 * ldapPublicKey class and given UID.
 */
#define LPK_FILTER \
  "(&" \
  "(objectClass=posixAccount)" \
  "(objectClass=ldapPublicKey)" \
  "(uid=%s)" \
  ")"

/*
 * The name of the attribute containing an SSH public key.
 */
#define LPK_ATTRIBUTE "sshPublicKey"

/*
 * Escape a string used in an LDAP filter. Returns 0 upon failure or a pointer
 * to the escaped string upon success. The returned string must be freed by the
 * caller with ber_memfree().
 */
static char *lpk_escape(char *in)
{
  BerValue in_ber = {
    .bv_len = strlen(in),
    .bv_val = in
  };

  BerValue out_ber;
  if (ldap_bv2escaped_filter_value(&in_ber, &out_ber) != 0)
  {
    ber_memfree(out_ber.bv_val);
    return 0;
  }

  return out_ber.bv_val;
}

int main(int argc, char **argv)
{
  int ret = 0; /* the return value of main() */

  /* Print usage help. */
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <username>\n", argc ? argv[0] : "ssh-ldap-publickey");
    return 1;
  }

  /* Escape UID and create the LDAP filter */
  char *uid = lpk_escape(argv[1]);
  if (!uid)
  {
    fprintf(stderr, "lpk_escape failed\n");
    return 1;
  }

  char *filter;
  if (asprintf(&filter, LPK_FILTER, uid) == -1)
  {
    fprintf(stderr, "asprintf failed\n");
    ret = 1;
    goto free_uid;
  }

  /* Create the LDAP struct and search for entries matching the filter. */
  LDAP *ldap;
  int err;
  if ((err = ldap_create(&ldap)) != LDAP_SUCCESS)
  {
    fprintf(stderr, "ldap_create: %s\n", ldap_err2string(err));
    ret = 1;
    goto free_filter;
  }

  LDAPMessage *result;
  if ((err = ldap_search_ext_s(ldap, 0, LDAP_SCOPE_SUBTREE, filter,
    0, 0, 0, 0, 0, LDAP_NO_LIMIT, &result)) != LDAP_SUCCESS)
  {
    fprintf(stderr, "ldap_search_ext_s: %s\n", ldap_err2string(err));
    ret = 1;
    goto free_ldap;
  }

  /* Iterate through each result. */
  for (LDAPMessage *entry = ldap_first_entry(ldap, result); entry;
    entry = ldap_next_entry(ldap, entry))
  {
    /* Fetch all the sshPublicKey attributes. */
    BerValue **keys = ldap_get_values_len(ldap, entry, LPK_ATTRIBUTE);
    if (!keys)
    {
      fprintf(stderr, "ldap_get_values_len failed\n");
      ret = 1;
      goto free_msg;
    }

    /* Write each sshPublicKey to stdout. */
    for (BerValue **it = keys, *key = *it++; key; key = *it++)
    {
      fwrite(key->bv_val, sizeof(char), key->bv_len, stdout);
      putchar('\n');
    }

    ldap_value_free_len(keys);
  }

  /* Tidy up. */
free_msg:
  ldap_msgfree(result);
free_ldap:
  ldap_destroy(ldap);
free_filter:
  free(filter);
free_uid:
  ber_memfree(uid);

  return ret;
}
