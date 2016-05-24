lib_LTLIBRARIES=libehht.la

CSTD_CFLAGS=-std=c89
#DEBUG_CFLAGS=-ggdb -O0
DEBUG_CFLAGS=-ggdb -O3 -DNDEBUG -Wno-unused-parameter
NOISY_CFLAGS=-Wall -Wextra -pedantic -Werror
AM_CFLAGS=$(CSTD_CFLAGS) $(DEBUG_CFLAGS) $(NOISY_CFLAGS)

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

# https://github.com/ericherman/simple_stats
SSTATS=../simple_stats/sstats

ACLOCAL_AMFLAGS=-I m4 --install

DEMOS=$(bin_PROGRAMS)
bin_PROGRAMS=demo-ehht demo-ehht-as-array

demo_ehht_SOURCES=demos/leveldb_util_hash.c demos/demo-ehht.c \
 src/ehht.h src/ehht-report.h
demo_ehht_LDADD=libehht.la

demo_ehht_as_array_SOURCES=demos/demo-ehht-as-array.c src/ehht.h
demo_ehht_as_array_LDADD=libehht.la
demo_ehht_as_array_CFLAGS=$(AM_CFLAGS) -D_GNU_SOURCE

check_PROGRAMS=\
 test_ehht_new \
 test_ehht_put_get_remove \
 test_ehht_clear \
 test_ehht_foreach_element \
 test_ehht_keys \
 test_ehht_resize \
 test_out_of_memory


tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t \
		-T ehht_s \
		-T ehht_key_s \
		-T ehht_keys_s \
		`find src tests demos -name '*.h' -o -name '*.c'`

demo: $(DEMOS)
	libtool --mode=execute ./demo-ehht-as-array
	@echo ""
	for num_buckets in 64 128 256 512 1024 2048 4096; do \
		echo ""; echo "num buckets: $$num_buckets"; \
		libtool --mode=execute ./demo-ehht \
			$$num_buckets | $(SSTATS) --channels=2 -; \
	done

spotless:
	rm -rf `cat .gitignore | sed -e 's/#.*//'`
	pushd src && rm -rf `cat ../.gitignore | sed -e 's/#.*//'`; popd
	pushd tests && rm -rf `cat ../.gitignore | sed -e 's/#.*//'`; popd
	pushd demos && rm -rf `cat ../.gitignore | sed -e 's/#.*//'`; popd

valgrind: $(check_PROGRAMS)
	libtool --mode=execute valgrind -q ./test_ehht_new
	libtool --mode=execute valgrind -q ./test_ehht_put_get_remove
	libtool --mode=execute valgrind -q ./test_ehht_clear
	libtool --mode=execute valgrind -q ./test_ehht_foreach_element
	libtool --mode=execute valgrind -q ./test_ehht_keys
	libtool --mode=execute valgrind -q ./test_ehht_resize


libehht_la_SOURCES=src/ehht.c

include_HEADERS=src/ehht.h src/ehht-interface.h

TESTS=$(check_PROGRAMS)

T_COMMON_SOURCES=tests/test-ehht.h $(include_HEADERS)
T_COMMON_LDADD=libehht.la -lecheck

test_ehht_clear_SOURCES=tests/test_ehht_clear.c \
 $(T_COMMON_SOURCES)
test_ehht_clear_LDADD=$(T_COMMON_LDADD)

test_ehht_foreach_element_SOURCES=tests/test_ehht_foreach_element.c \
 $(T_COMMON_SOURCES)
test_ehht_foreach_element_LDADD=$(T_COMMON_LDADD)

test_ehht_keys_SOURCES=tests/test_ehht_keys.c \
 $(T_COMMON_SOURCES)
test_ehht_keys_LDADD=$(T_COMMON_LDADD)

test_ehht_new_SOURCES=tests/test_ehht_new.c \
 $(T_COMMON_SOURCES)
test_ehht_new_LDADD=$(T_COMMON_LDADD)

test_ehht_put_get_remove_SOURCES=tests/test_ehht_put_get_remove.c \
 $(T_COMMON_SOURCES)
test_ehht_put_get_remove_LDADD=$(T_COMMON_LDADD)

test_ehht_resize_SOURCES=tests/test_ehht_resize.c \
 $(T_COMMON_SOURCES)
test_ehht_resize_LDADD=$(T_COMMON_LDADD)

test_out_of_memory_SOURCES=tests/test_out_of_memory.c \
 $(T_COMMON_SOURCES)
test_out_of_memory_LDADD=$(T_COMMON_LDADD)
