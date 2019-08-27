/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test-ehht.h: testing header for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include <stdio.h>		/* fprintf */
#include <string.h>		/* strlen memcpy */
#include <errno.h>

#include "../src/ehht.h"
#include "ehht-report.h"
#define REPORT_LEN 10
#include "echeck.h"

struct tracking_mem_context {
	unsigned allocs;
	unsigned alloc_bytes;
	unsigned frees;
	unsigned free_bytes;
	unsigned fails;
	unsigned max_used;
	unsigned malloc_multiplier;
};

void *test_malloc(size_t size, void *context)
{
	struct tracking_mem_context *ctx;
	unsigned char *tracking_buffer;
	void *ptr;
	size_t used, size2;

	ptr = NULL;
	ctx = (struct tracking_mem_context *)context;
	if (ctx->malloc_multiplier) {
		size2 = size * ctx->malloc_multiplier;
	} else {
		size2 = size;
	}
	tracking_buffer = malloc(sizeof(size_t) + size2);
	if (!tracking_buffer) {
		++ctx->fails;
		return NULL;
	}

	ptr = (void *)(tracking_buffer + sizeof(size_t));
	memcpy(tracking_buffer, &size, sizeof(size_t));
	++ctx->allocs;
	ctx->alloc_bytes += size;
	used = ctx->alloc_bytes - ctx->free_bytes;
	if (used > ctx->max_used) {
		ctx->max_used = used;
	}
	return ptr;
}

void test_free(void *ptr, void *context)
{
	struct tracking_mem_context *ctx;
	unsigned char *tracking_buffer;
	size_t size;

	ctx = (struct tracking_mem_context *)context;
	if (ptr == NULL) {
		++ctx->fails;
		return;
	}
	tracking_buffer = ((unsigned char *)ptr) - sizeof(size_t);
	memcpy(&size, tracking_buffer, sizeof(size_t));
	ctx->free_bytes += size;
	++ctx->frees;
	free(tracking_buffer);
}

#define TEST_EHHT_MAIN(func) \
int main(void) \
{ \
	int failures = 0; \
	failures += func; \
	if (failures) { \
		fprintf(stderr, "%d failures in %s\n", failures, __FILE__); \
	} \
	return failures ? 1 : 0; \
}
