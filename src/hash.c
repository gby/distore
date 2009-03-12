/*****************************************************************************
 *
 *    Copyright (C) 2009 Codefidence Ltd www.codefidence.com
 *
 *    This file is a part of Distore
 *
 *    Distore is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Distore is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Distore.  If not, see <http://www.gnu.org/licenses/>.
 * 
 ****************************************************************************/

#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>

#include "hash.h"

static EVP_MD_CTX mdctx;

void sha1sumInit() {
	const EVP_MD *md = EVP_sha1();

	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx, md, NULL);
}

void sha1sumUpdate(const char *input, const unsigned int input_size) {
	EVP_DigestUpdate(&mdctx, input, input_size);
}

void sha1sumFinal(unsigned char *digest) {
	unsigned int dlen = 0;

	EVP_DigestFinal_ex(&mdctx, digest, &dlen);
	EVP_MD_CTX_cleanup(&mdctx);
}
