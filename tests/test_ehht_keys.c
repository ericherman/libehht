/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_keys.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "test-ehht.h"

int test_ehht_keys()
{
	int failures = 0;
	struct ehht_s *table;
	struct ehht_keys_s *ks;
	int allocate_copies, err;
	size_t i, j, num_buckets, len;
	size_t *found;
	const char *e_keys[] = { "foo", "bar", "whiz", "bang", NULL };

	num_buckets = 3;

	for (allocate_copies = 0; allocate_copies < 2; ++allocate_copies) {

		table = ehht_new_custom(num_buckets, NULL, NULL, NULL, NULL);

		for (i = 0; e_keys[i] != NULL; ++i) {
			err = 0;
			len = strlen(e_keys[i]);
			table->put(table, e_keys[i], len, NULL, &err);
			failures += check_int(err, 0);
		}

		found = calloc(sizeof(size_t), table->size(table));
		if (!found) {
			fprintf(stderr, "could not allocate found array\n");
			return 1;
		}

		err = 0;
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
