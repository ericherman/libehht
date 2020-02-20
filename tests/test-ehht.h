/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test-ehht.h: testing header for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include <stdio.h>		/* fprintf */
#include <string.h>		/* strlen memcpy */
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#include "../src/ehht.h"
#include "ehht-report.h"
#define REPORT_LEN 10
#include "../submodules/libecheck/src/echeck.h"

struct tracking_mem_context {
	uint64_t allocs;
	uint64_t alloc_bytes;
	uint64_t frees;
	uint64_t free_bytes;
	uint64_t fails;
	uint64_t max_used;
	uint64_t attempts;
	uint64_t attempts_to_fail_bitmask;
};

void *test_malloc(size_t size, void *context)
{
	struct tracking_mem_context *ctx = NULL;
	unsigned char *tracking_buffer = NULL;
	void *ptr = NULL;
	size_t used = 0;

	ptr = NULL;
	ctx = (struct tracking_mem_context *)context;
	if (0x01 & (ctx->attempts_to_fail_bitmask >> ctx->attempts++)) {
		return NULL;
	}
	tracking_buffer = malloc(sizeof(size_t) + size);
	if (!tracking_buffer) {
		++ctx->fails;
		return NULL;
	}

	memcpy(tracking_buffer, &size, sizeof(size_t));
	++ctx->allocs;
	ctx->alloc_bytes += size;
	if (ctx->free_bytes > ctx->alloc_bytes) {
		fprintf(stderr,
			"%s: %d BAD MOJO: free_bytes > alloc_bytes?! (%lu > %lu)\n",
			__FILE__, __LINE__, (unsigned long)ctx->free_bytes,
			(unsigned long)ctx->alloc_bytes);
	} else {
		used = ctx->alloc_bytes - ctx->free_bytes;
		if (used > ctx->max_used) {
			ctx->max_used = used;
		}
	}
	ptr = (void *)(tracking_buffer + sizeof(size_t));
	return ptr;
}

void test_free(void *ptr, void *context)
{
	struct tracking_mem_context *ctx = NULL;
	unsigned char *tracking_buffer = NULL;
	size_t size = 0;

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
	if (ctx->free_bytes > ctx->alloc_bytes) {
		fprintf(stderr,
			"%s: %d BAD MOJO: free_bytes > alloc_bytes?! (%lu > %lu) just freed %lu\n",
			__FILE__, __LINE__, (unsigned long)ctx->free_bytes,
			(unsigned long)ctx->alloc_bytes, (unsigned long)size);
	}

}

struct ehht_sprintf_context_s {
	char *buf;
	size_t size;
};

int ehht_sprintf(void *log_context, const char *format, ...)
{
	int ret;
	struct ehht_sprintf_context_s *ctx = NULL;
	size_t len;
	va_list args;

	ctx = (struct ehht_sprintf_context_s *)log_context;
	assert(ctx != NULL);

	va_start(args, format);

	len = strlen(ctx->buf);
	assert(len < ctx->size);
	ret = vsprintf(ctx->buf + len, format, args);
	assert(ret >= 0);

	va_end(args);

	return ret;
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
