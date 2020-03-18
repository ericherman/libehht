/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehht.c: a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include <stdlib.h>		/* malloc calloc */
#include <string.h>		/* memcpy memcmp */
#include <stdio.h>		/* fprintf */
#include <assert.h>
#include <stdarg.h>

#ifndef Ehht_memset
#define Ehht_memset(ptr, byte, len) memset(ptr, byte, len)
#endif

#ifndef Ehht_memcpy
#define Ehht_memcpy(dest, src, len) memcpy(dest, src, len)
#endif

#ifndef EHHT_DEFAULT_BUCKETS
#define EHHT_DEFAULT_BUCKETS 64
#endif

#ifndef EHHT_DEFAULT_RESIZE_LOADFACTOR
/* we split when we collide and we have a load factor over 0.667 */
/* https://github.com/Perl/perl5/blob/blead/hv.c#L37 */
#define EHHT_DEFAULT_RESIZE_LOADFACTOR (2.0/3.0)
#endif

#define Ehht_error_malloc(logfunc, logctx, err_num, bytes, thing) \
	do { if (logfunc) { \
		logfunc(logctx, \
			"%s:%d Error %u: could not allocate %lu bytes (%s)\n", \
			__FILE__, __LINE__, err_num, (unsigned long)(bytes), \
			thing); \
	} } while (0)

#define Ehht_error(logfunc, logctx, err_num, msg) \
	do { if (logfunc) { \
		logfunc(logctx, "%s:%d Error %u: %s\n", __FILE__, __LINE__, \
			err_num, msg); \
	} } while (0)

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
	double collision_load_factor;
	ehht_log_error_func err_printf;
	void *err_context;
	int trust_keys_immutable;
};

static void ehht_set_table(struct ehht_s *this, struct ehht_table_s *table)
{
	this->data = (void *)table;
}

static struct ehht_table_s *ehht_get_table(struct ehht_s *this)
{
	return (struct ehht_table_s *)this->data;
}

static int ehht_fprintf(void *err_context, const char *format, ...)
{
	int ret = 0;
	va_list args;
	FILE *log;

	log = (FILE *)err_context;

	va_start(args, format);

	ret = vfprintf(log, format, args);

	va_end(args);

	return ret;
}

static void ehht_free_element(struct ehht_table_s *table,
			      struct ehht_element_s *element)
{
	if (!table->trust_keys_immutable) {
		table->free(table->mem_context, (char *)element->key.str);
	}
	table->free(table->mem_context, element);
}

