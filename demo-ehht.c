#include <stdio.h>		/* fprintf fscanf printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strlen */

#include "ehht.h"

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
	FILE *file;

	num_buckets = (argc > 1) ? (size_t)atoi(argv[1]) : 0;

	default_table = ehht_new(num_buckets, NULL);
	if (default_table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		return 1;
	}
	leveldb_table = ehht_new(num_buckets, &leveldb_hash);
	if (leveldb_table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		return 1;
	}

	file = fopen("COPYING.GPL3", "r");
	while (!feof(file)) {
		fscanf(file, WORD_SCANF_FMT, buf);
		ehht_put(default_table, buf, strlen(buf), NULL);
		ehht_put(leveldb_table, buf, strlen(buf), NULL);
	}
	fclose(file);

	num_buckets = ehht_num_buckets(default_table);
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
