/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehht.c: a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include "ehht.h"
#include "eembed.h"

#ifndef EHHT_DEFAULT_BUCKETS
#define EHHT_DEFAULT_BUCKETS 64
#endif

#ifndef EHHT_DEFAULT_RESIZE_LOADFACTOR
/* we split when we collide and we have a load factor over 0.667 */
/* https://github.com/Perl/perl5/blob/blead/hv.c#L37 */
#define EHHT_DEFAULT_RESIZE_LOADFACTOR (2.0/3.0)
#endif

#define Ehht_error_malloc(log, err_num, bytes, thing) \
	do { if (log) { \
		log->append_s(log, __FILE__); \
		log->append_s(log, ":"); \
		log->append_ul(log, __LINE__); \
		log->append_s(log, " Ehht Error "); \
		log->append_l(log, err_num); \
		log->append_s(log, ": could not allocate "); \
		log->append_ul(log, bytes); \
		log->append_s(log, " bytes ("); \
		log->append_s(log, thing); \
		log->append_s(log, ")"); \
		log->append_eol(log); \
	} } while (0)

#define Ehht_error(log, err_num, msg) \
	do { if (log) { \
		log->append_s(log, __FILE__); \
		log->append_s(log, ":"); \
		log->append_ul(log, __LINE__); \
		log->append_s(log, " Error "); \
		log->append_l(log, err_num); \
		log->append_s(log, ": "); \
		log->append_s(log, msg); \
		log->append_eol(log); \
	} } while (0)

struct ehht_element {
	struct ehht_key key;
	void *val;
	struct ehht_element *next;
};

struct ehht_table {
	size_t num_buckets;
	struct ehht_element **buckets;
	size_t size;
	ehht_hash_func hash_func;
	struct eembed_allocator *ea;
	struct eembed_log *log;
	double collision_load_factor;
	int trust_keys_immutable;
};

static void ehht_set_table(struct ehht *ht, struct ehht_table *table)
{
	eembed_assert(ht);
	ht->data = (void *)table;
}

