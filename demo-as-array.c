#include <stdio.h>		/* fprintf fscanf printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strlen */
#include <strings.h>		/* strdup */

#include "ehht.h"

#define MAKE_VALGRIND_HAPPY 1

struct kv_s {
	struct ehht_key_s key;
	void *val;
};

struct kva_s {
	size_t pos;
	size_t len;
	struct kv_s *kvs;
};

int to_array(struct ehht_key_s key, void *val, void *ctx)
{
	struct kva_s *kva;
	char *str;

	kva = ctx;

	str = strdup(key.str);
	if (strlen(str) != key.len) {
		fprintf(stderr, "len(%s) != len(%s) ?\n", key.str, str);
	}
	key.str = str;

	kva->kvs[kva->pos].key = key;
	kva->kvs[kva->pos].val = val;

	++kva->pos;

	return 0;
}

int comp_key_lens(const void *a, const void *b)
{
	const struct kv_s *l, *r;
	l = a;
	r = b;

	if (l->key.len == r->key.len) {
		return 0;
	}
	return l->key.len > r->key.len ? -1 : 1;
}

int main(int argc, char *argv[])
{
	struct ehht_s *table;
	size_t i, num_buckets;
	struct kva_s *kva;

	num_buckets = (argc > 1) ? atoi(argv[1]) : 0;

	table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);
	if (table == NULL) {
		fprintf(stderr, "ehht_new returned NULL");
		exit(EXIT_FAILURE);
	}

	table->put(table, "foo", strlen("foo"), "f");
	table->put(table, "bar", strlen("bar"), "b");
	table->put(table, "to", strlen("to"), "t");
	table->put(table, "long long int", strlen("long long int"), "l");
	table->put(table, "a", strlen("a"), "a");
	table->put(table, "whiz", strlen("whiz"), "w");
	table->put(table, "bang", strlen("bang"), "b");

	kva = malloc(sizeof(struct kva_s));
	kva->len = 1 + table->size(table);
	kva->pos = 0;
	kva->kvs = calloc(sizeof(struct kv_s), kva->len);

	table->for_each(table, to_array, kva);

	qsort(kva->kvs, kva->pos, sizeof(struct kv_s), comp_key_lens);

	for (i = 0; i < kva->pos; ++i) {
		printf("kva->kvs[%u].str=%s"
		       ", kva->kvs[%u].len:%u"
		       ", kva->vals[%u].val:%s\n",
		       (unsigned)i, kva->kvs[i].key.str,
		       (unsigned)i, (unsigned)kva->kvs[i].key.len,
		       (unsigned)i, (char *)kva->kvs[i].val);
	}

	if (MAKE_VALGRIND_HAPPY) {
		for (i = 0; i < kva->pos; ++i) {
			free((char *)kva->kvs[i].key.str);
		}
		free(kva->kvs);
		free(kva);
		ehht_free(table);
	}
	return 0;
}
