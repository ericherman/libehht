/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_put_get_remove: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int test_ehht_put_get_remove(void)
{
	int failures = 0;
	struct ehht_s *table;
	const char *key;
	size_t num_buckets = 3;
	void *val, *old_val;
	char buf[1000];
	int err;

	table =
	    ehht_new_custom(num_buckets, NULL, NULL, NULL, NULL, NULL, NULL);

	key = "key1";

	failures += check_int(table->has_key(table, key, strlen(key)), 0);
	failures += check_int(table->has_key(table, key, strlen(key)), 0);

	val = table->get(table, key, strlen(key));

	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get while empty");

	val = "foo";
	err = 0;
	old_val = table->put(table, key, strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m((unsigned long)old_val, (unsigned long)NULL,
				  "ehht_put while empty");
	failures += check_int(table->has_key(table, key, strlen(key)), 1);

	val = table->get(table, key, strlen(key));
	if (val == NULL) {
		failures += 1;
	} else {
		failures += check_str_m((const char *)val, "foo", "ehht_get");
	}
	old_val = table->put(table, key, strlen(key), "bar", &err);
	failures += check_int(err, 0);
	failures += check_str_m((const char *)old_val, val, "ehht_put over");

	val = table->get(table, "baz", strlen("baz"));
	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get bogus");

	val = table->get(table, key, strlen(key));
	failures += check_str_m((const char *)val, "bar", "ehht_get replaced");

	key = "two";
	table->put(table, key, strlen(key), "2", &err);
	failures += check_int(err, 0);

	key = "three";
	table->put(table, key, strlen(key), "3", &err);
	failures += check_int(err, 0);

	key = "four";
	table->put(table, key, strlen(key), "4", &err);
	failures += check_int(err, 0);

	key = "ping";
	table->put(table, key, strlen(key), "pong", &err);
	failures += check_int(err, 0);

	key = "whiz";
	table->put(table, key, strlen(key), "bang", &err);
	failures += check_int(err, 0);

	key = "seven";
	table->put(table, key, strlen(key), "7", &err);
	failures += check_int(err, 0);

	key = "eight";
	table->put(table, key, strlen(key), "8", &err);
	failures += check_int(err, 0);

	key = "nine";
	table->put(table, key, strlen(key), "9", &err);
	failures += check_int(err, 0);
	failures += check_unsigned_int_m(table->size(table), 9, "ehht_size");

	key = "ping";
	failures += check_int(table->has_key(table, key, strlen(key)), 1);
	old_val = table->remove(table, key, strlen(key));
	failures += check_str_m((const char *)old_val, "pong", "ehht_remove");
	failures += check_int(table->has_key(table, key, strlen(key)), 0);
	val = table->get(table, "ping", strlen(key));
	if (val != NULL) {
		++failures;
	}
	failures += check_int(table->has_key(table, key, strlen(key)), 0);
	failures += check_unsigned_int_m(table->size(table), 8, "remove size");
	old_val = table->remove(table, "bogus", strlen("bogus"));
	failures += check_unsigned_int_m(table->size(table), 8, "remove size");
	if (old_val != NULL) {
		++failures;
	}

	table->remove(table, "key1", strlen("key1"));
	table->remove(table, "two", strlen("two"));
	table->remove(table, "nine", strlen("nine"));
	table->remove(table, "three", strlen("three"));
	table->remove(table, "eight", strlen("eight"));
	table->remove(table, "four", strlen("four"));
	table->remove(table, "seven", strlen("seven"));

	table->to_string(table, buf, 1000);

	failures += check_unsigned_int_m(table->size(table), 1, buf);
	old_val = table->remove(table, "whiz", strlen("whiz"));
	failures += check_str_m((const char *)old_val, "bang", "last remove");
	failures += check_unsigned_int_m(table->size(table), 0, "last size");

	ehht_free(table);
	return failures;
}

TEST_EHHT_MAIN(test_ehht_put_get_remove())
