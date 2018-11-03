/* ehht-interface.h:  interface for a simple OO hashtable
   Copyright (C) 2016, 2017, 2018 Eric Herman <eric@freesa.org>

   This work is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or (at
   your option) any later version.

   This work is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

	https://www.gnu.org/licenses/lgpl-3.0.txt
	https://www.gnu.org/licenses/gpl-3.0.txt
*/
#ifndef EHHT_INTERFACE_H
#define EHHT_INTERFACE_H

/* a simple hash-table interface */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>		/* size_t */

struct ehht_key_s {
	const char *str;
	size_t len;
	unsigned hashcode;
};

struct ehht_keys_s {
	struct ehht_key_s *keys;
	size_t len;
	int keys_copied;
};

/* passed parameter functions */
typedef int (*ehht_iterator_func) (struct ehht_key_s each_key,
				   void *each_val, void *context);

/* interface */
struct ehht_s {
	/* private */
	void *data;

	/* public methods */
	void *(*get) (struct ehht_s *table, const char *key, size_t key_len);

	/* allocates a copy of the key parameter,
	 * copy will be freed when the key is no longer in use by the table
	 * returns the previous value or NULL */
	void *(*put) (struct ehht_s *table, const char *key, size_t key_len,
		      void *val);

	/* returns the previous value or NULL */
	void *(*remove) (struct ehht_s *table, const char *key, size_t key_len);

	size_t (*size) (struct ehht_s *table);

	void (*clear) (struct ehht_s *table);

	int (*for_each) (struct ehht_s *table, ehht_iterator_func func,
			 void *context);

	int (*has_key) (struct ehht_s *table, const char *key, size_t key_len);

	/* fills the keys array with (pointers or newly allocated) key strings
	   populates the lens array with the corresponding lengths
	   returns the number of elements populated */
	struct ehht_keys_s *(*keys) (struct ehht_s *table, int copy_keys);
	void (*free_keys) (struct ehht_s *table, struct ehht_keys_s *keys);

	/* returns the number of characters written to "buf"
	   (excluding the null byte terminator) */
	size_t (*to_string) (struct ehht_s *table, char *buf, size_t buf_len);

	size_t (*num_buckets) (struct ehht_s *table);
	size_t (*resize) (struct ehht_s *table, size_t num_buckets);
};

#ifdef __cplusplus
}
#endif

#endif /* EHHT_INTERFACE_H */
