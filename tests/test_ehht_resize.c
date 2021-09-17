/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_resize.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "ehht-report.h"
#include "echeck.h"

unsigned test_ehht_buckets_resize(void)
{
	const size_t bytes_len = 250 * sizeof(size_t);
	unsigned char bytes[250 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = NULL;
	size_t i, x, items_written, num_buckets, big_bucket1, big_bucket2;
	const size_t report_len = 10;
	size_t report[10];
	const size_t buf_len = 24;
	char buf[24];
	const size_t msg_len = 40;
	char msg[40];
	int err = 0;
	const size_t logbuf_len = 250;
	char logbuf[250];
	struct eembed_log slog;
	struct eembed_str_buf str_buf;
	struct eembed_log *log = NULL;

	if (!EEMBED_HOSTED) {
		eembed_memset(bytes, 0x00, bytes_len);
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	eembed_memset(msg, 0x00, msg_len);
	eembed_memset(buf, 0x00, buf_len);
	eembed_memset(logbuf, 0x00, logbuf_len);
	log = eembed_char_buf_log_init(&slog, &str_buf, logbuf, logbuf_len);
	if (check_ptr_not_null(log)) {
		++failures;
		goto test_ehht_buckets_resize_end;
	}

	num_buckets = 4;
	table = ehht_new_custom(num_buckets, NULL, NULL, log);

	if (table == NULL) {
		++failures;
		goto test_ehht_buckets_resize_end;
	}

	failures += check_unsigned_int_m(ehht_buckets_size(table), num_buckets,
					 "init num_buckets");

	x = num_buckets;
	for (i = 0; i < x; ++i) {
		eembed_ulong_to_str(buf, buf_len, i);
		table->put(table, buf, eembed_strlen(buf), NULL, &err);
		failures += check_int(err, 0);
	}

	items_written = ehht_distribution_report(table, report, report_len);
	failures += check_unsigned_int_m(items_written, num_buckets,
					 "first items_written");
	big_bucket1 = 0;
	for (i = 0; i < items_written; ++i) {
		if (report[i] > big_bucket1) {
			big_bucket1 = report[i];
		}
	}

	num_buckets *= 2;
	ehht_buckets_resize(table, num_buckets);

	items_written = ehht_distribution_report(table, report, report_len);
	failures +=
	    check_unsigned_int_m(items_written, table->size(table),
				 "second items_written");

	big_bucket2 = 0;
	for (i = 0; i < items_written; ++i) {
		if (report[i] > big_bucket1) {
			big_bucket2 = report[i];
		}
	}

	msg[0] = '\0';
	eembed_strcat(msg, "max1: ");
	eembed_strcat(msg, eembed_ulong_to_str(buf, buf_len, big_bucket1));
	eembed_strcat(msg, "max2: ");
	eembed_strcat(msg, eembed_ulong_to_str(buf, buf_len, big_bucket2));
	failures += check_unsigned_int_m((big_bucket2 > big_bucket1), 0, msg);
	for (i = 0; i < x; ++i) {
		eembed_ulong_to_str(buf, buf_len, i);
		failures +=
		    check_size_t_m(table->has_key
				   (table, buf, eembed_strlen(buf)), 1, buf);
	}

	/* do not resize if there is not memory to do so */
	ehht_buckets_resize(table, (SIZE_MAX / 64));
	failures +=
	    check_unsigned_int_m(ehht_buckets_size
				 (table), num_buckets,
				 "after too large num_buckets");
	ehht_free(table);
	failures += check_str_contains(logbuf, "Error 4:");

test_ehht_buckets_resize_end:
	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

ECHECK_TEST_MAIN(test_ehht_buckets_resize)
