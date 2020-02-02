/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_foreach_element.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int foreach_thing(struct ehht_key_s each_key, void *each_val, void *context)
{

	unsigned int *i;
	const char *val;

	i = (unsigned int *)context;
	val = (const char *)each_val;

	if (each_key.len > 2) {
		*i += (strlen(each_key.str) + strlen(val));
	}

	return 0;
}

int test_ehht_foreach_element(void)
{
	int failures = 0;
	struct ehht_s *table;
	unsigned int actual, expected, num_buckets = 5;
	int err = 0;

	table =
	    ehht_new_custom(num_buckets, NULL, NULL, NULL, NULL, NULL, NULL);

	if (table == NULL) {
		++failures;
		return failures;
	}

	err = 0;
	table->put(table, "g", 1, "wiz", &err);
	table->put(table, "foo", 3, "bar", &err);
	table->put(table, "whiz", 4, "bang", &err);
	table->put(table, "love", 4, "backend development", &err);
	failures += check_int(err, 0);

	expected = 3 + 3 + 4 + 4 + 4 + 19;
	actual = 0;

	table->for_each(table, foreach_thing, &actual);

	failures += check_unsigned_long_m(actual, expected, "foreach");

	failures += check_unsigned_int_m(table->size(table), 4, "ehht_size");

	ehht_free(table);
	return failures;
}

TEST_EHHT_MAIN(test_ehht_foreach_element())
