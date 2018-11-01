#include <stdio.h>		/* fprintf */
#include <string.h>		/* strlen */
#include <errno.h>

#include "../src/ehht.h"
#include "../src/ehht-report.h"
#define REPORT_LEN 10
#include "echeck.h"

struct mem_context {
	unsigned allocs;
	unsigned alloc_bytes;
	unsigned frees;
	unsigned free_bytes;
	unsigned fails;
	unsigned malloc_multiplier;
};

void *test_malloc(size_t size, void *context)
{
	struct mem_context *ctx = (struct mem_context *)context;
	++ctx->allocs;
	ctx->alloc_bytes += size;
	if (ctx->malloc_multiplier) {
		size *= ctx->malloc_multiplier;
	}
	return malloc(size);
}

void test_free(void *ptr, size_t size, void *context)
{
	struct mem_context *ctx = (struct mem_context *)context;
	++ctx->frees;
	ctx->free_bytes += size;
	if (ptr == NULL) {
		++ctx->fails;
	}
	free(ptr);
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
