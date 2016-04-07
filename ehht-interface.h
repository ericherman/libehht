/* ehht-interface.h */
#ifndef EHHT_INTERFACE_H
#define EHHT_INTERFACE_H

/* a simple hash-table interface */

#include <stddef.h>		/* size_t */

/* passed parameter functions */
typedef unsigned int (*ehht_hash_func) (const char *str, size_t str_len);
typedef void *(*ehht_malloc_func) (size_t size, void *context);
typedef void (*ehht_free_func) (void *ptr, size_t size, void *context);
typedef int (*ehht_iterator_func) (const char *each_key, size_t each_key_len,
				   void *each_val, void *context);

/* interface */
struct ehht_s {
	/* private */
	void *data;

	/* public methods */
	void *(*get) (struct ehht_s *table, const char *key, size_t key_len);

	/* returns the previous value or NULL */
	void *(*put) (struct ehht_s *table, const char *key, size_t key_len,
		      void *val);

	/* returns the previous value or NULL */
	void *(*remove) (struct ehht_s *table, const char *key, size_t key_len);

	size_t (*size) (struct ehht_s *table);

	void (*clear) (struct ehht_s *table);

	int (*for_each) (struct ehht_s *table, ehht_iterator_func func,
			 void *context);

	/* returns the number of characters written to "buf"
	   (excluding the null byte terminator) */
	size_t (*to_string) (struct ehht_s *table, char *buf, size_t buf_len);

	size_t (*num_buckets) (struct ehht_s *table);

	/* reports each of the bucket sizes
	   sizes_len should be equal to or greater than num_buckets of ehht_new
	   returns the number of size_t values written */
	size_t (*report) (struct ehht_s *table, size_t *sizes,
			  size_t sizes_len);
};

#endif /* EHHT_INTERFACE_H */
