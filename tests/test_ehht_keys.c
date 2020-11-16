/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehht_keys.c: test for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "echeck.h"

unsigned test_ehht_keys(void)
{
	const size_t bytes_len = 250 * sizeof(size_t);
	unsigned char bytes[250 * sizeof(size_t)];
	struct eembed_allocator *orig = eembed_global_allocator;
	struct eembed_allocator *ea = NULL;

	unsigned failures = 0;
	struct ehht *table = NULL;
	struct ehht_keys *ks = NULL;
	int allocate_copies = 0;
	int err = 0;
	size_t i = 0;
	size_t j = 0;
	size_t num_buckets = 0;
	size_t len = 0;
	size_t *found = NULL;
	const char *e_keys[] = { "foo", "bar", "whiz", "bang", NULL };

	if (!EEMBED_HOSTED) {
		ea = eembed_bytes_allocator(bytes, bytes_len);
		if (check_ptr_not_null(ea)) {
			return 1;
		}
		eembed_global_allocator = ea;
	}

	num_buckets = 3;

	ea = eembed_global_allocator;
	for (allocate_copies = 0; allocate_copies < 2; ++allocate_copies) {

		table = ehht_new_custom(num_buckets, NULL, NULL, NULL);

		for (i = 0; e_keys[i] != NULL; ++i) {
			err = 0;
			len = eembed_strlen(e_keys[i]);
			table->put(table, e_keys[i], len, NULL, &err);
			failures += check_int(err, 0);
		}

		found = ea->calloc(ea, sizeof(size_t), table->size(table));
		check_ptr_not_null_m(found, "could not allocate found array");
		if (!found) {
			++failures;
			goto test_ehht_keys_end;
		}

		err = 0;
		ks = table->keys(table, allocate_copies);
		failures += check_size_t(ks->len, table->size(table));
		for (i = 0; e_keys[i] != NULL; ++i) {
			for (j = 0; j < ks->len && !found[i]; ++j) {
				if (eembed_strcmp(e_keys[i], ks->keys[j].str) ==
				    0) {
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
		ea->free(ea, found);

		ehht_free(table);
	}

test_ehht_keys_end:
	if (!EEMBED_HOSTED) {
		eembed_global_allocator = orig;
	}
	return failures;
}

ECHECK_TEST_MAIN(test_ehht_keys)
