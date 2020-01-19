/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehht.h: a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#ifndef EHHT_H
#define EHHT_H

/* a simple hash-table */

#ifdef __cplusplus
#define Ehht_begin_C_functions extern "C" {
#define Ehht_end_C_functions }
#else
#define Ehht_begin_C_functions
#define Ehht_end_C_functions
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
typedef int (*ehht_iterator_func)(struct ehht_key_s each_key,
				  void *each_val, void *context);

/* interface */
struct ehht_s {
	/* private */
	void *data;

	/* public methods */
	void *(*get)(struct ehht_s *table, const char *key, size_t key_len);

	/* allocates a copy of the key parameter,
	 * copy will be freed when the key is no longer in use by the table
	 * returns the previous value or NULL */
	void *(*put)(struct ehht_s *table, const char *key, size_t key_len,
		     void *val);

	/* returns the previous value or NULL */
	void *(*remove)(struct ehht_s *table, const char *key, size_t key_len);

	size_t (*size)(struct ehht_s *table);

	void (*clear)(struct ehht_s *table);

	int (*for_each)(struct ehht_s *table, ehht_iterator_func func,
			void *context);

	int (*has_key)(struct ehht_s *table, const char *key, size_t key_len);

	/* fills the keys array with (pointers or newly allocated) key strings
	   populates the lens array with the corresponding lengths
	   returns the number of elements populated */
	struct ehht_keys_s *(*keys) (struct ehht_s *table, int copy_keys);

	void (*free_keys)(struct ehht_s *table, struct ehht_keys_s *keys);

	/* returns the number of characters written to "buf"
	   (excluding the null byte terminator) */
	size_t (*to_string)(struct ehht_s *table, char *buf, size_t buf_len);
};

Ehht_begin_C_functions
#undef Ehht_begin_C_functions
/*****************************************************************************/
/* constructors and destructor */
/*****************************************************************************/
/* default constructor */
struct ehht_s *ehht_new(void);

/* allocator aware constructor */
typedef unsigned int (*ehht_hash_func)(const char *data, size_t data_len);
typedef void *(*ehht_malloc_func)(size_t size, void *context);
typedef void (*ehht_free_func)(void *ptr, void *context);

/* if hash_func is NULL, a hashing function will be provided */
/* if ehht_malloc_func/free_func are NULL, malloc/free will be used */
struct ehht_s *ehht_new_custom(size_t num_buckets,
			       ehht_hash_func hash_func,
			       ehht_malloc_func alloc_func,
			       ehht_free_func free_func, void *mem_context);

/* destructor */
void ehht_free(struct ehht_s *table);
/*****************************************************************************/

/*****************************************************************************/
/* implementation-exposing "friend" functions are provided for testing and
 * other very special uses, but are not truly part of a hashtable API */
/*****************************************************************************/
size_t ehht_buckets_size(struct ehht_s *table);
size_t ehht_buckets_resize(struct ehht_s *table, size_t num_buckets);
void ehht_buckets_auto_resize_load_factor(struct ehht_s *table, double factor);
size_t ehht_bucket_for_key(struct ehht_s *table, const char *key,
			   size_t key_len);
/*****************************************************************************/

Ehht_end_C_functions
#undef Ehht_end_C_functions
#endif /* EHHT_H */
