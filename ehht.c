#include "ehht.h"
#include <stdlib.h>		/* malloc */
#include <stdio.h>		/* fprintf */

struct ehht_element_s {
	const char *key;
	unsigned int key_len;
	void *val;
	struct ehht_element_s *next;
};

struct ehht_s {
	unsigned int num_buckets;
	struct ehht_element_s **buckets;
};

struct ehht_s *ehht_new(unsigned int num_buckets)
{
	struct ehht_s *table;
	unsigned int i;

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

	return table;
}

void ehht_free(struct ehht_s *table)
{
	unsigned int i;
	struct ehht_element_s *element;

	if (table == NULL) {
		return;
	}

	for (i = 0; i < table->num_buckets; ++i) {
		while ((element = table->buckets[i]) != NULL) {
			table->buckets[i] = element->next;
			free(element);
		}
	}
	free(table->buckets);
	free(table);
}

static struct ehht_element_s *ehht_new_element(const char *key,
					       unsigned int key_len, void *val)
{
	struct ehht_element_s *element = malloc(sizeof(struct ehht_element_s));
	if (element == NULL) {
		return NULL;
	}

	element->key = key;
	element->key_len = key_len;
	element->val = val;
	element->next = NULL;

	return element;
}

static unsigned int ehht_hash_code_str(const char *str, unsigned int str_len)
{
	unsigned int i, hash;

	hash = 0;
	for (i = 0; i < str_len; ++i) {
		hash *= 31;
		hash += (unsigned int)str[i];
	}

	return hash;
}

static struct ehht_element_s *ehht_get_element(struct ehht_s *table,
					       const char *key,
					       unsigned int key_len)
{
	struct ehht_element_s *element;
	unsigned int i, mismatch, hashcode, bucket_num;

	hashcode = ehht_hash_code_str(key, key_len);
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
			if (mismatch == 0)
				return element;
		}
		element = element->next;
	}
	return NULL;
}

void *ehht_get(struct ehht_s *table, const char *key, unsigned int key_len)
{
	struct ehht_element_s *element;
	element = ehht_get_element(table, key, key_len);
	return (element == NULL) ? NULL : element->val;
}

void *ehht_put(struct ehht_s *table, const char *key, unsigned int key_len,
	       void *val)
{
	struct ehht_element_s *element;
	void *old_val;
	unsigned int hashcode, bucket_num;

	element = ehht_get_element(table, key, key_len);
	old_val = (element == NULL) ? NULL : element->val;

	if (element != NULL) {
		element->val = val;
	} else {
		hashcode = ehht_hash_code_str(key, key_len);
		bucket_num = hashcode % table->num_buckets;
		element = table->buckets[bucket_num];
		if (element == NULL) {
			element = ehht_new_element(key, key_len, val);
			if (element == NULL) {
				/* TODO set errno */
				fprintf(stderr,
					"could not allocate '%s'\n",
					"struct ehht_element_s");
				return NULL;
			}
			table->buckets[bucket_num] = element;
		} else {
			while (element->next != NULL) {
				element = element->next;
			}
			element->next = ehht_new_element(key, key_len, val);
		}
	}
	return old_val;
}

void *ehht_remove(struct ehht_s *table, const char *key, unsigned int key_len)
{
	struct ehht_element_s *previous_element, *element;
	void *old_val;
	unsigned int hashcode, bucket_num;

	element = ehht_get_element(table, key, key_len);
	if (element == NULL) {
		return NULL;
	}

	old_val = element->val;

	hashcode = ehht_hash_code_str(key, key_len);
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
	free(element);
	return old_val;
}

void ehht_foreach_element(struct ehht_s *table,
			  void (*func) (const char *each_key,
					unsigned int each_key_len,
					void *each_val, void *arg), void *arg)
{

	unsigned int i;
	struct ehht_element_s *element;

	for (i = 0; i < table->num_buckets; ++i) {
		for (element = table->buckets[i];
		     element != NULL; element = element->next) {
			(*func) (element->key, element->key_len, element->val,
				 arg);
		}
	}
}

static void foreach_count(const char *each_key, unsigned int each_key_len,
			  void *each_val, void *arg)
{
	if (0) {		/* [-Werror=unused-parameter] */
		fprintf(stderr, "key: %s (len: %u), val: %p", each_key,
			each_key_len, each_val);
	}

	*((unsigned int *)arg) += 1;
}

unsigned int ehht_size(struct ehht_s *table)
{
	unsigned int i = 0;

	ehht_foreach_element(table, foreach_count, &i);

	return i;
}

struct str_buf_s {
	char *buf;
	unsigned int buf_len;
	unsigned int buf_pos;
};

static void to_string_each(const char *each_key, unsigned int each_key_len,
			   void *each_val, void *arg)
{

	struct str_buf_s *str_buf;
	char *buf;
	int bytes_written;

	str_buf = (struct str_buf_s *)arg;
	buf = str_buf->buf + str_buf->buf_pos;

	bytes_written =
	    sprintf(buf, "'%s' => %p, ", each_key_len ? each_key : "",
		    each_val);

	if (bytes_written > 0) {
		str_buf->buf_pos += ((unsigned int)bytes_written);
	}
}

void ehht_to_string(struct ehht_s *table, char *buf, unsigned int buf_len)
{
	struct str_buf_s str_buf;
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
}