static struct ehht_table *ehht_get_table(struct ehht *ht)
{
	eembed_assert(ht);
	eembed_assert(ht->data);
	return (struct ehht_table *)ht->data;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
static void ehht_free_const_str(struct eembed_allocator *ea, const char *str)
{
	void *ptr = (void *)str;
	ea->free(ea, ptr);
}

#pragma GCC diagnostic pop

static void ehht_free_element(struct ehht_table *table,
			      struct ehht_element *element)
{
	struct eembed_allocator *ea = table->ea;
	if (!table->trust_keys_immutable) {
		ehht_free_const_str(ea, element->key.str);
	}
	ea->free(ea, element);
}

static void ehht_clear(struct ehht *ht)
{
	struct ehht_table *table = NULL;
	size_t i = 0;

	table = ehht_get_table(ht);

	for (i = 0; i < table->num_buckets; ++i) {
		struct ehht_element *element = NULL;
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

size_t ehht_bucket_for_key(struct ehht *ht, const char *key, size_t key_len)
{
	struct ehht_table *table = NULL;
	unsigned int hashcode = 0;

	table = ehht_get_table(ht);
	hashcode = table->hash_func(key, key_len);

	return ehht_bucket_for_hashcode(hashcode, table->num_buckets);
}

static struct ehht_element *ehht_alloc_element(struct ehht_table *table,
					       const char *key,
					       size_t key_len,
					       unsigned int hashcode, void *val)
{
	struct eembed_allocator *ea = NULL;
	char *key_copy = NULL;
	struct ehht_element *element = NULL;
	size_t size = 0;

	size = sizeof(struct ehht_element);
	ea = table->ea;
	element = (struct ehht_element *)ea->malloc(ea, size);
	if (element == NULL) {
		Ehht_error_malloc(table->log, 1, size, "struct ehht_element");
		return NULL;
	}
	eembed_memset(element, 0x00, size);

	if (table->trust_keys_immutable) {
		element->key.str = key;
	} else {
		size = key_len + 1;
		eembed_assert(size > 0);
		ea = table->ea;
		key_copy = (char *)ea->malloc(ea, size);
		if (!key_copy) {
			Ehht_error_malloc(table->log, 2, size, "key copy");
			ehht_free_element(table, element);
			return NULL;
		}
		eembed_memset(key_copy, 0x00, size);
		eembed_memcpy(key_copy, key, key_len);
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

static struct ehht_element *ehht_get_element(struct ehht_table *table,
					     const char *key, size_t key_len)
{
	struct ehht_element *element = NULL;
	unsigned int hashcode = 0;
	size_t bucket_num = 0;

	hashcode = table->hash_func(key, key_len);
	bucket_num = ehht_bucket_for_hashcode(hashcode, table->num_buckets);

	element = table->buckets[bucket_num];
	while (element != NULL) {
		if (element->key.len == key_len) {
			if (eembed_memcmp(key, element->key.str, key_len) == 0) {
				return element;
			}
		}
		element = element->next;
	}
	return NULL;
}

static void *ehht_get(struct ehht *ht, const char *key, size_t key_len)
{
	struct ehht_table *table = NULL;
	struct ehht_element *element = NULL;

	table = ehht_get_table(ht);

	element = ehht_get_element(table, key, key_len);
	return (element == NULL) ? NULL : element->val;
}

static void *ehht_put(struct ehht *ht, const char *key, size_t key_len,
		      void *val, int *err)
{
	struct ehht_table *table = NULL;
	struct ehht_element *element = NULL;
	void *old_val = NULL;
	unsigned int hashcode = 0;
	unsigned int collision = 0;
	size_t bucket_num = 0;

	table = ehht_get_table(ht);

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
			ehht_buckets_resize(ht, 0);
		}
	}

	element = ehht_alloc_element(table, key, key_len, hashcode, val);
	if (!element) {
		if (err) {
			*err = 1;
		}
		Ehht_error(table->log, 3, "ehht_put failed");
		return NULL;
	}

	bucket_num = ehht_bucket_for_hashcode(hashcode, table->num_buckets);
	element->next = table->buckets[bucket_num];
	table->buckets[bucket_num] = element;

	return NULL;
}

static void *ehht_remove(struct ehht *ht, const char *key, size_t key_len)
{
	struct ehht_table *table = NULL;
	struct ehht_element *element = NULL;
	struct ehht_element **ptr_to_element = NULL;
	void *old_val = 0;
	unsigned int hashcode = 0;
	size_t bucket_num = 0;

	table = ehht_get_table(ht);

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

static int ehht_for_each(struct ehht *ht,
			 int (*func)(struct ehht_key each_key,
				     void *each_val, void *context),
			 void *context)
{
	struct ehht_table *table = NULL;
	size_t i = 0;
	size_t end = 0;
	struct ehht_element *element = NULL;

	table = ehht_get_table(ht);

	end = 0;
	for (i = 0; i < table->num_buckets && !end; ++i) {
		for (element = table->buckets[i]; element != NULL;
		     element = element->next) {
			end = (*func) (element->key, element->val, context);
		}
	}

	return end;
}

static size_t ehht_size(struct ehht *ht)
{
	struct ehht_table *table = NULL;
	size_t size = 0;

	table = ehht_get_table(ht);
	size = table->size;

	return size;
}

static int ehht_to_string_each(struct ehht_key key, void *each_val,
			       void *context)
{
	struct eembed_log *slog = (struct eembed_log *)context;

	slog->append_s(slog, "'");
	slog->append_s(slog, key.len ? key.str : "");
	slog->append_s(slog, "' => ");
	slog->append_vp(slog, each_val);
	slog->append_s(slog, ", ");

	return 0;
}

static size_t ehht_to_string(struct ehht *ht, char *buf, size_t buf_len)
{
	struct eembed_str_buf str_buf = { NULL, 0 };
	struct eembed_log log =
	    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	struct eembed_log *slog = NULL;

	slog = eembed_char_buf_log_init(&log, &str_buf, buf, buf_len);
	if (!slog) {
		return 0;
	}

	slog->append_s(slog, "{ ");
	ehht_for_each(ht, ehht_to_string_each, slog);
	slog->append_s(slog, "}");

	return eembed_strnlen(buf, buf_len);
}

size_t ehht_buckets_resize(struct ehht *ht, size_t num_buckets)
{
	size_t i = 0;
	size_t old_num_buckets = 0;
	size_t new_bucket_num = 0;
	size_t size = 0;
	struct ehht_table *table = NULL;
	struct ehht_element **new_buckets = NULL;
	struct ehht_element **old_buckets = NULL;
	struct eembed_allocator *ea = NULL;

	table = ehht_get_table(ht);
	ea = table->ea;

	if (num_buckets == 0) {
		num_buckets = table->num_buckets * 2;
	}
	eembed_assert(num_buckets > 1);
	size = sizeof(struct ehht_element *) * num_buckets;
	eembed_assert(size > 0);
	new_buckets = (struct ehht_element **)ea->malloc(ea, size);
	if (new_buckets == NULL) {
		Ehht_error_malloc(table->log, 4, size, "buckets");
		return table->num_buckets;
	}
	eembed_assert(size > 0);
	eembed_memset(new_buckets, 0x00, size);

	old_num_buckets = table->num_buckets;
	old_buckets = table->buckets;
	for (i = 0; i < old_num_buckets; ++i) {
		struct ehht_element *element = NULL;
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

	ea->free(ea, old_buckets);
	return num_buckets;
}

size_t ehht_buckets_size(struct ehht *ht)
{
	struct ehht_table *table = NULL;

	table = ehht_get_table(ht);
	return table->num_buckets;
}

struct ehht_keys_foreach_context {
	struct ehht *ehht;
	struct ehht_keys *keys;
	size_t pos;
};

static int ehht_fill_keys_each(struct ehht_key key, void *each_val,
			       void *context)
{
	struct ehht_keys_foreach_context *fe_ctx = NULL;
	struct ehht *ehht = NULL;
	struct eembed_allocator *ea = NULL;

	fe_ctx = (struct ehht_keys_foreach_context *)context;
	ehht = fe_ctx->ehht;

	eembed_assert(each_val == ehht->get(ehht, key.str, key.len));
	(void)each_val;

	eembed_assert(fe_ctx->pos < fe_ctx->keys->len);
	if (fe_ctx->pos >= fe_ctx->keys->len) {
		return 1;
	}

	if (fe_ctx->keys->keys_copied) {
		struct ehht_table *table = NULL;
		size_t size = 0;
		char *key_copy = NULL;

		table = ehht_get_table(ehht);

		size = sizeof(char *) * (key.len + 1);
		eembed_assert(size > 0);
		ea = table->ea;
		key_copy = (char *)ea->malloc(ea, size);
		if (!key_copy) {
			Ehht_error_malloc(table->log, 5, size, "key copy");
			return 1;
		}
		eembed_memset(key_copy, 0x00, size);

		eembed_memcpy(key_copy, key.str, key.len + 1);
		fe_ctx->keys->keys[fe_ctx->pos].str = key_copy;
		fe_ctx->keys->keys[fe_ctx->pos].len = key.len;
		fe_ctx->keys->keys[fe_ctx->pos].hashcode = key.hashcode;
	} else {
		fe_ctx->keys->keys[fe_ctx->pos] = key;
	}

	++fe_ctx->pos;

	return 0;
}

static int ehht_has_key(struct ehht *ht, const char *key, size_t key_len)
{
	struct ehht_table *table = NULL;
	struct ehht_element *element = NULL;

	table = ehht_get_table(ht);

	element = ehht_get_element(table, key, key_len);
	return (element == NULL) ? 0 : 1;
}

static void ehht_free_keys(struct ehht *ht, struct ehht_keys *keys)
{
	struct ehht_table *table = NULL;
	struct eembed_allocator *ea = NULL;

	table = ehht_get_table(ht);
	ea = table->ea;

	if (keys->keys_copied) {
		size_t i;
		for (i = 0; i < keys->len; ++i) {
			ehht_free_const_str(ea, keys->keys[i].str);
		}
	}
	if (keys->keys) {
		ea->free(ea, keys->keys);
	}
	ea->free(ea, keys);
}

static struct ehht_keys *ehht_keys (struct ehht *ht, int copy_keys) {
	struct ehht_table *table = NULL;
	struct eembed_allocator *ea = NULL;
	struct ehht_keys_foreach_context fe_ctx = { NULL, NULL, 0 };
	size_t size = 0;

	table = ehht_get_table(ht);
	ea = table->ea;

	fe_ctx.ehht = ht;

	size = sizeof(struct ehht_keys);
	fe_ctx.keys = (struct ehht_keys *)ea->malloc(ea, size);
	if (!fe_ctx.keys) {
		Ehht_error_malloc(table->log, 6, size, "struct ehht_keys");
		return NULL;
	}
	eembed_memset(fe_ctx.keys, 0x00, size);
	fe_ctx.pos = 0;
	fe_ctx.keys->len = ht->size(ht);
	fe_ctx.keys->keys_copied = copy_keys;

	size = sizeof(struct ehht_key) * fe_ctx.keys->len;
	if (size == 0) {
		fe_ctx.keys->keys = NULL;
	} else {
		fe_ctx.keys->keys = (struct ehht_key *)ea->malloc(ea, size);
		if (!fe_ctx.keys->keys) {
			Ehht_error_malloc(table->log, 7, size, "key list");
			ea->free(ea, fe_ctx.keys);
			return NULL;
		}
		eembed_memset(fe_ctx.keys->keys, 0x00, size);
		if (ehht_for_each(ht, ehht_fill_keys_each, &fe_ctx)) {
			ehht_free_keys(ht, fe_ctx.keys);
			Ehht_error(table->log, 8, "ehht_keys failed");
			return NULL;
		}
	}

	return fe_ctx.keys;
}

void ehht_buckets_auto_resize_load_factor(struct ehht *ht, double factor)
{
	struct ehht_table *table = NULL;

	table = ehht_get_table(ht);
	table->collision_load_factor = factor;
}

int ehht_trust_keys_immutable(struct ehht *ht, int val)
{
	struct ehht_table *table = NULL;
	int new_val = val ? 1 : 0;

	table = ehht_get_table(ht);
	if (table->size && new_val != table->trust_keys_immutable) {
		Ehht_error(table->log, 12,
			   "invalid attempt to change key immutablity");
		return 1;
	}
	table->trust_keys_immutable = new_val;
	return 0;
}

struct ehht *ehht_new(void)
{
	size_t num_buckets = 0;
	struct eembed_allocator *ea = NULL;
	ehht_hash_func hash_func = NULL;
	struct eembed_log *log = NULL;

	return ehht_new_custom(num_buckets, hash_func, ea, log);
}

struct ehht *ehht_new_custom(size_t num_buckets, ehht_hash_func hash_func,
			     struct eembed_allocator *ea,
			     struct eembed_log *log)
{
	struct ehht *ht = NULL;
	struct ehht_table *table = NULL;
	size_t size = 0;

	if (num_buckets == 0) {
		num_buckets = EHHT_DEFAULT_BUCKETS;
	}
	if (hash_func == NULL) {
		hash_func = ehht_kr2_hashcode;
	}
	if (ea == NULL) {
		ea = eembed_global_allocator;
	}
	if (log == NULL) {
		log = eembed_err_log;
	}

	size = sizeof(struct ehht);
	ht = (struct ehht *)ea->malloc(ea, size);
	if (ht == NULL) {
		Ehht_error_malloc(log, 9, size, "struct ehht");
		return NULL;
	}
	eembed_memset(ht, 0x00, size);

	ht->get = ehht_get;
	ht->put = ehht_put;
	ht->remove = ehht_remove;
	ht->size = ehht_size;
	ht->clear = ehht_clear;
	ht->for_each = ehht_for_each;
	ht->has_key = ehht_has_key;
	ht->keys = ehht_keys;
	ht->free_keys = ehht_free_keys;
	ht->to_string = ehht_to_string;

	size = sizeof(struct ehht_table);
	table = (struct ehht_table *)ea->malloc(ea, size);
	if (table == NULL) {
		Ehht_error_malloc(log, 10, size, "struct ehht_table");
		ea->free(ea, ht);
		return NULL;
	}
	eembed_memset(table, 0x00, size);
	ehht_set_table(ht, table);

	table->hash_func = hash_func;
	table->ea = ea;
	table->log = log;

	size = sizeof(struct ehht_element *) * num_buckets;
	table->buckets = (struct ehht_element **)ea->malloc(ea, size);
	if (table->buckets == NULL) {
		Ehht_error_malloc(table->log, 11, size, "buckets");
		ea->free(ea, table);
		ea->free(ea, ht);
		return NULL;
	}
	eembed_memset(table->buckets, 0x00, size);
	table->num_buckets = num_buckets;
	table->size = 0;

	table->collision_load_factor = EHHT_DEFAULT_RESIZE_LOADFACTOR;
	table->trust_keys_immutable = 0;

	return ht;
}

void ehht_free(struct ehht *ht)
{
	struct ehht_table *table = NULL;
	struct eembed_allocator *ea = NULL;

	if (ht == NULL) {
		return;
	}

	table = ehht_get_table(ht);
	ea = table->ea;

	ht->clear(ht);

	ea->free(ea, table->buckets);
	ea->free(ea, table);
	ea->free(ea, ht);
}
