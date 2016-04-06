/* ehht.h */
#ifndef EHHT_H
#define EHHT_H

/* a simple hash-table */

#include <stddef.h>		/* size_t */

/* passed parameter functions */
typedef unsigned int (*ehht_hash_func) (const char *str, size_t str_len);
typedef void *(*ehht_malloc_func) (size_t size, void *context);
typedef void (*ehht_free_func) (void *ptr, size_t size, void *context);
typedef int (*ehht_iterator_func) (const char *each_key, size_t each_key_len,
				   void *each_val, void *context);

struct ehht_s;

/* methods */
typedef void *(*ehht_get_func) (struct ehht_s * table, const char *key,
				size_t key_len);

/* returns the previous value or NULL */
typedef void *(*ehht_put_func) (struct ehht_s * table, const char *key,
				size_t key_len, void *val);

/* returns the previous value or NULL */
typedef void *(*ehht_remove_func) (struct ehht_s * table, const char *key,
				   size_t key_len);

typedef size_t (*ehht_size_func) (struct ehht_s * table);

typedef void (*ehht_clear_func) (struct ehht_s * table);

typedef int (*ehht_for_each_element_func) (struct ehht_s * table,
					   ehht_iterator_func func,
					   void *context);

/* returns the number of characters written to "buf"
   (excluding the null byte terminator) */
typedef size_t (*ehht_to_string_func) (struct ehht_s * table, char *buf,
				       size_t buf_len);

typedef size_t (*ehht_num_buckets_func) (struct ehht_s * table);

/* reports each of the bucket sizes
   sizes_len should be equal to or greater than num_buckets of ehht_new
   returns the number of size_t values written */
typedef size_t (*ehht_distribution_report_func) (struct ehht_s * table,
						 size_t *sizes,
						 size_t sizes_len);

/* interface */
struct ehht_s {
	/* private */
	void *data;

	/* public methods */
	ehht_get_func get;
	ehht_put_func put;
	ehht_remove_func remove;
	ehht_size_func size;
	ehht_clear_func clear;
	ehht_for_each_element_func for_each;
	ehht_to_string_func to_string;
	ehht_num_buckets_func num_buckets;
	ehht_distribution_report_func report;
};

/* constructor */
/* if hash_func is NULL, a hashing function will be provided */
/* if ehht_malloc_func/free_func are NULL, malloc/free will be used */
struct ehht_s *ehht_new(size_t num_buckets, ehht_hash_func hash_func,
			ehht_malloc_func alloc_func, ehht_free_func free_func,
			void *mem_context);

/* destructor */
void ehht_free(struct ehht_s *table);

#endif /* EHHT_H */
