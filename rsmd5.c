/* $OpenLDAP$ */
/* This work is part of OpenLDAP Software <http://www.openldap.org/>.
 *
 * Copyright 1998-2012 The OpenLDAP Foundation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */
#include "portable.h"
#include <ac/string.h>
#include <lber.h>
#include <stdio.h>

#include "lber_pvt.h" /* BER_BVC definition */
#include "lutil_md5.h"
#include "lutil.h"

#define	SALT_SIZE	4

const struct berval rsmd5_scheme = BER_BVC("{RSMD5}");

static int chk_rsmd5(const struct berval *scheme, const struct berval *passwd,
		const struct berval *cred, const char **text) {
#ifdef SLAPD_RSDM5_DEBUG
	fprintf(stderr, "Validating password\n");
	fprintf(stderr, "  Password to validate: %s\n", cred->bv_val);
	fprintf(stderr, "  Stored password scheme: %s\n", scheme->bv_val);
	fprintf(stderr, "  Stored password value: %s\n", passwd->bv_val);
#endif
	lutil_MD5_CTX MD5context;
	unsigned char MD5digest[LUTIL_MD5_BYTES];
	int rc;
	unsigned char *orig_pass = NULL;
	struct berval digest;
	struct berval salt;

	/* safety check */
	if (LUTIL_BASE64_DECODE_LEN(passwd->bv_len) <= sizeof(MD5digest)) {
		return LUTIL_PASSWD_ERR;
	}

	orig_pass = (unsigned char *) ber_memalloc((size_t) (
	LUTIL_BASE64_DECODE_LEN(passwd->bv_len) + 1));

	if (orig_pass == NULL)
		return LUTIL_PASSWD_ERR;

	/* base64 un-encode password */
	rc = lutil_b64_pton(passwd->bv_val, orig_pass, passwd->bv_len);

	//length of digest with salt
	if (rc <= (int) (sizeof(MD5digest))) {
		ber_memfree(orig_pass);
		return LUTIL_PASSWD_ERR;
	}
	salt.bv_len = rc - sizeof(MD5digest);
	salt.bv_val = (char *)orig_pass;
	digest.bv_len = sizeof(MD5digest);
	digest.bv_val = (char *)orig_pass;
	digest.bv_val += salt.bv_len;

	/* hash credentials with salt */
	lutil_MD5Init(&MD5context);
	lutil_MD5Update(&MD5context, (const unsigned char *) salt.bv_val,
			salt.bv_len);
	lutil_MD5Update(&MD5context, (const unsigned char *) cred->bv_val,
			cred->bv_len);
	lutil_MD5Final(MD5digest, &MD5context);

	/* compare */
	rc = memcmp((char *) digest.bv_val, (char *) MD5digest, sizeof(MD5digest));
	ber_memfree(orig_pass);
#ifdef SLAPD_RSDM5_DEBUG
	fprintf(stderr,"  -> Passwords %s\n", rc == 0 ? "match" : "do not match");
#endif
	return rc ? LUTIL_PASSWD_ERR : LUTIL_PASSWD_OK;
}

static int hash_rsmd5(const struct berval *scheme, const struct berval *passwd,
		struct berval *hash, const char **text) {
	lutil_MD5_CTX MD5context;
	unsigned char MD5digest[LUTIL_MD5_BYTES];
	char saltdata[SALT_SIZE];
	struct berval digest;
	struct berval salt;

	digest.bv_val = (char *) MD5digest;
	digest.bv_len = sizeof(MD5digest);
	salt.bv_val = saltdata;
	salt.bv_len = sizeof(saltdata);

	if (lutil_entropy((unsigned char *) salt.bv_val, salt.bv_len) < 0) {
		return LUTIL_PASSWD_ERR;
	}

	lutil_MD5Init(&MD5context);
	lutil_MD5Update(&MD5context, (const unsigned char *) salt.bv_val,
			salt.bv_len);
	lutil_MD5Update(&MD5context, (const unsigned char *) passwd->bv_val,
			passwd->bv_len);
	lutil_MD5Final(MD5digest, &MD5context);

	return lutil_passwd_string64(scheme, &digest, hash, &salt);
}

int init_module(int argc, char *argv[]) {
	int rc;
	rc = lutil_passwd_add((struct berval *) &rsmd5_scheme, chk_rsmd5,
			hash_rsmd5);
	return rc;
}
