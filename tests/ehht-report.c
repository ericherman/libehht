/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehht-report.c: reporting function for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht-report.h"

size_t ehht_distribution_report(struct ehht *table, size_t *sizes,
				size_t sizes_len)
{
	struct ehht_keys *keys;
	int copy_keys;
	size_t i, bucket;

	if (!table || !sizes || !sizes_len) {
		return 0;
	}

	for (i = 0; i < sizes_len; ++i) {
		sizes[i] = 0;
	}

	copy_keys = 0;
	keys = table->keys(table, copy_keys);
	if (!keys) {
		return 0;
	}
	for (i = 0; i < keys->len; ++i) {
		bucket = ehht_bucket_for_key(table, keys->keys[i].str,
					     keys->keys[i].len);
		++sizes[bucket];
	}
	table->free_keys(table, keys);

	return i;
}
