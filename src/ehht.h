/* ehht.h: a simple OO hashtable
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
#ifndef EHHT_H
#define EHHT_H

/* a simple hash-table */

#ifdef __cplusplus
extern "C" {
#endif

#include "ehht-interface.h"
#include <stddef.h>		/* size_t */

typedef unsigned int (*ehht_hash_func) (const char *str, size_t str_len);
typedef void *(*ehht_malloc_func) (size_t size, void *context);
typedef void (*ehht_free_func) (void *ptr, size_t size, void *context);

/* constructor */
/* if hash_func is NULL, a hashing function will be provided */
/* if ehht_malloc_func/free_func are NULL, malloc/free will be used */
struct ehht_s *ehht_new(size_t num_buckets, ehht_hash_func hash_func,
			ehht_malloc_func alloc_func, ehht_free_func free_func,
			void *mem_context);

/* destructor */
void ehht_free(struct ehht_s *table);

#ifdef __cplusplus
}
#endif

#endif /* EHHT_H */
