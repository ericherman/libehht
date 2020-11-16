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

Ehht_begin_C_functions
#undef Ehht_begin_C_functions
#include <stddef.h>		/* size_t */
    struct eembed_log;		/* emmbed.h */
struct eembed_allocator;	/* emmbed.h */

struct ehht_key {
	const char *str;
	size_t len;
	unsigned hashcode;
};

struct ehht_keys {
	struct ehht_key *keys;
	size_t len;
	int keys_copied;
};

/* passed parameter functions */
typedef int (*ehht_iterator_func)(struct ehht_key each_key,
				  void *each_val, void *context);

/* interface */
struct ehht {
	/* private */
	void *data;

	/* public methods */
	void *(*get)(struct ehht *table, const char *key, size_t key_len);

	/* allocates a copy of the key parameter,
	 * copy will be freed when the key is no longer in use by the table
	 * returns the previous value or NULL
	 * if memory allocation fails, and the *err in not NULL, *err will
	 * be populated with a non-zero value */
	void *(*put)(struct ehht *table, const char *key, size_t key_len,
		     void *val, int *err);

	/* returns the previous value or NULL */
	void *(*remove)(struct ehht *table, const char *key, size_t key_len);

	size_t (*size)(struct ehht *table);

	void (*clear)(struct ehht *table);

	int (*for_each)(struct ehht *table, ehht_iterator_func func,
			void *context);

	int (*has_key)(struct ehht *table, const char *key, size_t key_len);

	/* fills the keys array with (pointers or newly allocated) key strings
	 * populates the lens array with the corresponding lengths
	 * returns the number of elements populated
	 */
	struct ehht_keys *(*keys) (struct ehht *table, int copy_keys);

	void (*free_keys)(struct ehht *table, struct ehht_keys *keys);

	/* returns the number of characters written to "buf"
	   (excluding the null byte terminator) */
	size_t (*to_string)(struct ehht *table, char *buf, size_t buf_len);
};

/*****************************************************************************/
/* constructors and destructor */
/*****************************************************************************/
/* default constructor */
struct ehht *ehht_new(void);

/* allocator aware constructor */
typedef unsigned int (*ehht_hash_func)(const char *data, size_t data_len);

/* if hash_func is NULL, a hashing function will be provided */
/* if ea is NULL, eembed_global_alloctor will be used */
/* if log is NULL, eembed_err_log will be used */
struct ehht *ehht_new_custom(size_t num_buckets,
			     ehht_hash_func hash_func,
			     struct eembed_allocator *ea,
			     struct eembed_log *log);

/* destructor */
void ehht_free(struct ehht *table);
/*****************************************************************************/

/*****************************************************************************/
/* implementation-exposing "friend" functions are provided for testing and
 * other very special uses, but are not truly part of a hashtable API */
/*****************************************************************************/
size_t ehht_buckets_size(struct ehht *table);
size_t ehht_buckets_resize(struct ehht *table, size_t num_buckets);
void ehht_buckets_auto_resize_load_factor(struct ehht *table, double factor);
size_t ehht_bucket_for_key(struct ehht *table, const char *key, size_t key_len);
/* The keys are not copied into the hashtable, rather referenced.
   If the key is modified, the behavior is undefined.
   If the key is freed while still in use by the hash, expect a crash
   To change this value, the table must be empty.
   Returns non-zero on error. */
int ehht_trust_keys_immutable(struct ehht *ht, int val);
/*****************************************************************************/

Ehht_end_C_functions
#undef Ehht_end_C_functions
#endif /* EHHT_H */
