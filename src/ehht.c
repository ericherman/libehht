/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehht.c: a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include <stdlib.h>		/* malloc calloc */
#include <string.h>		/* memcpy memcmp */
#include <stdio.h>		/* fprintf */
#include <assert.h>
#include <errno.h>

#ifndef EHHT_DEBUG
#ifdef NDEBUG
#define EHHT_DEBUG 0
#else
#define EHHT_DEBUG 1
#endif
#endif

struct ehht_element_s {
	struct ehht_key_s key;
	void *val;
	struct ehht_element_s *next;
};

struct ehht_table_s {
	size_t num_buckets;
	struct ehht_element_s **buckets;
	size_t size;
	ehht_hash_func hash_func;
	ehht_malloc_func alloc;
	ehht_free_func free;
	void *mem_context;
};

static void ehht_set_table(struct ehht_s *this, struct ehht_table_s *table)
{
	this->data = (void *)table;
}

static struct ehht_table_s *ehht_get_table(struct ehht_s *this)
{
	return (struct ehht_table_s *)this->data;
}

static void ehht_free_element(struct ehht_table_s *table,
			      struct ehht_element_s *element)
{
	struct ehht_key_s key;

	key = element->key;
	table->free((char *)key.str, table->mem_context);
	table->free(element, table->mem_context);
}

static void ehht_clear(struct ehht_s *this)
{
	struct ehht_table_s *table;
	size_t i;
	struct ehht_element_s *element;

	table = ehht_get_table(this);

	for (i = 0; i < table->num_buckets; ++i) {
		while ((element = table->buckets[i]) != NULL) {
			table->buckets[i] = element->next;
			ehht_free_element(table, element);
		}
	}
	table->size = 0;
}

static size_t ehht_bucket_for_hashcode(unsigned int hashcode,
				       size_t num_buckets)
{
	return (size_t)(hashcode % num_buckets);
}

static size_t ehht_bucket_for_key(struct ehht_s *this, const char *key,
				  size_t key_len)
{
	struct ehht_table_s *table;
	unsigned int hashcode;

	table = ehht_get_table(this);
	hashcode = table->hash_func(key, key_len);

	return ehht_bucket_for_hashcode(hashcode, table->num_buckets);
}

static struct ehht_element_s *ehht_alloc_element(struct ehht_table_s *table,
						 const char *key,
						 size_t key_len,
						 unsigned int hashcode,
						 void *val)
{
	char *str_copy;
	struct ehht_element_s *element;

	element =
	    table->alloc(sizeof(struct ehht_element_s), table->mem_context);
	if (element == NULL) {
		assert(errno == ENOMEM);
		return NULL;
	}

	str_copy = table->alloc(key_len + 1, table->mem_context);
	if (!str_copy) {
		ehht_free_element(table, element);
		assert(errno == ENOMEM);
		return NULL;
	}
	memcpy(str_copy, key, key_len);
	str_copy[key_len] = '\0';

	element->key.str = str_copy;
	element->key.len = key_len;
	element->key.hashcode = hashcode;
	element->val = val;
	element->next = NULL;

	++(table->size);

	return element;
}

/* "This is not the best possible hash function,
   but it is short and effective."
   The C Programming Language, 2nd Edition */
unsigned int ehht_kr2_hashcode(const char *data, size_t len)
{
	unsigned int hash;
	size_t i;

	hash = 0;
	for (i = 0; i < len; ++i) {
		hash *= 31;
		hash += (unsigned int)data[i];
	}

	return hash;
}

static struct ehht_element_s *ehht_get_element(struct ehht_table_s *table,
					       const char *key, size_t key_len)
{
	struct ehht_element_s *element;
	unsigned int hashcode;
	size_t bucket_num;

	hashcode = table->hash_func(key, key_len);
	bucket_num = ehht_bucket_for_hashcode(hashcode, table->num_buckets);

	element = table->buckets[bucket_num];
	while (element != NULL) {
		if (element->key.len == key_len) {
			if (memcmp(key, element->key.str, key_len) == 0) {
				return element;
			}
		}
		element = element->next;
	}
	return NULL;
}

