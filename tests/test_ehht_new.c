#include "test-ehht.h"

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

TEST_EHHT_MAIN(test_ehht_new())
