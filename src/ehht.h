/* ehht.h */
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
