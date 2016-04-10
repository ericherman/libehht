#include <echeck.h>
#include <stdio.h>		/* fprintf */
#include <string.h>		/* strlen */

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
	val = table->get(table, key, strlen(key));

	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get while empty");

	val = "foo";
	old_val = table->put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m((unsigned long)old_val, (unsigned long)NULL,
				  "ehht_put while empty");

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
	old_val = table->remove(table, key, strlen(key));
	failures += check_str_m((const char *)old_val, "pong", "ehht_remove");
	val = table->get(table, "ping", strlen(key));
	if (val != NULL) {
		++failures;
	}
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

int foreach_thing(const char *each_key, size_t each_key_len,
		  void *each_val, void *context)
{

	unsigned int *i;
	const char *val;

	i = (unsigned int *)context;
	val = (const char *)each_val;

	if (each_key_len > 2) {
		*i += (strlen(each_key) + strlen(val));
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
};

void *test_malloc(size_t size, void *context)
{
	struct mem_context *ctx = (struct mem_context *)context;
	++ctx->allocs;
	ctx->alloc_bytes += size;
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
	struct mem_context ctx = { 0, 0, 0, 0, 0 };

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
	int allocate_copies;
	size_t i, j, keys_len, filled, num_buckets;
	size_t *lens, *found;
	const char **keys;
	const char *e_keys[] = { "foo", "bar", "whiz", "bang", NULL };

	num_buckets = 3;

	for (allocate_copies = 0; allocate_copies < 2; ++allocate_copies) {

		table = ehht_new(num_buckets, NULL, NULL, NULL, NULL);

		for (i = 0; e_keys[i] != NULL; ++i) {
			table->put(table, e_keys[i], strlen(e_keys[i]), NULL);
		}

		keys_len = table->size(table);
		keys = malloc(sizeof(char *) * keys_len);
		if (!keys) {
			fprintf(stderr, "could not allocate keys array\n");
			return 1;
		}
		lens = malloc(sizeof(size_t) * keys_len);
		if (!lens) {
			fprintf(stderr, "could not allocate lens array\n");
			return 1;
		}
		found = calloc(sizeof(size_t), keys_len);
		if (!found) {
			fprintf(stderr, "could not allocate found array\n");
			return 1;
		}

		filled =
		    table->keys(table, keys, lens, keys_len, allocate_copies);
		failures += check_size_t_m(filled, keys_len, "filled");
		for (i = 0; i < keys_len; ++i) {
			for (j = 0; j < keys_len && !found[i]; ++j) {
				if (strcmp(e_keys[i], keys[j]) == 0) {
					found[i] = 1;
				}
			}
		}
		for (i = 0; i < keys_len; ++i) {
			if (!found[i]) {
				failures += check_str(e_keys[i], "");
			}
		}

		if (allocate_copies) {
			for (i = 0; i < filled; ++i) {
				free((char *)keys[i]);
			}
		}
		free(keys);
		free(lens);
		free(found);

		ehht_free(table);
	}

	return failures;
}

int main(void)
{				/* int argc, char *argv[]) */
	int failures = 0;

	failures += test_ehht_new();
	failures += test_ehht_put_get_remove();
	failures += test_ehht_foreach_element();
	failures += test_ehht_clear();
	failures += test_ehht_keys();

	if (failures) {
		fprintf(stderr, "%d failures in total\n", failures);
	}
	return failures;
}
