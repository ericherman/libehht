/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_out_of_memory.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "echeck.h"

unsigned test_flyweight(void)
{
	const size_t bytes_len = 500 * sizeof(size_t);
	unsigned char bytes[500 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	size_t keysize = 0;
	size_t alloc_bytes_before_puts = 0;
	size_t alloc_bytes_after_puts = 0;
	size_t alloc_bytes_diff = 0;
	int err = 0;
	struct ehht *table = NULL;
	struct echeck_err_injecting_context ctx;
	struct eembed_allocator wrap;
	struct eembed_allocator *real = NULL;
	struct eembed_log *log = eembed_err_log;
	size_t num_buckets = 5;

	char *key1 = NULL;
	char *key2 = NULL;
	char *key3 = NULL;

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	real = eembed_global_allocator;
	echeck_err_injecting_allocator_init(&wrap, real, &ctx, log);

	table = ehht_new_custom(num_buckets, NULL, &wrap, log);
	if (check_ptr_not_null(table)) {
		++failures;
		goto end_test_flyweight;
	}

	keysize = (bytes_len / 8) - 10;
	key1 = (char *)real->calloc(real, sizeof(char), (keysize + 1));
	if (check_ptr_not_null(key1)) {
		++failures;
		goto end_test_flyweight;
	}
	eembed_memset(key1, 'A', keysize);

	key2 = (char *)real->calloc(real, sizeof(char), (keysize + 1));
	if (check_ptr_not_null(key2)) {
		++failures;
		goto end_test_flyweight;
	}
	eembed_memset(key2, 'B', keysize);

	key3 = (char *)real->calloc(real, sizeof(char), (keysize + 1));
	if (check_ptr_not_null(key3)) {
		++failures;
		goto end_test_flyweight;
	}
	eembed_memset(key3, 'C', keysize);

	alloc_bytes_before_puts = ctx.alloc_bytes;

	err = 0;
	table->put(table, key1, keysize, "A0", &err);
	if (err) {
		log->append_s(log, "A0 Err: ");
		log->append_l(log, err);
		log->append_s(log, "?");
		log->append_eol(log);
		++failures;
		goto end_test_flyweight;
	}
	table->put(table, key2, keysize, "B0", &err);
	if (err) {
		log->append_s(log, "B0 Err: ");
		log->append_l(log, err);
		log->append_s(log, "?");
		log->append_eol(log);
		++failures;
		goto end_test_flyweight;
	}

	alloc_bytes_after_puts = ctx.alloc_bytes;
	alloc_bytes_diff = alloc_bytes_after_puts - alloc_bytes_before_puts;

	err =
	    check_int_m(alloc_bytes_diff >= (2 * keysize) ? 1 : 0, 1,
			"failed to alloc key copies?");
	if (err) {
		++failures;
		goto end_test_flyweight;
	}

	err = ehht_trust_keys_immutable(table, 0);
	failures += check_int_m(err, 0, "ehht_trust_keys_immutable 0, same");

	err = ehht_trust_keys_immutable(table, 1);
	failures +=
	    check_int_m(err ? 1 : 0, 1, "ehht_trust_keys_immutable 1, invalid");
	table->put(table, key3, keysize, "C0", &err);

	table->clear(table);
	err = ehht_trust_keys_immutable(table, 1);
	failures += check_int_m(err, 0, "ehht_trust_keys_immutable(1)");

	alloc_bytes_before_puts = ctx.alloc_bytes;

	err = 0;
	table->put(table, key1, keysize, "A1", &err);
	if (err) {
		log->append_s(log, "A1 Err: ");
		log->append_l(log, err);
		log->append_s(log, "?");
		log->append_eol(log);
		++failures;
		goto end_test_flyweight;
	}
	table->put(table, key2, keysize, "B2", &err);
	if (err) {
		log->append_s(log, "B2 Err: ");
		log->append_l(log, err);
		log->append_s(log, "?");
		log->append_eol(log);
		++failures;
		goto end_test_flyweight;
	}

	alloc_bytes_after_puts = ctx.alloc_bytes;
	alloc_bytes_diff = alloc_bytes_after_puts - alloc_bytes_before_puts;

	err = check_int_m(alloc_bytes_diff <= keysize ? 1 : 0, 1,
			  "alloc'd key copies?");
	if (err) {
		++failures;
		goto end_test_flyweight;
	}
	failures += check_str((char *)table->get(table, key1, keysize), "A1");
	failures += check_str((char *)table->get(table, key2, keysize), "B2");

end_test_flyweight:
	table->clear(table);
	ehht_free(table);

	real->free(real, key3);
	real->free(real, key2);
	real->free(real, key1);

	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

ECHECK_TEST_MAIN(test_flyweight)
