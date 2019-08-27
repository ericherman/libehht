/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* djb2_hash.c: a re-implementation of djb2 hash for NULL-containing data */
/* Copyright (C) 2019 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */
/* See Also: http://www.cse.yorku.ca/~oz/hash.html */

#include <stddef.h>		/* size_t */

#ifndef DJB2_HASH_SEED
/*
        primes[0] == N/A
        primes[1] == 2
        primes[2] == 3
        primes[3] == 5
        primes[5] == 11
        primes[11] == 31
        primes[31] == 127
        primes[127] == 709
        primes[709] == 5381
        primes[5381] == 52711
        primes[52711] == 648391
        ...
*/
#define DJB2_HASH_SEED 5381
#endif /* DJB2_HASH_SEED */
unsigned int djb2_hash(const char *data, size_t len)
{
	unsigned int hash;
	size_t i;

	hash = DJB2_HASH_SEED;
	for (i = 0; i < len; ++i) {
		hash = ((hash << 5) + hash) + ((unsigned int)data[i]);
	}

	return hash;
}