static void *ehht_get(struct ehht_s *this, const char *key, size_t key_len)
{
	struct ehht_table_s *table;
	struct ehht_element_s *element;

	table = ehht_get_table(this);

	element = ehht_get_element(table, key, key_len);
	return (element == NULL) ? NULL : element->val;
}

static void *ehht_put(struct ehht_s *this, const char *key, size_t key_len,
		      void *val)
{
	struct ehht_table_s *table;
	struct ehht_element_s *element;
	void *old_val;
	unsigned int hashcode;
	size_t bucket_num;

	table = ehht_get_table(this);

	element = ehht_get_element(table, key, key_len);
	old_val = (element == NULL) ? NULL : element->val;

	if (element != NULL) {
		element->val = val;
		return old_val;
	}

	hashcode = table->hash_func(key, key_len);
	bucket_num = ehht_bucket_for_hashcode(hashcode, table->num_buckets);
	element = ehht_alloc_element(table, key, key_len, hashcode, val);
	if (!element) {
		if (EHHT_DEBUG) {
			fprintf(stderr,
				"could not allocate struct ehht_element_s\n");
		}
		assert(errno == ENOMEM);
		/* should this exit? */
		return NULL;
	}
	element->next = table->buckets[bucket_num];
	table->buckets[bucket_num] = element;
	return NULL;
}

static void *ehht_remove(struct ehht_s *this, const char *key, size_t key_len)
{
	struct ehht_table_s *table;
	struct ehht_element_s *element;
	struct ehht_element_s **ptr_to_element;
	void *old_val;
	unsigned int hashcode;
	size_t bucket_num;

	table = ehht_get_table(this);

	element = ehht_get_element(table, key, key_len);
	if (element == NULL) {
		return NULL;
	}

	old_val = element->val;

	hashcode = table->hash_func(key, key_len);
	bucket_num = ehht_bucket_for_hashcode(hashcode, table->num_buckets);

	/* find what points to this element */
	ptr_to_element = &(table->buckets[bucket_num]);
	while (*ptr_to_element != element) {
		ptr_to_element = &((*ptr_to_element)->next);
	}
	/* make that point to the next element */
	*ptr_to_element = element->next;

	--(table->size);
	ehht_free_element(table, element);

	return old_val;
}

static int ehht_for_each(struct ehht_s *this,
			 int (*func)(struct ehht_key_s each_key,
				     void *each_val, void *context),
			 void *context)
{
	struct ehht_table_s *table;
	size_t i, end;
	struct ehht_element_s *element;

	table = ehht_get_table(this);

	end = 0;
	for (i = 0; i < table->num_buckets && !end; ++i) {
		for (element = table->buckets[i]; element != NULL;
		     element = element->next) {
			end = (*func) (element->key, element->val, context);
		}
	}

	return end;
}

static size_t ehht_size(struct ehht_s *this)
{
	struct ehht_table_s *table;

	table = ehht_get_table(this);
	return table->size;
}

struct ehht_str_buf_s {
	char *buf;
	size_t buf_len;
	size_t buf_pos;
};

static int ehht_to_string_each(struct ehht_key_s key, void *each_val,
			       void *context)
{

	struct ehht_str_buf_s *str_buf;
	char *buf;
	const char *fmt;
	size_t bytes_to_write;
	int bytes_written;

	str_buf = (struct ehht_str_buf_s *)context;
	buf = str_buf->buf + str_buf->buf_pos;

	fmt = "'%s' => %p, ";
	bytes_to_write = key.len + (__WORDSIZE / 4) + strlen(fmt);
	if (str_buf->buf_len < (str_buf->buf_pos + bytes_to_write)) {
		return -1;
	}

	bytes_written = sprintf(buf, fmt, key.len ? key.str : "", each_val);
	if (bytes_written > 0) {
		str_buf->buf_pos += ((unsigned int)bytes_written);
	}

	return 0;
}

