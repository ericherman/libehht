/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_out_of_memory.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int test_flyweight(void)
{
	int failures = 0;
	struct ehht_sprintf_context_s err_ctx = { NULL, 0 };
	size_t keysize = 0;
	size_t alloc_bytes_before_puts = 0;
	size_t alloc_bytes_after_puts = 0;
	size_t alloc_bytes_diff = 0;
	int err = 0;
	struct ehht_s *table = NULL;
	struct tracking_mem_context ctx;
	char *key1 = NULL;
	char *key2 = NULL;
	char *key3 = NULL;
	char errmsg[10];
	char *found = NULL;

	memset(&ctx, 0x00, sizeof(struct tracking_mem_context));

	err_ctx.size = 80 * 1000;
	err_ctx.buf = calloc(err_ctx.size, 1);
	assert(err_ctx.buf != NULL);

	table =
	    ehht_new_custom(0, NULL, test_malloc, test_free, &ctx, ehht_sprintf,
			    &err_ctx);
	assert(table);

	keysize = 500;
	key1 = calloc(sizeof(char), (keysize + 1));
	memset(key1, 'A', keysize);

	key2 = calloc(sizeof(char), (keysize + 1));
	memset(key2, 'B', keysize);

	key3 = calloc(sizeof(char), (keysize + 1));
	memset(key3, 'C', keysize);

	alloc_bytes_before_puts = ctx.alloc_bytes;

	err = 0;
	table->put(table, key1, keysize, "A0", &err);
	table->put(table, key2, keysize, "B0", &err);
	if (err) {
		fprintf(stderr, "Err: %d?A\n", err);
		++failures;
		goto end_test_flyweight;
	}

	alloc_bytes_after_puts = ctx.alloc_bytes;
	alloc_bytes_diff = alloc_bytes_after_puts - alloc_bytes_before_puts;

	if (alloc_bytes_diff < (2 * keysize)) {
		fprintf(stderr, "failed to alloc key copies?\n");
		++failures;
		goto end_test_flyweight;
	}

	err = ehht_trust_keys_immutable(table, 0);
	failures += check_int_m(err, 0, "ehht_trust_keys_immutable 0, same");

	err = ehht_trust_keys_immutable(table, 1);
	failures +=
	    check_int_m(err ? 1 : 0, 1, "ehht_trust_keys_immutable 1, invalid");
	sprintf(errmsg, "Error %d:", 12);
	found = strstr(err_ctx.buf, errmsg);
	if (found == NULL) {
		failures += check_str(found, errmsg);
		if (strlen(err_ctx.buf)) {
			fprintf(stderr, "%s\n", err_ctx.buf);
		}
	}
	memset(err_ctx.buf, 0x00, err_ctx.size);
	table->put(table, key3, keysize, "C0", &err);

	table->clear(table);
	err = ehht_trust_keys_immutable(table, 1);
	failures += check_int_m(err, 0, "ehht_trust_keys_immutable(1)");

	alloc_bytes_before_puts = ctx.alloc_bytes;

	err = 0;
	table->put(table, key1, keysize, "A1", &err);
	table->put(table, key2, keysize, "B2", &err);
	if (err) {
		fprintf(stderr, "Err: %d?\n", err);
		++failures;
		goto end_test_flyweight;
	}

	alloc_bytes_after_puts = ctx.alloc_bytes;
	alloc_bytes_diff = alloc_bytes_after_puts - alloc_bytes_before_puts;

	if (alloc_bytes_diff > keysize) {
		fprintf(stderr, "alloc'd key copies?\n");
		++failures;
		goto end_test_flyweight;
	}
	failures += check_str((char *)table->get(table, key1, keysize), "A1");
	failures += check_str((char *)table->get(table, key2, keysize), "B2");

end_test_flyweight:
	table->clear(table);
	ehht_free(table);

	if (strlen(err_ctx.buf)) {
		++failures;
		fprintf(stderr, "%s\n", err_ctx.buf);
	}

	free(key3);
	free(key2);
	free(key1);

	return failures;
}

TEST_EHHT_MAIN(test_flyweight())
