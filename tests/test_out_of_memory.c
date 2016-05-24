#include "test-ehht.h"

int test_out_of_memory()
{
	int failures = 0;
	struct ehht_s *table;
	char buf[40];
	size_t i;
	struct mem_context ctx = { 0, 0, 0, 0, 0, 0 };

	ctx.malloc_multiplier = 8 * 1024;
	table = ehht_new(0, NULL, test_malloc, test_free, &ctx);

	if (table == NULL) {
		++failures;
		return failures;
	}

	check_int(errno, 0);

	for (i = 0; i == table->size(table); ++i) {
		if (i % 256 == 0) {
			fprintf(stderr, "\r%lu,", (unsigned long)i);
		}
		sprintf(buf, "_%lu_", (unsigned long)i);
		table->put(table, buf, strlen(buf), NULL);
		if (i % 256 == 0) {
			fprintf(stderr, "%lu ...",
				(unsigned long)table->size(table));
		}
	}
	fprintf(stderr, "\n");
	table->clear(table);
	check_int(errno, ENOMEM);
	errno = 0;

	ehht_free(table);
	return failures;
}

int main(int argc, char **argv)
{
	int failures = 0;
	int test_oom = (argc > 1) ? atoi(argv[1]) : 0;

	if (test_oom) {
		failures += test_out_of_memory();
	} else {
		fprintf(stderr, "skipping test_out_of_memory\n");
	}

	if (failures) {
		fprintf(stderr, "%d failures in total\n", failures);
	}
	return failures ? 1 : 0;
}