static size_t ehht_to_string(struct ehht_s *this, char *buf, size_t buf_len)
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
	ehht_for_each(this, ehht_to_string_each, &str_buf);
	bytes_written = sprintf(str_buf.buf + str_buf.buf_pos, "}");
	if (bytes_written > 0) {
		str_buf.buf_pos += ((unsigned int)bytes_written);
	}
	return str_buf.buf_pos;
}

static size_t ehht_resize(struct ehht_s *this, size_t num_buckets)
{
	size_t i, old_num_buckets, new_bucket_num, size;
	struct ehht_table_s *table;
	struct ehht_element_s **new_buckets, **old_buckets, *element;

	table = ehht_get_table(this);

	if (num_buckets == 0) {
		num_buckets = table->num_buckets * 2;
	}
	assert(num_buckets > 1);
	size = sizeof(struct ehht_element_s *) * num_buckets;
	new_buckets = table->alloc(size, table->mem_context);
	if (new_buckets == NULL) {
		return table->num_buckets;
	}
	for (i = 0; i < num_buckets; ++i) {
		new_buckets[i] = NULL;
	}

	old_num_buckets = table->num_buckets;
	old_buckets = table->buckets;
	for (i = 0; i < old_num_buckets; ++i) {
		while ((element = old_buckets[i]) != NULL) {
			old_buckets[i] = element->next;
			new_bucket_num =
			    ehht_bucket_for_hashcode(element->key.hashcode,
						     num_buckets);
			element->next = new_buckets[new_bucket_num];
			new_buckets[new_bucket_num] = element;
		}
	}
	table->buckets = new_buckets;
	table->num_buckets = num_buckets;

	size = sizeof(struct ehht_element_s *) * old_num_buckets;
	table->free(old_buckets, table->mem_context);
	return num_buckets;
}

static size_t ehht_num_buckets(struct ehht_s *this)
{
	struct ehht_table_s *table;

	table = ehht_get_table(this);
	return table->num_buckets;
}

struct ehht_kl_s {
	struct ehht_s *ehht;
	struct ehht_keys_s *keys;
	size_t pos;
};

static int ehht_fill_keys_each(struct ehht_key_s key, void *each_val,
			       void *context)
{
	struct ehht_kl_s *kls;
	struct ehht_s *ehht;
	struct ehht_table_s *table;
	char *str_copy;
	size_t size;

	kls = (struct ehht_kl_s *)context;
	ehht = kls->ehht;

	assert(each_val == ehht->get(ehht, key.str, key.len));
	assert(kls->pos < kls->keys->len);

	if (kls->pos >= kls->keys->len) {
		return 1;
	}

	if (kls->keys->keys_copied) {
		table = ehht_get_table(ehht);
		size = sizeof(char *) * (key.len + 1);
		str_copy = table->alloc(size, table->mem_context);
		if (!str_copy) {
			assert(errno == ENOMEM);
			return 1;
		}
		memcpy(str_copy, key.str, key.len + 1);
		kls->keys->keys[kls->pos].str = str_copy;
		kls->keys->keys[kls->pos].len = key.len;
		kls->keys->keys[kls->pos].hashcode = key.hashcode;
	} else {
		kls->keys->keys[kls->pos] = key;
	}

	++kls->pos;

	return 0;
}

static int ehht_has_key(struct ehht_s *this, const char *key, size_t key_len)
{
	struct ehht_table_s *table;
	struct ehht_element_s *element;

	table = ehht_get_table(this);

	element = ehht_get_element(table, key, key_len);
	return (element != NULL);
}

static struct ehht_keys_s *ehht_keys(struct ehht_s *this, int copy_keys)
{
	struct ehht_table_s *table;
	struct ehht_kl_s kls;
	size_t size;

	table = ehht_get_table(this);