static void ehht_clear(struct ehht_s *this)
{
	struct ehht_table_s *table = NULL;
	size_t i = 0;

	table = ehht_get_table(this);

	for (i = 0; i < table->num_buckets; ++i) {
		struct ehht_element_s *element = NULL;
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

size_t ehht_bucket_for_key(struct ehht_s *this, const char *key, size_t key_len)
{
	struct ehht_table_s *table = NULL;
	unsigned int hashcode = 0;

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
	char *key_copy = NULL;
	struct ehht_element_s *element = NULL;
	size_t size = 0;

	size = sizeof(struct ehht_element_s);
	element = table->alloc(table->mem_context, size);
	if (element == NULL) {
		Ehht_error_malloc(table->err_printf, table->err_context, 1,
				  size, "struct ehht_element_s");
		return NULL;
	}
	Ehht_memset(element, 0x00, size);

	if (table->trust_keys_immutable) {
		element->key.str = key;
	} else {
		size = key_len + 1;
		assert(size > 0);
		key_copy = table->alloc(table->mem_context, size);
		if (!key_copy) {
			Ehht_error_malloc(table->err_printf, table->err_context,
					  2, size, "key copy");
			ehht_free_element(table, element);
			return NULL;
		}
		Ehht_memset(key_copy, 0x00, size);
		Ehht_memcpy(key_copy, key, key_len);
		key_copy[key_len] = '\0';

		element->key.str = key_copy;
	}

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
	unsigned int hash = 0;
	size_t i = 0;

	for (i = 0; i < len; ++i) {
		hash *= 31;
		hash += (unsigned int)data[i];
	}

	return hash;
}

static struct ehht_element_s *ehht_get_element(struct ehht_table_s *table,
					       const char *key, size_t key_len)
{
	struct ehht_element_s *element = NULL;
	unsigned int hashcode = 0;
	size_t bucket_num = 0;

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
	struct ehht_table_s *table = NULL;
	struct ehht_element_s *element = NULL;

	table = ehht_get_table(this);

	element = ehht_get_element(table, key, key_len);
	return (element == NULL) ? NULL : element->val;
}

static void *ehht_put(struct ehht_s *this, const char *key, size_t key_len,
		      void *val, int *err)
{
	struct ehht_table_s *table = NULL;
	struct ehht_element_s *element = NULL;
	void *old_val = NULL;
	unsigned int hashcode = 0;
	unsigned int collision = 0;
	size_t bucket_num = 0;

	table = ehht_get_table(this);

	element = ehht_get_element(table, key, key_len);
	old_val = (element == NULL) ? NULL : element->val;

	if (element != NULL) {
		element->val = val;
		return old_val;
	}

	hashcode = table->hash_func(key, key_len);
	bucket_num = ehht_bucket_for_hashcode(hashcode, table->num_buckets);
	collision = (table->buckets[bucket_num] == NULL) ? 0 : 1;
	if (collision && table->collision_load_factor > 0.0) {
		if (table->size >=
		    (table->num_buckets * table->collision_load_factor)) {
			ehht_buckets_resize(this, 0);
		}
	}

	element = ehht_alloc_element(table, key, key_len, hashcode, val);
	if (!element) {
		if (err) {
			*err = 1;
		}
		Ehht_error(table->err_printf, table->err_context, 3,
			   "ehht_put failed");
		return NULL;
	}

	bucket_num = ehht_bucket_for_hashcode(hashcode, table->num_buckets);
	element->next = table->buckets[bucket_num];
	table->buckets[bucket_num] = element;

	return NULL;
}

static void *ehht_remove(struct ehht_s *this, const char *key, size_t key_len)
{
	struct ehht_table_s *table = NULL;
	struct ehht_element_s *element = NULL;
	struct ehht_element_s **ptr_to_element = NULL;
	void *old_val = 0;
	unsigned int hashcode = 0;
	size_t bucket_num = 0;

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
	struct ehht_table_s *table = NULL;
	size_t i = 0;
	size_t end = 0;
	struct ehht_element_s *element = NULL;

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
	struct ehht_table_s *table = NULL;

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

	struct ehht_str_buf_s *str_buf = NULL;
	char *buf = NULL;
	const char *fmt = NULL;
	size_t bytes_to_write = 0;
	int bytes_written = 0;

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
	struct ehht_str_buf_s str_buf = { NULL, 0, 0 };
	int bytes_written = 0;

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

size_t ehht_buckets_resize(struct ehht_s *this, size_t num_buckets)
{
	size_t i = 0;
	size_t old_num_buckets = 0;
	size_t new_bucket_num = 0;
	size_t size = 0;
	struct ehht_table_s *table = NULL;
	struct ehht_element_s **new_buckets = NULL;
	struct ehht_element_s **old_buckets = NULL;

	table = ehht_get_table(this);

	if (num_buckets == 0) {
		num_buckets = table->num_buckets * 2;
	}
	assert(num_buckets > 1);
	size = sizeof(struct ehht_element_s *) * num_buckets;
	assert(size > 0);
	new_buckets = table->alloc(table->mem_context, size);
	if (new_buckets == NULL) {
		Ehht_error_malloc(table->err_printf, table->err_context, 4,
				  size, "buckets");
		return table->num_buckets;
	}
	assert(size > 0);
	Ehht_memset(new_buckets, 0x00, size);

	old_num_buckets = table->num_buckets;
	old_buckets = table->buckets;
	for (i = 0; i < old_num_buckets; ++i) {
		struct ehht_element_s *element = NULL;
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

	table->free(table->mem_context, old_buckets);
	return num_buckets;
}

size_t ehht_buckets_size(struct ehht_s *this)
{
	struct ehht_table_s *table = NULL;

	table = ehht_get_table(this);
	return table->num_buckets;
}

struct ehht_keys_foreach_context_s {
	struct ehht_s *ehht;
	struct ehht_keys_s *keys;
	size_t pos;
};

static int ehht_fill_keys_each(struct ehht_key_s key, void *each_val,
			       void *context)
{
	struct ehht_keys_foreach_context_s *fe_ctx = NULL;
	struct ehht_s *ehht = NULL;

	fe_ctx = (struct ehht_keys_foreach_context_s *)context;
	ehht = fe_ctx->ehht;

	assert(each_val == ehht->get(ehht, key.str, key.len));
	assert(fe_ctx->pos < fe_ctx->keys->len);

	if (fe_ctx->pos >= fe_ctx->keys->len) {
		return 1;
	}

	if (fe_ctx->keys->keys_copied) {
		struct ehht_table_s *table = NULL;
		size_t size = 0;
		char *key_copy = NULL;

		table = ehht_get_table(ehht);

		size = sizeof(char *) * (key.len + 1);
		assert(size > 0);
		key_copy = table->alloc(table->mem_context, size);
		if (!key_copy) {
			Ehht_error_malloc(table->err_printf, table->err_context,
					  5, size, "key copy");
			return 1;
		}
		Ehht_memset(key_copy, 0x00, size);

		Ehht_memcpy(key_copy, key.str, key.len + 1);
		fe_ctx->keys->keys[fe_ctx->pos].str = key_copy;
		fe_ctx->keys->keys[fe_ctx->pos].len = key.len;
		fe_ctx->keys->keys[fe_ctx->pos].hashcode = key.hashcode;
	} else {
		fe_ctx->keys->keys[fe_ctx->pos] = key;
	}

	++fe_ctx->pos;

	return 0;
}

static int ehht_has_key(struct ehht_s *this, const char *key, size_t key_len)
{
	struct ehht_table_s *table = NULL;
	struct ehht_element_s *element = NULL;

	table = ehht_get_table(this);

	element = ehht_get_element(table, key, key_len);
	return (element == NULL) ? 0 : 1;
}

static void ehht_free_keys(struct ehht_s *this, struct ehht_keys_s *keys)
{
	struct ehht_table_s *table = NULL;

	table = ehht_get_table(this);
	if (keys->keys_copied) {
		size_t i;
		for (i = 0; i < keys->len; ++i) {
			void *ptr = (void *)keys->keys[i].str;
			table->free(table->mem_context, ptr);
		}
	}
	if (keys->keys) {
		table->free(table->mem_context, keys->keys);
	}
	table->free(table->mem_context, keys);
}

static struct ehht_keys_s *ehht_keys(struct ehht_s *this, int copy_keys)
{
	struct ehht_table_s *table = NULL;
	struct ehht_keys_foreach_context_s fe_ctx = { NULL, NULL, 0 };
	size_t size = 0;

	table = ehht_get_table(this);

	fe_ctx.ehht = this;

	size = sizeof(struct ehht_keys_s);
	fe_ctx.keys = table->alloc(table->mem_context, size);
	if (!fe_ctx.keys) {
		Ehht_error_malloc(table->err_printf, table->err_context, 6,
				  size, "struct ehht_keys_s");
		return NULL;
	}
	Ehht_memset(fe_ctx.keys, 0x00, size);
	fe_ctx.pos = 0;
	fe_ctx.keys->len = this->size(this);
	fe_ctx.keys->keys_copied = copy_keys;

	size = sizeof(struct ehht_key_s) * fe_ctx.keys->len;
	if (size == 0) {
		fe_ctx.keys->keys = NULL;
	} else {
		fe_ctx.keys->keys = table->alloc(table->mem_context, size);
		if (!fe_ctx.keys->keys) {
			Ehht_error_malloc(table->err_printf, table->err_context,
					  7, size, "key list");
			table->free(table->mem_context, fe_ctx.keys);
			return NULL;
		}
		Ehht_memset(fe_ctx.keys->keys, 0x00, size);
		if (ehht_for_each(this, ehht_fill_keys_each, &fe_ctx)) {
			ehht_free_keys(this, fe_ctx.keys);
			Ehht_error(table->err_printf, table->err_context, 8,
				   "ehht_keys failed");
			return NULL;
		}
	}

	return fe_ctx.keys;
}

static void *ehht_malloc(void *context, size_t size)
{
	assert(context == NULL);
	return malloc(size);
}

static void ehht_mem_free(void *context, void *ptr)
{
	assert(context == NULL);
	free(ptr);
}

void ehht_buckets_auto_resize_load_factor(struct ehht_s *this, double factor)
{
	struct ehht_table_s *table = NULL;

	table = ehht_get_table(this);
	table->collision_load_factor = factor;
}

int ehht_trust_keys_immutable(struct ehht_s *this, int val)
{
	struct ehht_table_s *table = NULL;
	int new_val = val ? 1 : 0;

	table = ehht_get_table(this);
	if (table->size && new_val != table->trust_keys_immutable) {
		Ehht_error(table->err_printf, table->err_context, 12,
			   "invalid attempt to change key immutablity");
		return 1;
	}
	table->trust_keys_immutable = new_val;
	return 0;
}

struct ehht_s *ehht_new(void)
{
	size_t num_buckets = 0;
	ehht_hash_func hash_func = NULL;
	ehht_malloc_func mem_alloc = NULL;
	ehht_free_func mem_free = NULL;
	void *mem_context = NULL;
	ehht_log_error_func err_func = NULL;
	void *err_context = stderr;

	return ehht_new_custom(num_buckets, hash_func, mem_alloc, mem_free,
			       mem_context, err_func, err_context);
}

struct ehht_s *ehht_new_custom(size_t num_buckets, ehht_hash_func hash_func,
			       ehht_malloc_func mem_alloc,
			       ehht_free_func mem_free, void *mem_context,
			       ehht_log_error_func err_func, void *err_context)
{
	struct ehht_s *this = NULL;
	struct ehht_table_s *table = NULL;
	size_t size = 0;

	if (num_buckets == 0) {
		num_buckets = EHHT_DEFAULT_BUCKETS;
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
	if (err_func == NULL && err_context != NULL) {
		err_func = ehht_fprintf;
	}

	size = sizeof(struct ehht_s);
	this = mem_alloc(mem_context, size);
	if (this == NULL) {
		Ehht_error_malloc(err_func, err_context, 9, size,
				  "struct ehht_s");
		return NULL;
	}
	Ehht_memset(this, 0x00, size);

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

	size = sizeof(struct ehht_table_s);
	table = mem_alloc(mem_context, size);
	if (table == NULL) {
		Ehht_error_malloc(err_func, err_context, 10, size,
				  "struct ehht_table_s");
		mem_free(mem_context, this);
		return NULL;
	}
	Ehht_memset(table, 0x00, size);
	ehht_set_table(this, table);

	table->hash_func = hash_func;
	table->alloc = mem_alloc;
	table->free = mem_free;
	table->mem_context = mem_context;
	table->err_printf = err_func;
	table->err_context = err_context;

	size = sizeof(struct ehht_element_s *) * num_buckets;
	table->buckets = mem_alloc(mem_context, size);
	if (table->buckets == NULL) {
		Ehht_error_malloc(err_func, err_context, 11, size, "buckets");
		mem_free(mem_context, table);
		mem_free(mem_context, this);
		return NULL;
	}
	Ehht_memset(table->buckets, 0x00, size);
	table->num_buckets = num_buckets;
	table->size = 0;

	table->collision_load_factor = EHHT_DEFAULT_RESIZE_LOADFACTOR;
	table->trust_keys_immutable = 0;

	return this;
}

void ehht_free(struct ehht_s *this)
{
	struct ehht_table_s *table = NULL;
	ehht_free_func ctx_free = NULL;
	void *mem_context = NULL;

	if (this == NULL) {
		return;
	}

	table = ehht_get_table(this);

	this->clear(this);
	if (table->err_printf == ehht_fprintf) {
		assert(table->err_context);
		fflush(table->err_context);
	}

	ctx_free = table->free;
	mem_context = table->mem_context;

	ctx_free(mem_context, table->buckets);
	ctx_free(mem_context, table);
	ctx_free(mem_context, this);
}
