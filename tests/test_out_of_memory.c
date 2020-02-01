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

int test_out_of_memory(void)
{
	int failures = 0;

	failures += test_out_of_memory_construction(0);
	failures += test_out_of_memory_construction(1 << 0);
	failures += test_out_of_memory_construction(1 << 1);
	failures += test_out_of_memory_construction(1 << 2);

	return failures;
}

TEST_EHHT_MAIN(test_out_of_memory())
