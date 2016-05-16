lib_LTLIBRARIES=libehht.la
libehht_la_SOURCES=src/ehht.c

include_HEADERS=src/ehht.h

TESTS=$(check_PROGRAMS)
check_PROGRAMS=test-ehht
test_ehht_SOURCES=tests/test-ehht.c src/ehht.h src/ehht-report.h
test_ehht_LDADD=libehht.la -lecheck

DEMOS=$(demo_PROGRAMS)
demo_PROGRAMS=demo-ehht demo-as-array
demo_ehht_SOURCES=demos/leveldb_util_hash.c demos/demo-ehht.c \
 src/ehht.h src/ehht-report.h
demo_ehht_LDADD=libehht.la
demo_as_array_SOURCES=demos/demo-as-array.c src/ehht.h
demo_as_array_LDADD=libehht.la

ACLOCAL_AMFLAGS=-I m4 --install

CSTD_CFLAGS=-std=c89
#DEBUG_CFLAGS=-ggdb -O0
DEBUG_CFLAGS=-ggdb -O3 -DNDEBUG -Wno-unused-parameter
NOISY_CFLAGS=-Wall -Wextra -pedantic -Werror
AM_CFLAGS=$(CSTD_CFLAGS) $(DEBUG_CFLAGS) $(NOISY_CFLAGS)

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

# https://github.com/ericherman/simple_stats
SSTATS=../simple_stats/sstats

valgrind: test-ehht
	valgrind ./test-ehht

tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t \
		-T ehht_s \
		-T ehht_key_s \
		-T ehht_keys_s \
		`find src tests demos -name *.h -o -name *.c`

demo: $(DEMOS)
	for num_buckets in 64 128 256 512 1024 2048 4096; do \
		echo ""; echo "num buckets: $$num_buckets"; \
		./demo-ehht $$num_buckets | $(SSTATS) --channels=2 -; \
	done

spotless:
	rm -rf `cat .gitignore`
	( cd src; rm -rf `cat ../.gitignore`; cd .. )
	( cd tests; rm -rf `cat ../.gitignore`; cd .. )
	( cd demos ; rm -rf `cat ../.gitignore`; cd .. )
