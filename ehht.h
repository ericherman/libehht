/* ehht.h */
#ifndef EHHT_H
#define EHHT_H

/* a simple hash-table */

struct ehht_s *ehht_new(unsigned int num_buckets);

void ehht_free(struct ehht_s *table);

void *ehht_get(struct ehht_s *table, const char *key, unsigned int key_len);

void *ehht_put(struct ehht_s *table, const char *key,
	       unsigned int key_len, void *val);

void *ehht_remove(struct ehht_s *table, const char *key, unsigned int key_len);

unsigned int ehht_size(struct ehht_s *table);

void ehht_clear(struct ehht_s *table);

void ehht_foreach_element(struct ehht_s *table,
			  void (*func) (const char *each_key,
					unsigned int each_key_len,
					void *each_val, void *arg), void *arg);

void ehht_to_string(struct ehht_s *table, char *buf, unsigned int buf_len);

#endif /* EHHT_H */