	kls.ehht = this;
	size = sizeof(struct ehht_keys_s);
	kls.keys = table->alloc(size, table->mem_context);
	if (!kls.keys) {
		goto keys_alloc_failed_0;
	}
	kls.keys->keys_copied = copy_keys;
	kls.keys->len = this->size(this);
	size = sizeof(struct ehht_key_s) * kls.keys->len;
	kls.keys->keys = table->alloc(size, table->mem_context);
	if (!kls.keys->keys) {
		goto keys_alloc_failed_1;
	}
	kls.pos = 0;

	ehht_for_each(this, ehht_fill_keys_each, &kls);

	return kls.keys;

keys_alloc_failed_1:
	table->free(kls.keys, table->mem_context);
keys_alloc_failed_0:
	assert(errno == ENOMEM);
	return NULL;
}

static void ehht_free_keys(struct ehht_s *this, struct ehht_keys_s *keys)
{
	struct ehht_table_s *table;
	size_t i;

	table = ehht_get_table(this);
	if (keys->keys_copied) {
		for (i = 0; i < keys->len; ++i) {
			table->free((void *)keys->keys[i].str,
				    table->mem_context);
		}
	}
	table->free(keys->keys, table->mem_context);
	table->free(keys, table->mem_context);
}

static void *ehht_malloc(size_t size, void *context)
{
	assert(context == NULL);
	return malloc(size);
}

static void ehht_mem_free(void *ptr, void *context)
{
	assert(context == NULL);
	free(ptr);
}

struct ehht_s *ehht_new(size_t num_buckets, ehht_hash_func hash_func,
			ehht_malloc_func mem_alloc, ehht_free_func mem_free,
			void *mem_context)
{
	struct ehht_s *this;
	struct ehht_table_s *table;
	size_t i;

	if (num_buckets == 0) {
		num_buckets = 4096;
	}
	if (hash_func == NULL) {
		hash_func = ehht_kr2_hashcode;
	}
	if (mem_alloc == NULL) {
		mem_alloc = ehht_malloc;
	}
	if (mem_free == NULL) {
		mem_free = ehht_mem_free;
	}

	this = mem_alloc(sizeof(struct ehht_s), mem_context);
	if (this == NULL) {
		return NULL;
	}
	this->get = ehht_get;
	this->put = ehht_put;
	this->remove = ehht_remove;
	this->size = ehht_size;
	this->clear = ehht_clear;
	this->for_each = ehht_for_each;
	this->has_key = ehht_has_key;
	this->keys = ehht_keys;
	this->free_keys = ehht_free_keys;
	this->to_string = ehht_to_string;
	this->num_buckets = ehht_num_buckets;
	this->resize = ehht_resize;
	this->bucket_for_key = ehht_bucket_for_key;

	table = mem_alloc(sizeof(struct ehht_table_s), mem_context);
	if (table == NULL) {
		mem_free(this, mem_context);
		return NULL;
	}
	ehht_set_table(this, table);

	table->num_buckets = num_buckets;
	table->buckets =
	    mem_alloc(sizeof(struct ehht_element_s *) * num_buckets,
		      mem_context);
	if (table->buckets == NULL) {
		mem_free(table, mem_context);
		mem_free(this, mem_context);
		return NULL;
	}
	for (i = 0; i < num_buckets; ++i) {
		table->buckets[i] = NULL;
	}
	table->size = 0;

	table->hash_func = hash_func;
	table->alloc = mem_alloc;
	table->free = mem_free;
	table->mem_context = mem_context;

	return this;
}

void ehht_free(struct ehht_s *this)
{
	struct ehht_table_s *table;
	ehht_free_func free_func;
	void *mem_context;

	if (this == NULL) {
		return;
	}

	table = ehht_get_table(this);

	this->clear(this);

	free_func = table->free;
	mem_context = table->mem_context;

	free_func(table->buckets, mem_context);
	free_func(table, mem_context);
	free_func(this, mem_context);
}
