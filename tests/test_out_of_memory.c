/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_out_of_memory.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "echeck.h"

unsigned test_out_of_memory_construction(uint64_t allocs_to_fail)
{
	const size_t bytes_len = 500 * sizeof(size_t);
	unsigned char bytes[500 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = 0;
	const size_t msg_len = 40;
	char msg[40];

	struct echeck_err_injecting_context ctx;
	struct eembed_allocator wrap;
	struct eembed_allocator *real = NULL;

	const size_t logbuf_len = 250;
	char logbuf[250];
	struct eembed_log slog;
	struct eembed_str_buf str_buf;
	struct eembed_log *log = NULL;

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	msg[0] = '\0';
	logbuf[0] = '\0';

	real = eembed_global_allocator;
	if (allocs_to_fail == 0) {
		log = eembed_err_log;
	} else {
		log =
		    eembed_char_buf_log_init(&slog, &str_buf, logbuf,
					     logbuf_len);
		if (check_ptr_not_null(log)) {
			++failures;
			goto test_out_of_memory_construction_end;
		}
	}

	echeck_err_injecting_allocator_init(&wrap, real, &ctx, eembed_err_log);

	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	table = ehht_new_custom(0, NULL, &wrap, log);
	if (table) {
		if (allocs_to_fail) {
			failures += check_int(allocs_to_fail, 0);
		}
		ehht_free(table);
	} else if (!allocs_to_fail) {
		eembed_ulong_to_str(msg, msg_len, allocs_to_fail);
		failures += check_int_m(allocs_to_fail > 0 ? 1 : 0, 1, msg);
	}

	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");

test_out_of_memory_construction_end:
	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

unsigned test_out_of_memory_put(uint64_t allocs_to_fail)
{
	const size_t bytes_len = 500 * sizeof(size_t);
	unsigned char bytes[500 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = NULL;
	size_t i = 0;
	size_t num_buckets = 0;
	int err = 0;
	const size_t buf_len = 80;
	char buf[80];
	char msg[40];

	struct echeck_err_injecting_context ctx;
	struct eembed_allocator wrap;
	struct eembed_allocator *real = NULL;

	const size_t logbuf_len = 250;
	char logbuf[250];
	struct eembed_log slog;
	struct eembed_str_buf str_buf;
	struct eembed_log *log = NULL;

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	real = eembed_global_allocator;
	msg[0] = '\0';
	buf[0] = '\0';
	logbuf[0] = '\0';
	if (allocs_to_fail == 0) {
		log = eembed_err_log;
	} else {
		log =
		    eembed_char_buf_log_init(&slog, &str_buf, logbuf,
					     logbuf_len);
		if (check_ptr_not_null(log)) {
			++failures;
			goto test_out_of_memory_put_end;
		}
	}

	echeck_err_injecting_allocator_init(&wrap, real, &ctx, eembed_err_log);

	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	num_buckets = 5;
	table = ehht_new_custom(num_buckets, NULL, &wrap, log);
	if (!table) {
		failures += check_ptr_not_null(table);
		goto test_out_of_memory_put_end;
	}

	err = 0;
	for (i = 0; i < (num_buckets * 3); ++i) {
		eembed_ulong_to_str(buf, buf_len, i);
		table->put(table, buf, eembed_strlen(buf), NULL, &err);
		if (err && !allocs_to_fail) {
			eembed_strcpy(msg, buf);
			eembed_strcat(msg, ":");

			eembed_strcat(msg, " err: ");
			eembed_ulong_to_str(buf, buf_len, err);
			eembed_strcat(msg, buf);

			eembed_strcat(msg, " allocs_to_fail: ");
			eembed_ulong_to_str(buf, buf_len, allocs_to_fail);
			eembed_strcat(msg, buf);
			failures += check_int_m(1, 0, msg);
		}
	}

	ehht_free(table);

	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");

test_out_of_memory_put_end:
	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

unsigned test_out_of_memory_keys(uint64_t allocs_to_fail)
{
	const size_t bytes_len = 500 * sizeof(size_t);
	unsigned char bytes[500 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = NULL;
	size_t i = 0;
	int err = 0;
	int allocate_copies = 0;
	const size_t buf_len = 80;
	char buf[80];
	const size_t msg_len = 40;
	char msg[40];
	struct ehht_keys *ks;

	struct echeck_err_injecting_context ctx;
	struct eembed_allocator wrap;
	struct eembed_allocator *real = NULL;

	const size_t logbuf_len = 250;
	char logbuf[250];
	struct eembed_log slog;
	struct eembed_str_buf str_buf;
	struct eembed_log *log = NULL;

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	buf[0] = '\0';
	msg[0] = '\0';
	logbuf[0] = '\0';

	real = eembed_global_allocator;
	if (allocs_to_fail == 0) {
		log = eembed_err_log;
	} else {
		log =
		    eembed_char_buf_log_init(&slog, &str_buf, logbuf,
					     logbuf_len);
		if (check_ptr_not_null(log)) {
			++failures;
			goto test_out_of_memory_keys_end;
		}
		eembed_err_log = log;
	}
	echeck_err_injecting_allocator_init(&wrap, real, &ctx, eembed_err_log);

	ctx.attempts_to_fail_bitmask = allocs_to_fail;

	table = ehht_new_custom(0, NULL, &wrap, log);
	if (!table) {
		failures += check_ptr_not_null(table);
		goto test_out_of_memory_keys_end;
	}

	err = 0;
	for (i = 0; i < 10; ++i) {
		eembed_ulong_to_str(buf, buf_len, i);
		table->put(table, buf, eembed_strlen(buf), NULL, &err);
		failures += check_int_m(err, 0, buf);
		goto test_out_of_memory_keys_end;
	}

	err = 0;
	for (i = 0; i < 10; ++i) {
		allocate_copies = 1;
		ks = table->keys(table, allocate_copies);
		if (!ks && !allocs_to_fail) {
			eembed_ulong_to_str(msg, msg_len, i);
			eembed_strcat(msg, ":");

			eembed_strcat(msg, " ks is ");
			eembed_strcat(msg, ks ? "not null" : "null");

			eembed_strcat(msg, " allocs_to_fail: ");
			eembed_ulong_to_str(buf, buf_len, allocs_to_fail);
			eembed_strcat(msg, buf);
			failures += check_int_m(1, 0, msg);
			goto test_out_of_memory_keys_end;
		}
		if (ks) {
			table->free_keys(table, ks);
		} else {
			eembed_ulong_to_str(msg, msg_len, allocs_to_fail);
			failures +=
			    check_int_m(allocs_to_fail > 0 ? 1 : 0, 1, msg);
			++err;
		}
	}

	if (!err) {
		failures += check_int(allocs_to_fail, 0);
	}

	ehht_free(table);

	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");

test_out_of_memory_keys_end:
	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

unsigned test_out_of_memory(void)
{
	unsigned failures = 0;
	size_t i;

	failures += test_out_of_memory_construction(0);
	failures += test_out_of_memory_construction(1UL << 0);
	failures += test_out_of_memory_construction(1UL << 1);
	failures += test_out_of_memory_construction(1UL << 2);

	failures += test_out_of_memory_put(0);
	for (i = 4; i < 64; ++i) {
		failures += test_out_of_memory_put(1UL << i);
	}

	failures += test_out_of_memory_keys(0);
	failures += test_out_of_memory_keys(1UL << 23);
	failures += test_out_of_memory_keys(1UL << 24);
	failures += test_out_of_memory_keys(1UL << 25);
	failures += test_out_of_memory_keys(1UL << 27);
	failures += test_out_of_memory_keys(1UL << 28);

	return failures;
}

ECHECK_TEST_MAIN(test_out_of_memory)
