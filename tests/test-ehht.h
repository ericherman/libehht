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
#include "../submodules/context-alloc/util/oom-injecting-malloc.h"

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
