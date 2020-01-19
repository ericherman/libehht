/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_new.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int test_ehht_new()
{
	int failures = 0;
	struct ehht_s *table;
	size_t num_buckets = 35;

	/* msg = "test_ehht_new"; */
	table = ehht_new_custom(num_buckets, NULL, NULL, NULL, NULL);

	if (table == NULL) {
		++failures;
	}

	failures += check_unsigned_int_m(table->size(table), 0, "ehht_size");
	failures +=
	    check_unsigned_int_m(ehht_buckets_size(table), num_buckets,
				 "ehht_buckets_size");

	ehht_free(table);
	return failures;
}

TEST_EHHT_MAIN(test_ehht_new())
