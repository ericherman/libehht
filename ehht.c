#include "ehht.h"
#include <stdlib.h>		/* malloc */
#include <string.h>		/* memcpy */
#include <stdio.h>		/* fprintf */

struct ehht_element_s {
	char *key;
	size_t key_len;
	void *val;
	struct ehht_element_s *next;
};

struct ehht_s {
	size_t num_buckets;
	struct ehht_element_s **buckets;
	unsigned int (*hash_func) (const char *str, size_t str_len);
};

static unsigned int ehht_hash_code_str(const char *str, size_t str_len);

struct ehht_s *ehht_new(size_t num_buckets,
			unsigned int (*hash_func) (const char *str,
						   size_t str_len))
{
	struct ehht_s *table;
	size_t i;

	if (num_buckets == 0) {
		num_buckets = 4096;
	}

	table = malloc(sizeof(struct ehht_s));
	if (table == NULL) {
		return NULL;
	}
	table->num_buckets = num_buckets;
	table->buckets = malloc(sizeof(struct ehht_element_s *) * num_buckets);
	if (table->buckets == NULL) {
		free(table);
		return NULL;
	}
	for (i = 0; i < table->num_buckets; ++i) {
		table->buckets[i] = NULL;
	}

	table->hash_func = (hash_func == NULL) ? ehht_hash_code_str : hash_func;

	return table;
}

static void ehht_free_element(struct ehht_element_s *element)
{
	free(element->key);
	free(element);
}

void ehht_clear(struct ehht_s *table)
{
	size_t i;
	struct ehht_element_s *element;

	for (i = 0; i < table->num_buckets; ++i) {
		while ((element = table->buckets[i]) != NULL) {
			table->buckets[i] = element->next;
			ehht_free_element(element);
		}
	}
}

void ehht_free(struct ehht_s *table)
{
	if (table == NULL) {
		return;
	}

	ehht_clear(table);

	free(table->buckets);
	free(table);
}

static struct ehht_element_s *ehht_new_element(const char *key,
					       size_t key_len, void *val)
{
	char *key_copy;
	struct ehht_element_s *element;

	element = malloc(sizeof(struct ehht_element_s));
	if (element == NULL) {
		return NULL;
	}

	key_copy = malloc(key_len + 1);
	if (!key_copy) {
		ehht_free_element(element);
		return NULL;
	}
	memcpy(key_copy, key, key_len);
	key_copy[key_len] = '\0';

	element->key = key_copy;
	element->key_len = key_len;
	element->val = val;
	element->next = NULL;

	return element;
}

static unsigned int ehht_hash_code_str(const char *str, size_t str_len)
{
	unsigned int hash;
	size_t i;

	hash = 0;
	for (i = 0; i < str_len; ++i) {
		hash *= 31;
		hash += (unsigned int)str[i];
	}

	return hash;
}

static struct ehht_element_s *ehht_get_element(struct ehht_s *table,
					       const char *key, size_t key_len)
{
	struct ehht_element_s *element;
	unsigned int mismatch, hashcode;
	size_t i, bucket_num;

	hashcode = table->hash_func(key, key_len);
	bucket_num = hashcode % table->num_buckets;

	element = table->buckets[bucket_num];
	while (element != NULL) {
		if (element->key_len == key_len) {
			mismatch = 0;
			for (i = 0; mismatch == 0 && i < key_len; ++i) {
				if (key[i] != element->key[i]) {
					++mismatch;
				}
			}
			if (mismatch == 0) {
				return element;
			}
		}
		element = element->next;
	}
	return NULL;
}

void *ehht_get(struct ehht_s *table, const char *key, size_t key_len)
{
	struct ehht_element_s *element;
	element = ehht_get_element(table, key, key_len);
	return (element == NULL) ? NULL : element->val;
}

