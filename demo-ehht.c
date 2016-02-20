#include <stdio.h>		/* fprintf fscanf printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strlen */

#include "ehht.h"

#ifndef MAKE_VALGRIND_HAPPY
#define MAKE_VALGRIND_HAPPY 0
#endif

#define MAX_WORD_LEN 80
#define WORD_SCANF_FMT "%79s"

/* from leveldb, a murmur-lite */
static unsigned int leveldb_bloom_hash(const char *b, size_t len)
{
	const unsigned int seed = 0xbc9f1d34;
	const unsigned int m = 0xc6a4a793;

	unsigned int h = seed ^ len * m;
	while (len >= 4) {
		h += b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
		h *= m;
		h ^= h >> 16;
		b += 4;
		len -= 4;
	}

	switch (len) {
	case 3:
		h += b[2] << 16;
	case 2:
		h += b[1] << 8;
	case 1:
		h += b[0];
		h *= m;
		h ^= h >> 24;
	}
	return h;
}

int main(int argc, char *argv[])
{
	struct ehht_s *default_table, *bloom_table;
	size_t i, num_buckets, *default_sizes, *bloom_sizes;
	char buf[MAX_WORD_LEN];
	FILE *file;

	num_buckets = (argc > 1) ? (size_t)atoi(argv[1]) : 0;

	default_table = ehht_new(num_buckets, NULL);
	if (default_table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		return 1;
	}
	bloom_table = ehht_new(num_buckets, &leveldb_bloom_hash);
	if (bloom_table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		return 1;
	}

	file = fopen("COPYING.GPL3", "r");
	while (!feof(file)) {
		fscanf(file, WORD_SCANF_FMT, buf);
		ehht_put(default_table, buf, strlen(buf), NULL);
		ehht_put(bloom_table, buf, strlen(buf), NULL);
	}
	fclose(file);

	num_buckets = ehht_num_buckets(default_table);
	default_sizes = (size_t *)malloc(sizeof(size_t *) * num_buckets);
	if (default_sizes == NULL) {
		fprintf(stderr, "malloc returned NULL");
		return 2;
	}
	bloom_sizes = (size_t *)malloc(sizeof(size_t *) * num_buckets);
	if (bloom_sizes == NULL) {
		fprintf(stderr, "malloc returned NULL");
		return 2;
	}
	ehht_distribution_report(default_table, default_sizes, num_buckets);
	ehht_distribution_report(bloom_table, bloom_sizes, num_buckets);
	for (i = 0; i < num_buckets; ++i) {
		printf("%lu, %lu\n", (unsigned long)default_sizes[i],
		       (unsigned long)bloom_sizes[i]);
	}

	if (MAKE_VALGRIND_HAPPY) {
		ehht_free(default_table);
		ehht_free(bloom_table);
		free(default_sizes);
		free(bloom_sizes);
	}
	return 0;
}
