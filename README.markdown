ssh-ldap-publickey
==================

A program for looking up SSH public keys with LDAP. Inspired by, and (mostly)
compatible with, Andrii Grytsenko's [openssh-ldap-publickey][1] Perl script.

It can be used with OpenSSH to fetch the authorized keys for a user from an LDAP
directory, instead of from the user's `~/.ssh/authorized_keys` file.

Prerequisites
-------------

* C99 compiler
* libldap and liblber

Building
--------

Run `make` to build the project.

Installation
------------

Run `make install` to install the project to `/usr/local`. You can set the
`PREFIX` environment variable to change this to something else e.g. `/usr`. The
`DESTDIR` variable is also supported.

Usage
-----

Ensure your system's `ldap.conf` file is configured correctly, then add the
following directives to your `sshd_config` file:

    AuthorizedKeysCommand /path/to/ssh-ldap-publickey
    AuthorizedKeysCommandUser nobody

OpenSSH's `AuthorizedKeysCommand` support requires version 6.1 or greater.

Debian Package
--------------

The repository also contains the files for creating a Debian package, which can
be done with the `dpkg-buildpackage` command. Alternatively, I distribute
pre-built versions for the stable i386 and amd64 versions of Debian in my
[personal APT repository][2].

[1]: https://github.com/AndriiGrytsenko/openssh-ldap-publickey
[2]: http://grahamedgecombe.com/apt-repository
