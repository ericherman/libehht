#include <stdio.h>		/* fprintf fscanf printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strlen */
#include <strings.h>		/* strdup */

#include "ehht.h"

#define MAKE_VALGRIND_HAPPY 1

struct kva_s {
	size_t pos;
	size_t len;
	char **keys;
	size_t *lens;
	void **vals;
};

int to_array(const char *key, size_t len, void *val, void *ctx)
{
	struct kva_s *kva;
	char *str;

	kva = ctx;

	str = strdup(key);
	if (strlen(str) != len) {
		fprintf(stderr, "len(%s) != len(%s) ?\n", key, str);
	}

	kva->keys[kva->pos] = str;
	kva->lens[kva->pos] = strlen(str);
	kva->vals[kva->pos] = val;

	++kva->pos;

	return 0;
}

int comp_key_lens(const void *a, const void *b)
{
	size_t llen, rlen;

	llen = strlen(a);
	rlen = strlen(b);
	if (llen == rlen) {
		return 0;
	}
	return llen > rlen ? -1 : 1;
}

int main(int argc, char *argv[])
{
	struct ehht_s *table;
	size_t i, num_buckets;
	struct kva_s kva;

	num_buckets = (argc > 1) ? atoi(argv[1]) : 0;

	table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);
	if (table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		exit(EXIT_FAILURE);
	}

	table->put(table, "foo", strlen("foo"), "f");
	table->put(table, "bar", strlen("bar"), "b");
	table->put(table, "to", strlen("to"), "t");
	table->put(table, "whiz", strlen("whiz"), "w");
	table->put(table, "bang", strlen("bang"), "b");

	kva.len = 1 + table->size(table);
	kva.pos = 0;
	kva.keys = calloc(sizeof(char *), kva.len);
	kva.lens = calloc(sizeof(size_t), kva.len);
	kva.vals = calloc(sizeof(void *), kva.len);

	table->for_each(table, to_array, &kva);

	qsort(kva.keys, kva.len, sizeof(char *), comp_key_lens);

	for (i = 0; i < kva.pos; ++i) {
		printf("kva.keys[%u]=%s, kva.lens[%u]:%u, kva.vals[%i]:%s\n",
		       (unsigned)i, kva.keys[i], (unsigned)i, kva.lens[i],
		       (unsigned)i, (char *)kva.vals[i]);
	}

	if (MAKE_VALGRIND_HAPPY) {
		for (i = 0; i < kva.pos; ++i) {
			free(kva.keys[i]);
		}
		free(kva.keys);
		free(kva.lens);
		free(kva.vals);
		ehht_free(table);
	}
	return 0;
}
