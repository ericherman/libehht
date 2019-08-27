/* demo-ehht.c: demo of a simple OO hashtable
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
#include <stdio.h>		/* fprintf fscanf printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strlen */
#include <errno.h>		/* errno strerror */
#include <stdint.h>		/* int32_t */

#include "../src/ehht.h"
#include "../tests/ehht-report.h"

#ifndef MAKE_VALGRIND_HAPPY
#define MAKE_VALGRIND_HAPPY 0
#endif

#define MAX_WORD_LEN 80
#define WORD_SCANF_FMT "%79s"

unsigned int leveldb_hash(const char *data, size_t len);
unsigned int djb2_hash(const char *data, size_t len);
unsigned int ehht_kr2_hashcode(const char *str, size_t str_len);
int32_t jumphash(uint64_t key, int32_t num_buckets);

/* global for easy use of jumphash-ing */
size_t num_buckets;

unsigned int djb2_jump(const char *data, size_t len)
{
	return jumphash(djb2_hash(data, len), num_buckets);
}

unsigned int ehht_jump(const char *data, size_t len)
{
	return jumphash(ehht_kr2_hashcode(data, len), num_buckets);
}

#define new_table(target, buckets, hash_pfunc) \
	do { \
		(*target) = ehht_new(buckets, hash_pfunc, NULL, NULL, NULL); \
		if (!(*target)) { \
			fprintf(stderr, "%s:%d: ehht_new returned NULL", \
				__FILE__, __LINE__); \
			return 1; \
		} \
	} while (0)

#define new_sizes(target, target_sizes) \
	do { \
		actual_buckets = target->num_buckets(target); \
		if (num_buckets != actual_buckets) { \
			fprintf(stderr, "%s:%d: %lu != %lu\n", \
				__FILE__, __LINE__, \
				num_buckets, actual_buckets); \
			return 3; \
		} \
		size = sizeof(size_t *) * num_buckets; \
		target_sizes = (size_t *)malloc(size); \
		if (target_sizes == NULL) { \
			fprintf(stderr, "%s: %d: malloc(%lu) returned NULL", \
				__FILE__, __LINE__, (unsigned long)size); \
			return 4; \
		} \
	} while (0)

int main(int argc, char *argv[])
{
	struct ehht_s *default_table, *leveldb_table, *djb2_table;
	struct ehht_s *ehht_jump_table, *djb2_jump_table;
	size_t i, size, actual_buckets;
	size_t *default_sizes, *leveldb_sizes, *djb2_sizes;
	size_t *ehht_jump_sizes, *djb2_jump_sizes;
	char buf[MAX_WORD_LEN];
	char *file_name;
	int save_errno;
	FILE *file;

	num_buckets = (argc > 1) ? (size_t)atoi(argv[1]) : 0;
	file_name = (argc > 2) ? argv[2] : "COPYING";

	new_table(&default_table, num_buckets, NULL);
	new_table(&leveldb_table, num_buckets, &leveldb_hash);
	new_table(&djb2_table, num_buckets, &djb2_hash);
	new_table(&ehht_jump_table, num_buckets, &ehht_jump);
	new_table(&djb2_jump_table, num_buckets, &djb2_jump);

	errno = 0;
	file = fopen(file_name, "r");
	if (!file) {
		save_errno = errno;
		fprintf(stderr, "%s: %s\n", strerror(save_errno), file_name);
		return 2;

	}
	while (!feof(file)) {
		if (!fscanf(file, WORD_SCANF_FMT, buf)) {
			continue;
		}
		size = strlen(buf);

		default_table->put(default_table, buf, size, NULL);
		leveldb_table->put(leveldb_table, buf, size, NULL);
		djb2_table->put(djb2_table, buf, size, NULL);
		ehht_jump_table->put(ehht_jump_table, buf, size, NULL);
		djb2_jump_table->put(djb2_jump_table, buf, size, NULL);
	}
	fclose(file);

	new_sizes(default_table, default_sizes);
	new_sizes(leveldb_table, leveldb_sizes);
	new_sizes(djb2_table, djb2_sizes);
	new_sizes(ehht_jump_table, ehht_jump_sizes);
	new_sizes(djb2_jump_table, djb2_jump_sizes);

	ehht_distribution_report(default_table, default_sizes, num_buckets);
	ehht_distribution_report(leveldb_table, leveldb_sizes, num_buckets);
	ehht_distribution_report(djb2_table, djb2_sizes, num_buckets);
	ehht_distribution_report(ehht_jump_table, ehht_jump_sizes, num_buckets);
	ehht_distribution_report(djb2_jump_table, djb2_jump_sizes, num_buckets);

	for (i = 0; i < num_buckets; ++i) {
		printf("%lu, %lu, %lu, %lu, %lu\n",
		       (unsigned long)default_sizes[i],
		       (unsigned long)leveldb_sizes[i],
		       (unsigned long)djb2_sizes[i],
		       (unsigned long)ehht_jump_sizes[i],
		       (unsigned long)djb2_jump_sizes[i]
		    );
	}

	if (MAKE_VALGRIND_HAPPY) {
		ehht_free(default_table);
		ehht_free(leveldb_table);
		free(default_sizes);
		free(leveldb_sizes);
	}
	return 0;
}
