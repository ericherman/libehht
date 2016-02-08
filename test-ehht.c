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

	failures += check_unsigned_int_m(ehht_size(table), 0, "ehht_size");

	ehht_free(table);
	return failures;
}

int test_ehht_put_get_remove()
{
	int failures = 0;
	struct ehht_s *table;
	const char *key;
	unsigned int num_buckets = 3;
	void *val, *old_val;
	char buf[1000];

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

	key = "two";
	ehht_put(table, key, strlen(key), "2");
	key = "three";
	ehht_put(table, key, strlen(key), "3");
	key = "four";
	ehht_put(table, key, strlen(key), "4");
	key = "ping";
	ehht_put(table, key, strlen(key), "pong");
	key = "whiz";
	ehht_put(table, key, strlen(key), "bang");
	key = "seven";
	ehht_put(table, key, strlen(key), "7");
	key = "eight";
	ehht_put(table, key, strlen(key), "8");
	key = "nine";
	ehht_put(table, key, strlen(key), "9");
	failures += check_unsigned_int_m(ehht_size(table), 9, "ehht_size");

	key = "ping";
	old_val = ehht_remove(table, key, strlen(key));
	failures += check_str_m((const char *)old_val, "pong", "ehht_remove");
	val = ehht_get(table, "ping", strlen(key));
	if (val != NULL) {
		++failures;
	}
	failures += check_unsigned_int_m(ehht_size(table), 8, "remove size");
	old_val = ehht_remove(table, "bogus", strlen("bogus"));
	failures += check_unsigned_int_m(ehht_size(table), 8, "remove size");
	if (old_val != NULL) {
		++failures;
	}

	ehht_remove(table, "key1", strlen("key1"));
	ehht_remove(table, "two", strlen("two"));
	ehht_remove(table, "nine", strlen("nine"));
	ehht_remove(table, "three", strlen("three"));
	ehht_remove(table, "eight", strlen("eight"));
	ehht_remove(table, "four", strlen("four"));
	ehht_remove(table, "seven", strlen("seven"));

	ehht_to_string(table, buf, 1000);

	failures += check_unsigned_int_m(ehht_size(table), 1, buf);
	old_val = ehht_remove(table, "whiz", strlen("whiz"));
	failures += check_str_m((const char *)old_val, "bang", "last remove");
	failures += check_unsigned_int_m(ehht_size(table), 0, "last size");

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

	failures += check_unsigned_int_m(ehht_size(table), 4, "ehht_size");

	ehht_free(table);
	return failures;
}

int main(void)
{				/* int argc, char *argv[]) */
	int failures = 0;

	failures += test_ehht_new();
	failures += test_ehht_put_get_remove();
	failures += test_ehht_foreach_element();

	if (failures) {
		fprintf(stderr, "%d failures in total\n", failures);
	}
	return failures;
}
