# $OpenLDAP$

top_srcdir = ../../../..
prefix=/usr
#prefix=`grep -e "^prefix =" $(top_srcdir)/Makefile | cut -d= -f2 | tr -d ' '`
exec_prefix=$(prefix)
ldap_subdir=/ldap

libdir=$(exec_prefix)/lib
libexecdir=$(exec_prefix)/lib
moduledir = $(libexecdir)$(ldap_subdir)

LIBTOOL = $(top_srcdir)/libtool
CC=gcc
OPT=-Wall -g -O2

#DEFS=-DSLAPD_RSDM5_DEBUG

LDAP_INC=-I$(top_srcdir)/include -I$(top_srcdir)/servers/slapd
INCS=$(LDAP_INC)

LDAP_LIB=-lldap_r -llber
LIBS=$(LDAP_LIB)

all: rsmd5.la

rsmd5.lo: rsmd5.c
	$(LIBTOOL) --mode=compile $(CC) $(OPT) $(DEFS) $(INCS) -c $?

rsmd5.la: rsmd5.lo
	$(LIBTOOL) --mode=link $(CC) $(OPT) -version-info 0:0:0 \
	-rpath $(moduledir) -module -o $@ $? $(LIBS)

clean:
	rm -rf *.o *.lo *.la .libs

install: rsmd5.la
	mkdir -p $(DESTDIR)$(moduledir)
	$(LIBTOOL) --mode=install cp rsmd5.la $(DESTDIR)$(moduledir)
