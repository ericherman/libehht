#include <echeck.h>
#include <stdio.h>		/* fprintf */
#include <string.h>		/* strlen */
#include <errno.h>

#include "ehht.h"

int test_ehht_new()
{
	int failures = 0;
	struct ehht_s *table;
	size_t num_buckets = 35;

	/* msg = "test_ehht_new"; */
	table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);

	if (table == NULL) {
		++failures;
	}

	failures += check_unsigned_int_m(table->size(table), 0, "ehht_size");
	failures +=
	    check_unsigned_int_m(table->num_buckets(table), num_buckets,
				 "ehht_num_buckets");

	ehht_free(table);
	return failures;
}

int test_ehht_put_get_remove()
{
	int failures = 0;
	struct ehht_s *table;
	const char *key;
	size_t num_buckets = 3;
	void *val, *old_val;
	char buf[1000];

	table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);

	key = "key1";

	failures += check_int(table->has_key(table, key, strlen(key)), 0);
	failures += check_int(table->has_key(table, key, strlen(key)), 0);

	val = table->get(table, key, strlen(key));

	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get while empty");

	val = "foo";
	old_val = table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m((unsigned long)old_val, (unsigned long)NULL,
				  "ehht_put while empty");
	failures += check_int(table->has_key(table, key, strlen(key)), 1);

	val = table->get(table, key, strlen(key));
	if (val == NULL) {
		failures += 1;
	} else {
		failures += check_str_m((const char *)val, "foo", "ehht_get");
	}
	old_val = table->put(table, key, strlen(key), "bar");
	failures += check_str_m((const char *)old_val, val, "ehht_put over");

	val = table->get(table, "baz", strlen("baz"));
	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get bogus");

	val = table->get(table, key, strlen(key));
	failures += check_str_m((const char *)val, "bar", "ehht_get replaced");

	key = "two";
	table->put(table, key, strlen(key), "2");
	key = "three";
	table->put(table, key, strlen(key), "3");
	key = "four";
	table->put(table, key, strlen(key), "4");
	key = "ping";
	table->put(table, key, strlen(key), "pong");
	key = "whiz";
	table->put(table, key, strlen(key), "bang");
	key = "seven";
	table->put(table, key, strlen(key), "7");
	key = "eight";
	table->put(table, key, strlen(key), "8");
	key = "nine";
	table->put(table, key, strlen(key), "9");
	failures += check_unsigned_int_m(table->size(table), 9, "ehht_size");

	key = "ping";
	failures += check_int(table->has_key(table, key, strlen(key)), 1);
	old_val = table->remove(table, key, strlen(key));
	failures += check_str_m((const char *)old_val, "pong", "ehht_remove");
	failures += check_int(table->has_key(table, key, strlen(key)), 0);
	val = table->get(table, "ping", strlen(key));
	if (val != NULL) {
		++failures;
	}
	failures += check_int(table->has_key(table, key, strlen(key)), 0);
	failures += check_unsigned_int_m(table->size(table), 8, "remove size");
	old_val = table->remove(table, "bogus", strlen("bogus"));
	failures += check_unsigned_int_m(table->size(table), 8, "remove size");
	if (old_val != NULL) {
		++failures;
	}

	table->remove(table, "key1", strlen("key1"));
	table->remove(table, "two", strlen("two"));
	table->remove(table, "nine", strlen("nine"));
	table->remove(table, "three", strlen("three"));
	table->remove(table, "eight", strlen("eight"));
	table->remove(table, "four", strlen("four"));
	table->remove(table, "seven", strlen("seven"));

	table->to_string(table, buf, 1000);

	failures += check_unsigned_int_m(table->size(table), 1, buf);
	old_val = table->remove(table, "whiz", strlen("whiz"));
	failures += check_str_m((const char *)old_val, "bang", "last remove");
	failures += check_unsigned_int_m(table->size(table), 0, "last size");

	ehht_free(table);
	return failures;
}

int foreach_thing(struct ehht_key_s each_key, void *each_val, void *context)
{

	unsigned int *i;
	const char *val;

	i = (unsigned int *)context;
	val = (const char *)each_val;

	if (each_key.len > 2) {
		*i += (strlen(each_key.key) + strlen(val));
	}

	return 0;
}

