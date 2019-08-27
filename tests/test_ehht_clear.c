/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_clear.h: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int test_ehht_clear()
{
	int failures = 0;
	struct ehht_s *table;
	size_t num_buckets = 5;

	size_t i, count, items_written;
	size_t report[REPORT_LEN];
	struct tracking_mem_context ctx = { 0, 0, 0, 0, 0, 0, 0 };

	table = ehht_new(num_buckets, NULL, test_malloc, test_free, &ctx);

	table->put(table, "g", 1, "wiz");
	table->put(table, "foo", 3, "bar");
	table->put(table, "whiz", 4, "bang");
	table->put(table, "love", 4, "backend development");

	failures += check_unsigned_int_m(table->size(table), 4, "ehht_size");

	items_written = ehht_distribution_report(table, report, REPORT_LEN);
	count = 0;
	for (i = 0; i < REPORT_LEN; ++i) {
		count += report[i];
	}
	failures += check_size_t_m(count, 4, "ehht_report 1");
	failures += check_size_t_m(items_written, count, "ehht_report 1");

	table->clear(table);

	failures += check_unsigned_int_m(table->size(table), 0, "clear");

	items_written = ehht_distribution_report(table, report, 10);
	count = 0;
	for (i = 0; i < items_written; ++i) {
		count += report[i];
	}
	failures += check_size_t_m(count, 0, "ehht_report empty");
	failures += check_size_t_m(items_written, count, "ehht_report full");

	ehht_free(table);

	failures += check_unsigned_int_m(ctx.allocs > 0, 1, "ctx.allocs");
	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures += check_unsigned_int_m(ctx.alloc_bytes > 0, 1, "bytes > 0");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");
	failures += check_unsigned_int_m(ctx.fails, 0, "free NULL pointers");

	return failures;
}

TEST_EHHT_MAIN(test_ehht_clear())
