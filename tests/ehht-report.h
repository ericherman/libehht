/* ehht-report.h: reporting interface for a simple OO hashtable
   Copyright (C) 2016, 2017, 2018 Eric Herman <eric@freesa.org>

   This work is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or (at
   your option) any later version.

   This work is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

	https://www.gnu.org/licenses/lgpl-3.0.txt
	https://www.gnu.org/licenses/gpl-3.0.txt
*/
#ifndef EHHT_REPORT_H
#define EHHT_REPORT_H

#ifdef __cplusplus
#define Ehht_report_begin_C_functions extern "C" {
#define Ehht_report_end_C_functions }
#else
#define Ehht_report_begin_C_functions
#define Ehht_report_end_C_functions
#endif

#include "../src/ehht.h"

Ehht_report_begin_C_functions
#undef Ehht_report_begin_C_functions
/** reports table's key.hashcode values distributed over sizes_len buckets
 * returns total items across all buckets  */
size_t ehht_distribution_report(struct ehht_s *table, size_t *sizes,
				size_t sizes_len)
{
	struct ehht_keys_s *keys;
	int copy_keys;
	size_t i, bucket;

	for (i = 0; i < sizes_len; ++i) {
		sizes[i] = 0;
	}

	copy_keys = 0;
	keys = table->keys(table, copy_keys);
	for (i = 0; i < keys->len; ++i) {
		bucket = table->bucket_for_key(table, keys->keys[i].str,
					       keys->keys[i].len);
		++sizes[bucket];
	}
	table->free_keys(table, keys);

	return i;
}

Ehht_report_end_C_functions
#undef Ehht_report_end_C_functions
#endif /* EHHT_REPORT_H */
