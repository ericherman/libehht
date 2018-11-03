/* test_ehht_foreach_element.c: test for a simple OO hashtable
   Copyright (C) 2016, 2017, 2018 Eric Herman <eric@freesa.org>

   This work is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or (at
   your option) any later version.

   This work is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

	https://www.gnu.org/licenses/lgpl-3.0.txt
	https://www.gnu.org/licenses/gpl-3.0.txt
*/
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

int test_ehht_foreach_element()
{
	int failures = 0;
	struct ehht_s *table;
	unsigned int actual, expected, num_buckets = 5;

	table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);

	if (table == NULL) {
		++failures;
	}

	table->put(table, "g", 1, "wiz");
	table->put(table, "foo", 3, "bar");
	table->put(table, "whiz", 4, "bang");
	table->put(table, "love", 4, "backend development");

	expected = 3 + 3 + 4 + 4 + 4 + 19;
	actual = 0;

	table->for_each(table, foreach_thing, &actual);

	failures += check_unsigned_long_m(actual, expected, "foreach");

	failures += check_unsigned_int_m(table->size(table), 4, "ehht_size");

	ehht_free(table);
	return failures;
}

TEST_EHHT_MAIN(test_ehht_foreach_element())
