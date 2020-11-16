/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_foreach_element.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "echeck.h"

int foreach_thing(struct ehht_key each_key, void *each_val, void *context)
{

	unsigned int *i = NULL;
	const char *val = NULL;

	i = (unsigned int *)context;
	val = (const char *)each_val;

	if (each_key.len > 2) {
		*i += (eembed_strlen(each_key.str) + eembed_strlen(val));
	}

	return 0;
}

unsigned test_ehht_foreach_element(void)
{
	const size_t bytes_len = 250 * sizeof(size_t);
	unsigned char bytes[250 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = NULL;
	unsigned int actual, expected, num_buckets = 5;
	int err = 0;

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	table = ehht_new_custom(num_buckets, NULL, NULL, NULL);

	if (table == NULL) {
		++failures;
		goto test_ehht_foreach_element_end;
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

test_ehht_foreach_element_end:
	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

ECHECK_TEST_MAIN(test_ehht_foreach_element)
