#include <echeck.h>
#include <stdio.h>		/* fprintf */
#include <string.h>		/* strlen */

#include "ehht.h"

int test_ehht_new()
{
	int failures = 0;
	struct ehht_s *table;
	unsigned int num_buckets = 35;

	/* msg = "test_ehht_new"; */
	table = ehht_new(num_buckets);

	if (table == NULL) {
		++failures;
	}

	/* TODO
	   failures += check_unsigned_int_m(ehht_size(table), 0, "test_ehht_new ehht_size");
	 */

	ehht_free(table);
	return failures;
}

int test_ehht_put_get()
{
	int failures = 0;
	struct ehht_s *table;
	const char *key;
	unsigned int num_buckets = 35;
	void *val, *old_val;

	table = ehht_new(num_buckets);

	key = "key1";
	val = ehht_get(table, key, strlen(key));

	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get while empty");

	val = "foo";
	old_val = ehht_put(table, key, strlen(key), val);
	failures +=
	    check_unsigned_long_m((unsigned long)old_val, (unsigned long)NULL,
				  "ehht_put while empty");

	val = ehht_get(table, key, strlen(key));
	if (val == NULL) {
		failures += 1;
	} else {
		failures += check_str_m((const char *)val, "foo", "ehht_get");
	}
	old_val = ehht_put(table, key, strlen(key), "bar");
	failures += check_str_m((const char *)old_val, val, "ehht_put over");

	val = ehht_get(table, "baz", strlen("baz"));
	failures +=
	    check_unsigned_long_m((unsigned long)val, (unsigned long)NULL,
				  "ehht_get bogus");

	val = ehht_get(table, key, strlen(key));
	failures += check_str_m((const char *)val, "bar", "ehht_get replaced");

	ehht_free(table);
	return failures;
}

void foreach_thing(const char *each_key, unsigned int each_key_len,
		   void *each_val, void *arg)
{

	unsigned int *i;
	const char *val;

	i = (unsigned int *)arg;
	val = (const char *)each_val;

	if (each_key_len > 2) {
		*i += (strlen(each_key) + strlen(val));
	}
}

int test_ehht_foreach_element()
{
	int failures = 0;
	struct ehht_s *table;
	unsigned int actual, expected, num_buckets = 5;

	table = ehht_new(num_buckets);

	if (table == NULL) {
		++failures;
	}

	ehht_put(table, "g", 1, "wiz");
	ehht_put(table, "foo", 3, "bar");
	ehht_put(table, "whiz", 4, "bang");
	ehht_put(table, "love", 4, "backend development");

	expected = 3 + 3 + 4 + 4 + 4 + 19;
	actual = 0;

	ehht_foreach_element(table, foreach_thing, &actual);

	failures += check_unsigned_long_m(actual, expected, "foreach");

	ehht_free(table);
	return failures;
}

int main(void)
{				/* int argc, char *argv[]) */
	int failures = 0;

	failures += test_ehht_new();
	failures += test_ehht_put_get();
	failures += test_ehht_foreach_element();

	if (failures) {
		fprintf(stderr, "%d failures in total\n", failures);
	}
	return failures;
}