void *ehht_put(struct ehht_s *table, const char *key, size_t key_len, void *val)
{
	struct ehht_element_s *element;
	void *old_val;
	unsigned int hashcode;
	size_t bucket_num;

	element = ehht_get_element(table, key, key_len);
	old_val = (element == NULL) ? NULL : element->val;

	if (element != NULL) {
		element->val = val;
		return old_val;
	}

	hashcode = table->hash_func(key, key_len);
	bucket_num = hashcode % table->num_buckets;
	element = table->buckets[bucket_num];
	table->buckets[bucket_num] = ehht_new_element(key, key_len, val);
	if (table->buckets[bucket_num] == NULL) {
		/* TODO set errno */
		fprintf(stderr, "could not allocate struct ehht_element_s\n");
		table->buckets[bucket_num] = element;
	} else {
		table->buckets[bucket_num]->next = element;
	}
	return NULL;
}

void *ehht_remove(struct ehht_s *table, const char *key, size_t key_len)
{
	struct ehht_element_s *previous_element, *element;
	void *old_val;
	unsigned int hashcode;
	size_t bucket_num;

	element = ehht_get_element(table, key, key_len);
	if (element == NULL) {
		return NULL;
	}

	old_val = element->val;

	hashcode = table->hash_func(key, key_len);
	bucket_num = hashcode % table->num_buckets;

	previous_element = table->buckets[bucket_num];
	if (previous_element == element) {
		table->buckets[bucket_num] = element->next;
	} else {
		while (previous_element->next != element) {
			previous_element = previous_element->next;
			/* assert(previous_element != NULL); */
		}
		previous_element->next = element->next;
	}
	ehht_free_element(element);
	return old_val;
}

void ehht_foreach_element(struct ehht_s *table,
			  void (*func) (const char *each_key,
					size_t each_key_len,
					void *each_val, void *context),
			  void *context)
{
	size_t i;
	struct ehht_element_s *element;

	for (i = 0; i < table->num_buckets; ++i) {
		for (element = table->buckets[i];
		     element != NULL; element = element->next) {
			(*func) (element->key, element->key_len, element->val,
				 context);
		}
	}
}

static void foreach_count(const char *each_key, size_t each_key_len,
			  void *each_val, void *context)
{
	if (0) {		/* [-Werror=unused-parameter] */
		fprintf(stderr, "key: %s (len: %u), val: %p", each_key,
			(unsigned int)each_key_len, each_val);
	}

	*((size_t *)context) += 1;
}

size_t ehht_size(struct ehht_s *table)
{
	size_t i = 0;

	ehht_foreach_element(table, foreach_count, &i);

	return i;
}

struct ehht_str_buf_s {
	char *buf;
	size_t buf_len;
	size_t buf_pos;
};

static void to_string_each(const char *each_key, size_t each_key_len,
			   void *each_val, void *context)
{

	struct ehht_str_buf_s *str_buf;
	char *buf;
	int bytes_written;

	str_buf = (struct ehht_str_buf_s *)context;
	buf = str_buf->buf + str_buf->buf_pos;

	bytes_written =
	    sprintf(buf, "'%s' => %p, ", each_key_len ? each_key : "",
		    each_val);

	if (bytes_written > 0) {
		str_buf->buf_pos += ((unsigned int)bytes_written);
	}
}

size_t ehht_to_string(struct ehht_s *table, char *buf, size_t buf_len)
{
	struct ehht_str_buf_s str_buf;
	int bytes_written;

	str_buf.buf = buf;
	str_buf.buf_len = buf_len;
	str_buf.buf_pos = 0;

	bytes_written = sprintf(buf, "{ ");
	if (bytes_written > 0) {
		str_buf.buf_pos += ((unsigned int)bytes_written);
	}
	ehht_foreach_element(table, to_string_each, &str_buf);
	bytes_written = sprintf(str_buf.buf + str_buf.buf_pos, "}");
	if (bytes_written > 0) {
		str_buf.buf_pos += ((unsigned int)bytes_written);
	}
	return str_buf.buf_pos;
}

size_t ehht_distribution_report(struct ehht_s *table, size_t *sizes,
				size_t sizes_len)
{
	size_t i;
	struct ehht_element_s *element;

	for (i = 0; i < table->num_buckets && i < sizes_len; ++i) {
		sizes[i] = 0;
		element = table->buckets[i];
		while (element != NULL) {
			sizes[i] += 1;
			element = element->next;
		}
	}

	return i;
}
