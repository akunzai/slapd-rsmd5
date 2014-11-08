slapd-rsmd5
===========

Reverse Salt MD5 Password for OpenLDAP

rsmd5.c provides Reverse Salt MD5 Password scheme for OpenLDAP

Scheme: {RSMD5}

# Compare to SMD5

SMD5: "{SMD5}" + Base64Encode(MD5(password + salt) + salt)

RSMD5: "{RSMD5}" + Base64Encode(MD5(salt + password) + salt)

# Requirements

* OpenLDAP

# Installations

First,You need to configure OpenLDAP.

	$ cd <OPENLDAP_BUILD_DIR>
	$ ./configure
	$ cd <OPENLDAP_BUILD_DIR>/contrib/slapd-modules/passwd/
	$ git clone https://github.com/akunzai/slapd-rsmd5.git
	$ cd slapd-rsmd5
	$ make 
	$ sudo make install
	
# Configuration

In slapd.conf:

	moduleload rsmd5.so

OLC Style:

	$ vi rsmd5.ldif

```ldif
dn: cn=module{0},cn=config
changetype: modify
add: olcModuleLoad
olcModuleLoad: rsmd5
```

	$ sudo ldapadd -Y EXTERNAL -H ldapi:/// -f rsmd5.ldif

# Testing

you can update userPassword with:

	userPassword: {RSMD5}c2FsdNQqD3Qv/Bvk4cwIRcJCnJc=

and trying to bind with "secret",should be success.

	$ ldapsearch -x -D 'cn=rsmd5user,dc=example,dc=com' -w secret
	
# References

* [PBKDF2 for OpenLDAP][openldap-pbkdf2]

[openldap-pbkdf2]: https://github.com/hamano/openldap-pbkdf2

# License

This work is part of OpenLDAP Software <http://www.openldap.org/>.

Copyright 2009-2013 The OpenLDAP Foundation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted only as authorized by the OpenLDAP
Public License.

A copy of this license is available in the file LICENSE in the
top-level directory of the distribution or, alternatively, at
<http://www.OpenLDAP.org/license.html>.
