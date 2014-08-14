TARGET=ssh-ldap-publickey
SOURCE=$(TARGET).c
MAN=$(TARGET).8
MAN_GZ=$(MAN).gz
SCHEMA=$(TARGET).schema
PREFIX=/usr/local

.PHONY: clean all install

all: $(TARGET) $(MAN_GZ)

clean:
	$(RM) $(TARGET) $(MAN_GZ)

install: all
	install -D $(TARGET) $(DESTDIR)/$(PREFIX)/bin/$(TARGET)
	install -Dm644 $(MAN_GZ) $(DESTDIR)/$(PREFIX)/share/man/man8/$(MAN_GZ)
	install -Dm644 $(SCHEMA) $(DESTDIR)/$(PREFIX)/share/doc/$(TARGET)/$(SCHEMA)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -std=c99 -o $@ $< -lldap -llber

$(MAN_GZ): $(MAN)
	gzip -9c $(MAN) > $(MAN_GZ)
