/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_out_of_memory.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int test_out_of_memory_construction(struct ehht_sprintf_context_s *err_ctx,
				    uint64_t allocs_to_fail)
{
	int failures = 0;
	struct ehht_s *table;
	struct oom_injecting_context_s ctx;

	memset(&ctx, 0x00, sizeof(struct oom_injecting_context_s));
	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	table =
	    ehht_new_custom(0, NULL, oom_injecting_malloc, oom_injecting_free,
			    &ctx, ehht_sprintf, err_ctx);
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

int test_out_of_memory_put(struct ehht_sprintf_context_s *err_ctx,
			   uint64_t allocs_to_fail)
{
	int failures = 0;
	struct ehht_s *table;
	struct oom_injecting_context_s ctx;
	size_t i, num_buckets;
	int err;
	char buf[80];

	memset(&ctx, 0x00, sizeof(struct oom_injecting_context_s));
	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	num_buckets = 10;
	table =
	    ehht_new_custom(num_buckets, NULL, oom_injecting_malloc,
			    oom_injecting_free, &ctx, ehht_sprintf, err_ctx);
	if (!table) {
		fprintf(stderr, "%s:%d: ehht_new_custom\n", __FILE__, __LINE__);
		return 1;
	}

	err = 0;
	for (i = 0; i < 100; ++i) {
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

int test_out_of_memory_keys(struct ehht_sprintf_context_s *err_ctx,
			    uint64_t allocs_to_fail)
{
	int failures = 0;
	struct ehht_s *table;
	struct oom_injecting_context_s ctx;
	size_t i;
	int err, allocate_copies;
	char buf[80];
	struct ehht_keys_s *ks;

	memset(&ctx, 0x00, sizeof(struct oom_injecting_context_s));
	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	table =
	    ehht_new_custom(0, NULL, oom_injecting_malloc, oom_injecting_free,
			    &ctx, ehht_sprintf, err_ctx);
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
	struct ehht_sprintf_context_s ctx = { NULL, 0 };
	size_t i;

	ctx.size = 80 * 1000;
	ctx.buf = calloc(ctx.size, 1);
	assert(ctx.buf != NULL);

	failures += test_out_of_memory_construction(&ctx, 0);
	failures += test_out_of_memory_construction(&ctx, 1UL << 0);
	failures += test_out_of_memory_construction(&ctx, 1UL << 1);
	failures += test_out_of_memory_construction(&ctx, 1UL << 2);

	failures += test_out_of_memory_put(&ctx, 0);
	for (i = 4; i < 64; ++i) {
		failures += test_out_of_memory_put(&ctx, 1UL << i);
	}

	failures += test_out_of_memory_keys(&ctx, 0);
	failures += test_out_of_memory_keys(&ctx, 1UL << 23);
	failures += test_out_of_memory_keys(&ctx, 1UL << 24);
	failures += test_out_of_memory_keys(&ctx, 1UL << 25);
	failures += test_out_of_memory_keys(&ctx, 1UL << 27);
	failures += test_out_of_memory_keys(&ctx, 1UL << 28);

	for (i = 1; i <= 11; ++i) {
		char errmsg[10];
		char *found = NULL;

		sprintf(errmsg, "Error %u:", (unsigned)i);
		found = strstr(ctx.buf, errmsg);
		if (found == NULL) {
			failures += check_str(found, errmsg);
		}
	}

	free(ctx.buf);

	return failures;
}

TEST_EHHT_MAIN(test_out_of_memory())