int test_ehht_foreach_element()
{
	int failures = 0;
	struct ehht_s *table;
	unsigned int actual, expected, num_buckets = 5;

	table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);

	if (table == NULL) {
		++failures;
	}

	table->put(table, "g", 1, "wiz");
	table->put(table, "foo", 3, "bar");
	table->put(table, "whiz", 4, "bang");
	table->put(table, "love", 4, "backend development");

	expected = 3 + 3 + 4 + 4 + 4 + 19;
	actual = 0;

	table->for_each(table, foreach_thing, &actual);

	failures += check_unsigned_long_m(actual, expected, "foreach");

	failures += check_unsigned_int_m(table->size(table), 4, "ehht_size");

	ehht_free(table);
	return failures;
}

struct mem_context {
	unsigned allocs;
	unsigned alloc_bytes;
	unsigned frees;
	unsigned free_bytes;
	unsigned fails;
	unsigned malloc_multiplier;
};

void *test_malloc(size_t size, void *context)
{
	struct mem_context *ctx = (struct mem_context *)context;
	++ctx->allocs;
	ctx->alloc_bytes += size;
	if (ctx->malloc_multiplier) {
		size *= ctx->malloc_multiplier;
	}
	return malloc(size);
}

void test_free(void *ptr, size_t size, void *context)
{
	struct mem_context *ctx = (struct mem_context *)context;
	++ctx->frees;
	ctx->free_bytes += size;
	if (ptr == NULL) {
		++ctx->fails;
	}
	free(ptr);
}

int test_ehht_clear()
{
	int failures = 0;
	struct ehht_s *table;
	size_t num_buckets = 5;

	size_t i, count, items_written;
	size_t report[10];
	struct mem_context ctx = { 0, 0, 0, 0, 0, 0 };

	table = ehht_new(num_buckets, NULL, test_malloc, test_free, &ctx);

	table->put(table, "g", 1, "wiz");
	table->put(table, "foo", 3, "bar");
	table->put(table, "whiz", 4, "bang");
	table->put(table, "love", 4, "backend development");

	failures += check_unsigned_int_m(table->size(table), 4, "ehht_size");

	items_written = table->report(table, report, 10);
	failures += check_size_t_m(items_written, num_buckets, "ehht_report 1");
	count = 0;
	for (i = 0; i < items_written; ++i) {
		count += report[i];
	}
	failures += check_size_t_m(count, 4, "ehht_report 1");

	table->clear(table);

	failures += check_unsigned_int_m(table->size(table), 0, "clear");

	items_written = table->report(table, report, 10);
	failures +=
	    check_size_t_m(items_written, num_buckets, "ehht_report full");
	count = 0;
	for (i = 0; i < items_written; ++i) {
		count += report[i];
	}
	failures += check_size_t_m(count, 0, "ehht_report empty");

	ehht_free(table);

	failures += check_unsigned_int_m(ctx.allocs > 0, 1, "ctx.allocs");
	failures += check_unsigned_int_m(ctx.frees, ctx.allocs, "alloc/free");
	failures += check_unsigned_int_m(ctx.alloc_bytes > 0, 1, "bytes > 0");
	failures +=
	    check_unsigned_int_m(ctx.free_bytes, ctx.alloc_bytes, "bytes");
	failures += check_unsigned_int_m(ctx.fails, 0, "free NULL pointers");

	return failures;
}

