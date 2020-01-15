/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_collision_resize.c: test for a simple OO hashtable */
/* Copyright (C) 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

/* this fake hashcode function allows easy testing of hash collisions */
unsigned int ehht_first_letter_bogus_hashcode(const char *data, size_t len)
{
	return (data && len) ? (unsigned int)(data[0]) : 0;
}

int test_ehht_collision_resize(void)
{
	int failures = 0;
	struct ehht_s *table;
	const char *key;
	void *val;
	ehht_hash_func first_letter_func = ehht_first_letter_bogus_hashcode;
	size_t num_buckets = 10;

	table = ehht_new(num_buckets, first_letter_func, NULL, NULL, NULL);
	ehht_set_collision_resize_load_factor(table, 0.75);

	failures += check_unsigned_long(num_buckets, table->num_buckets(table));

	val = NULL;
	key = "a1";
	table->put(table, key, strlen(key), val);
	failures += check_unsigned_long(num_buckets, table->num_buckets(table));

	key = "a2";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? a2");

	key = "c3";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? c3");

	key = "d4";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? d4");

	key = "e5";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? e5");

	key = "f6";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? f6");

	key = "g7";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? g7");

	/* we are not above the load factor, but no a bucket collision */
	key = "g8";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? g8");

	num_buckets *= 2;
	key = "g9";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "failed to split? g9");

	key = "g10";
	table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m(num_buckets, table->num_buckets(table),
				  "premature split? g10");

	ehht_free(table);
	return failures;
}

TEST_EHHT_MAIN(test_ehht_collision_resize())
