#include "test-ehht.h"

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
				if (strcmp(e_keys[i], ks->keys[j].str) == 0) {
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

TEST_EHHT_MAIN(test_ehht_keys())
