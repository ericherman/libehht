#include "test-ehht.h"

int foreach_thing(struct ehht_key_s each_key, void *each_val, void *context)
{

	unsigned int *i;
	const char *val;

	i = (unsigned int *)context;
	val = (const char *)each_val;

	if (each_key.len > 2) {
		*i += (strlen(each_key.str) + strlen(val));
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

TEST_EHHT_MAIN(test_ehht_foreach_element())
