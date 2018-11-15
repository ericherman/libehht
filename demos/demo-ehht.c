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

#include "ehht.h"
#include "ehht-report.h"

#ifndef MAKE_VALGRIND_HAPPY
#define MAKE_VALGRIND_HAPPY 0
#endif

#define MAX_WORD_LEN 80
#define WORD_SCANF_FMT "%79s"

unsigned int leveldb_hash(const char *data, size_t n);

int main(int argc, char *argv[])
{
	struct ehht_s *default_table, *leveldb_table;
	size_t i, num_buckets, *default_sizes, *leveldb_sizes;
	char buf[MAX_WORD_LEN];
	char *file_name;
	int save_errno;
	FILE *file;

	num_buckets = (argc > 1) ? (size_t)atoi(argv[1]) : 0;
	file_name = (argc > 2) ? argv[2] : "COPYING";

	default_table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);
	if (default_table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		return 1;
	}
	leveldb_table = ehht_new(num_buckets, &leveldb_hash, NULL, NULL, NULL);
	if (leveldb_table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		return 1;
	}

	errno = 0;
	file = fopen(file_name, "r");
	if (!file) {
		save_errno = errno;
		fprintf(stderr, "%s: %s\n", strerror(save_errno), file_name);
		return 1;

	}
	while (!feof(file)) {
		if (!fscanf(file, WORD_SCANF_FMT, buf)) {
			continue;
		}
		default_table->put(default_table, buf, strlen(buf), NULL);
		leveldb_table->put(leveldb_table, buf, strlen(buf), NULL);
	}
	fclose(file);

	num_buckets = default_table->num_buckets(default_table);
	default_sizes = (size_t *)malloc(sizeof(size_t *) * num_buckets);
	if (default_sizes == NULL) {
		fprintf(stderr, "malloc returned NULL");
		return 2;
	}
	leveldb_sizes = (size_t *)malloc(sizeof(size_t *) * num_buckets);
	if (leveldb_sizes == NULL) {
		fprintf(stderr, "malloc returned NULL");
		return 2;
	}
	ehht_distribution_report(default_table, default_sizes, num_buckets);
	ehht_distribution_report(leveldb_table, leveldb_sizes, num_buckets);
	for (i = 0; i < num_buckets; ++i) {
		printf("%lu, %lu\n", (unsigned long)default_sizes[i],
		       (unsigned long)leveldb_sizes[i]);
	}

	if (MAKE_VALGRIND_HAPPY) {
		ehht_free(default_table);
		ehht_free(leveldb_table);
		free(default_sizes);
		free(leveldb_sizes);
	}
	return 0;
}