int test_ehht_keys()
{
	int failures = 0;
	struct ehht_s *table;
	struct ehht_keys_s *ks;
	int allocate_copies;
	size_t i, j, num_buckets;
	size_t *found;
	const char *e_keys[] = { "foo", "bar", "whiz", "bang", NULL };

	num_buckets = 3;

	for (allocate_copies = 0; allocate_copies < 2; ++allocate_copies) {

		table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);

		for (i = 0; e_keys[i] != NULL; ++i) {
			table->put(table, e_keys[i], strlen(e_keys[i]), NULL);
		}

		found = calloc(sizeof(size_t), table->size(table));
		if (!found) {
			fprintf(stderr, "could not allocate found array\n");
			return 1;
		}

		ks = table->keys(table, allocate_copies);
		failures += check_size_t(ks->len, table->size(table));
		for (i = 0; e_keys[i] != NULL; ++i) {
			for (j = 0; j < ks->len && !found[i]; ++j) {
				if (strcmp(e_keys[i], ks->keys[j].key) == 0) {
					found[i] = 1;
				}
			}
		}
		for (i = 0; i < ks->len; ++i) {
			if (!found[i]) {
				failures += check_str("", e_keys[i]);
			}
		}

		table->free_keys(table, ks);
		free(found);

		ehht_free(table);
	}

	return failures;
}

int test_ehht_resize()
{
	int failures = 0;
	struct ehht_s *table;
	size_t i, x, items_written, num_buckets, big_bucket1, big_bucket2;
	size_t report[10];
	char buf[24];

	num_buckets = 4;
	table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);

	if (table == NULL) {
		return ++failures;
	}

	failures += check_unsigned_int_m(table->num_buckets(table), num_buckets,
					 "init num_buckets");

	x = num_buckets;
	for (i = 0; i < x; ++i) {
		sprintf(buf, "_%lu_", (unsigned long)i);
		table->put(table, buf, strlen(buf), NULL);
	}

	items_written = table->report(table, report, 10);
	failures += check_unsigned_int_m(items_written, num_buckets,
					 "first items_written");
	big_bucket1 = 0;
	for (i = 0; i < items_written; ++i) {
		if (report[i] > big_bucket1) {
			big_bucket1 = report[i];
		}
	}

	num_buckets *= 2;
	table->resize(table, num_buckets);

	items_written = table->report(table, report, 10);
	failures += check_unsigned_int_m(items_written, num_buckets,
					 "second items_written");
	big_bucket2 = 0;
	for (i = 0; i < items_written; ++i) {
		if (report[i] > big_bucket1) {
			big_bucket2 = report[i];
		}
	}

	sprintf(buf, "max1: %lu, max2: %lu", (unsigned long)big_bucket1,
		(unsigned long)big_bucket2);
	failures += check_unsigned_int_m((big_bucket2 <= big_bucket1), 1, buf);

	for (i = 0; i < x; ++i) {
		sprintf(buf, "_%lu_", (unsigned long)i);
		failures +=
		    check_size_t_m(table->has_key(table, buf, strlen(buf)), 1,
				   buf);
	}

	ehht_free(table);
	return failures;
}

int test_out_of_memory()
{
	int failures = 0;
	struct ehht_s *table;
	char buf[40];
	size_t i;
	struct mem_context ctx = { 0, 0, 0, 0, 0, 0 };

	ctx.malloc_multiplier = 8 * 1024;
	table = ehht_new(0, NULL, test_malloc, test_free, &ctx);

	if (table == NULL) {
		++failures;
		return failures;
	}

	check_int(errno, 0);

	for (i = 0; i == table->size(table); ++i) {
		if (i % 256 == 0) {
			fprintf(stderr, "\r%lu,", (unsigned long)i);
		}
		sprintf(buf, "_%lu_", (unsigned long)i);
		table->put(table, buf, strlen(buf), NULL);
		if (i % 256 == 0) {
			fprintf(stderr, "%lu ...",
				(unsigned long)table->size(table));
		}
	}
	fprintf(stderr, "\n");
	table->clear(table);
	check_int(errno, ENOMEM);
	errno = 0;

	ehht_free(table);
	return failures;
}

int main(int argc, char **argv)
{
	int failures = 0;
	int test_oom = (argc > 1) ? atoi(argv[1]) : 0;

	failures += test_ehht_new();
	failures += test_ehht_put_get_remove();
	failures += test_ehht_foreach_element();
	failures += test_ehht_clear();
	failures += test_ehht_keys();
	failures += test_ehht_resize();

	if (test_oom) {
		failures += test_out_of_memory();
	}

	if (failures) {
		fprintf(stderr, "%d failures in total\n", failures);
	}
	return failures;
}
