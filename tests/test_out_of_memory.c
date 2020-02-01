/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_out_of_memory.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int test_out_of_memory_construction(unsigned allocs_to_fail)
{
	int failures = 0;
	struct ehht_s *table;
	struct tracking_mem_context ctx;

	memset(&ctx, 0x00, sizeof(struct tracking_mem_context));
	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	table = ehht_new_custom(0, NULL, test_malloc, test_free, &ctx);
	if (table) {
		if (allocs_to_fail) {
			++failures;
		}
		ehht_free(table);
	} else if (!allocs_to_fail) {
		++failures;
	}

	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");

	return failures;
}

int test_out_of_memory_put(unsigned allocs_to_fail)
{
	int failures = 0;
	struct ehht_s *table;
	struct tracking_mem_context ctx;
	size_t i;
	int err;
	char buf[80];

	memset(&ctx, 0x00, sizeof(struct tracking_mem_context));
	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	table = ehht_new_custom(0, NULL, test_malloc, test_free, &ctx);
	if (!table) {
		fprintf(stderr, "%s:%d: ehht_new_custom\n", __FILE__, __LINE__);
		return 1;
	}

	err = 0;
	for (i = 0; i < 10; ++i) {
		sprintf(buf, "%u", (unsigned)i);
		table->put(table, buf, strlen(buf), NULL, &err);
		if (err && !allocs_to_fail) {
			++failures;
		}
	}
	if (allocs_to_fail && !err) {
		++failures;
	}

	ehht_free(table);

	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");

	return failures;
}

int test_out_of_memory_keys(unsigned allocs_to_fail)
{
	int failures = 0;
	struct ehht_s *table;
	struct tracking_mem_context ctx;
	size_t i;
	int err, allocate_copies;
	char buf[80];
	struct ehht_keys_s *ks;

	memset(&ctx, 0x00, sizeof(struct tracking_mem_context));
	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	table = ehht_new_custom(0, NULL, test_malloc, test_free, &ctx);
	if (!table) {
		fprintf(stderr, "%s:%d: ehht_new_custom\n", __FILE__, __LINE__);
		return 1;
	}

	err = 0;
	for (i = 0; i < 10; ++i) {
		sprintf(buf, "%u", (unsigned)i);
		table->put(table, buf, strlen(buf), NULL, &err);
		if (err) {
			fprintf(stderr, "%s:%d: put %u\n", __FILE__, __LINE__,
				(unsigned)i);
			return 1;
		}
	}

	err = 0;
	for (i = 0; i < 10; ++i) {
		allocate_copies = 1;
		ks = table->keys(table, allocate_copies);
		if (!ks && !allocs_to_fail) {
			fprintf(stderr, "%s:%d: !ks %u\n", __FILE__, __LINE__,
				(unsigned)i);
			return 1;
		}
		if (ks) {
			table->free_keys(table, ks);
		} else {
			++err;
		}
	}
	failures += (!err && allocs_to_fail) ? 1 : 0;
	failures += (err && !allocs_to_fail) ? 1 : 0;

	ehht_free(table);

	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");

	return failures;
}

int test_out_of_memory(void)
{
	int failures = 0;

	failures += test_out_of_memory_construction(0);
	failures += test_out_of_memory_construction(1 << 0);
	failures += test_out_of_memory_construction(1 << 1);
	failures += test_out_of_memory_construction(1 << 2);

	failures += test_out_of_memory_put(0);
	failures += test_out_of_memory_put(1 << 4);
	failures += test_out_of_memory_put(1 << 5);
	failures += test_out_of_memory_put(1 << 6);
	failures += test_out_of_memory_put(1 << 7);

	failures += test_out_of_memory_keys(0);
	failures += test_out_of_memory_keys(1 << 23);
	failures += test_out_of_memory_keys(1 << 24);
	failures += test_out_of_memory_keys(1 << 25);
	failures += test_out_of_memory_keys(1 << 27);
	failures += test_out_of_memory_keys(1 << 28);

	return failures;
}

TEST_EHHT_MAIN(test_out_of_memory())
