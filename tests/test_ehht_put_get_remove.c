/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_put_get_remove: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "echeck.h"

unsigned test_ehht_put_get_remove(void)
{
	const size_t bytes_len = 250 * sizeof(size_t);
	unsigned char bytes[250 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = NULL;
	const char *key = NULL;
	size_t num_buckets = 3;
	void *val = NULL;
	void *old_val = NULL;
	const size_t buflen = 500;
	char buf[500];
	int err = 0;

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	buf[0] = '\0';
	table = ehht_new_custom(num_buckets, NULL, NULL, NULL);

	key = "key1";

	failures +=
	    check_int(table->has_key(table, key, eembed_strlen(key)), 0);
	failures +=
	    check_int(table->has_key(table, key, eembed_strlen(key)), 0);

	val = table->get(table, key, eembed_strlen(key));

	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get while empty");

	val = "foo";
	err = 0;
	old_val = table->put(table, key, eembed_strlen(key), val, &err);
	failures += check_int(err, 0);
	failures +=
	    check_unsigned_long_m((unsigned long)old_val, (unsigned long)NULL,
				  "ehht_put while empty");
	failures +=
	    check_int(table->has_key(table, key, eembed_strlen(key)), 1);

	val = table->get(table, key, eembed_strlen(key));
	if (val == NULL) {
		failures += 1;
	} else {
		failures += check_str_m((const char *)val, "foo", "ehht_get");
	}
	old_val = table->put(table, key, eembed_strlen(key), "bar", &err);
	failures += check_int(err, 0);
	failures += check_str_m((char *)old_val, (char *)val, "ehht_put over");

	val = table->get(table, "baz", eembed_strlen("baz"));
	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get bogus");

	val = table->get(table, key, eembed_strlen(key));
	failures += check_str_m((const char *)val, "bar", "ehht_get replaced");

	key = "two";
	table->put(table, key, eembed_strlen(key), "2", &err);
	failures += check_int(err, 0);

	key = "three";
	table->put(table, key, eembed_strlen(key), "3", &err);
	failures += check_int(err, 0);

	key = "four";
	table->put(table, key, eembed_strlen(key), "4", &err);
	failures += check_int(err, 0);

	key = "ping";
	table->put(table, key, eembed_strlen(key), "pong", &err);
	failures += check_int(err, 0);

	key = "whiz";
	table->put(table, key, eembed_strlen(key), "bang", &err);
	failures += check_int(err, 0);

	key = "seven";
	table->put(table, key, eembed_strlen(key), "7", &err);
	failures += check_int(err, 0);

	key = "eight";
	table->put(table, key, eembed_strlen(key), "8", &err);
	failures += check_int(err, 0);

	key = "nine";
	table->put(table, key, eembed_strlen(key), "9", &err);
	failures += check_int(err, 0);
	failures += check_unsigned_int_m(table->size(table), 9, "ehht_size");

	key = "ping";
	failures +=
	    check_int(table->has_key(table, key, eembed_strlen(key)), 1);
	old_val = table->remove(table, key, eembed_strlen(key));
	failures += check_str_m((const char *)old_val, "pong", "ehht_remove");
	failures +=
	    check_int(table->has_key(table, key, eembed_strlen(key)), 0);
	val = table->get(table, "ping", eembed_strlen(key));
	if (val != NULL) {
		++failures;
	}
	failures +=
	    check_int(table->has_key(table, key, eembed_strlen(key)), 0);
	failures += check_unsigned_int_m(table->size(table), 8, "remove size");
	old_val = table->remove(table, "bogus", eembed_strlen("bogus"));
	failures += check_unsigned_int_m(table->size(table), 8, "remove size");
	if (old_val != NULL) {
		++failures;
	}

	table->remove(table, "key1", eembed_strlen("key1"));
	table->remove(table, "two", eembed_strlen("two"));
	table->remove(table, "nine", eembed_strlen("nine"));
	table->remove(table, "three", eembed_strlen("three"));
	table->remove(table, "eight", eembed_strlen("eight"));
	table->remove(table, "four", eembed_strlen("four"));
	table->remove(table, "seven", eembed_strlen("seven"));

	table->to_string(table, buf, buflen);

	failures += check_unsigned_int_m(table->size(table), 1, buf);
	old_val = table->remove(table, "whiz", eembed_strlen("whiz"));
	failures += check_str_m((const char *)old_val, "bang", "last remove");
	failures += check_unsigned_int_m(table->size(table), 0, "last size");

	ehht_free(table);

	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

ECHECK_TEST_MAIN(test_ehht_put_get_remove)
