/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_clear.h: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "ehht-report.h"
#include "echeck.h"

unsigned test_ehht_clear(void)
{
	const size_t bytes_len = 250 * sizeof(size_t);
	unsigned char bytes[250 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table;
	size_t num_buckets = 5;
	size_t i, count, items_written;
	const size_t REPORT_LEN = 10;
	size_t report[10];
	struct echeck_err_injecting_context ctx;
	struct eembed_allocator wrap;
	struct eembed_allocator *real = NULL;
	struct eembed_log *log = eembed_err_log;

	int err;

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

	err = 0;
	table->put(table, "g", 1, "wiz", &err);
	failures += check_int(err, 0);
	table->put(table, "foo", 3, "bar", &err);
	failures += check_int(err, 0);
	table->put(table, "whiz", 4, "bang", &err);
	failures += check_int(err, 0);
	table->put(table, "love", 4, "backend development", &err);
	failures += check_int(err, 0);

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

	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

ECHECK_TEST_MAIN(test_ehht_clear)
