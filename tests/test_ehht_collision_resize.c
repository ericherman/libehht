/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_collision_resize.c: test for a simple OO hashtable */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "echeck.h"

/* this fake hashcode function allows easy testing of hash collisions */
unsigned int ehht_first_char_bogus_hashcode(const char *data, size_t len)
{
	return (data && len) ? (unsigned int)(data[0]) : 0;
}

unsigned test_ehht_collision_resize_buckets(void)
{
	const size_t bytes_len = 250 * sizeof(size_t);
	unsigned char bytes[250 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = NULL;
	const char *key = NULL;
	void *val = NULL;
	ehht_hash_func first_char_func = ehht_first_char_bogus_hashcode;
	size_t num_buckets = 10;
	int err = 0;

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	table = ehht_new_custom(num_buckets, first_char_func, NULL, NULL);
	ehht_buckets_auto_resize_load_factor(table, 0.75);

	failures += check_unsigned_long(num_buckets, ehht_buckets_size(table));

	val = NULL;
	key = "a1";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures += check_unsigned_long(num_buckets, ehht_buckets_size(table));

	key = "a2";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? a2");

	key = "c3";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? c3");

	key = "d4";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? d4");

	key = "e5";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? e5");

	key = "f6";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? f6");

	key = "g7";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? g7");

	/* we are not above the load factor, but no a bucket collision */
	key = "g8";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? g8");

	num_buckets *= 2;
	key = "g9";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "failed to split? g9");

	key = "g10";
	table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m(num_buckets, ehht_buckets_size(table),
				  "premature split? g10");

	ehht_free(table);

	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

ECHECK_TEST_MAIN(test_ehht_collision_resize_buckets)
