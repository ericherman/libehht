# SPDX-License-Identifier: LGPL-3.0-or-later
# Makefile.am: build instructions for a simple OO hashtable
# Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org>
# https://github.com/ericherman/libehht

lib_LTLIBRARIES=libehht.la

CSTD_CFLAGS=-std=c89 -Wno-long-long
#DEBUG_CFLAGS=-ggdb -O0
DEBUG_CFLAGS=-ggdb -O2 -DNDEBUG -Wno-unused-parameter -fomit-frame-pointer
NOISY_CFLAGS=-Wall -Wextra -pedantic -Werror
#USE_JUMPHASH_CFLAGS=-DEHHT_USE_JUMPHASH=1
AM_CFLAGS=\
 $(CSTD_CFLAGS) \
 $(DEBUG_CFLAGS) \
 $(NOISY_CFLAGS) \
 $(USE_JUMPHASH_CFLAGS) \
 -I src/ -pipe

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

# https://github.com/ericherman/simple_stats
SSTATS=../simple_stats/sstats

ACLOCAL_AMFLAGS=-I m4 --install

EXTRA_DIST=COPYING COPYING.LESSER

DEMOS=$(bin_PROGRAMS)
bin_PROGRAMS=demo-ehht demo-ehht-as-array

demo_ehht_SOURCES=demos/leveldb_util_hash.c demos/djb2_hash.c \
 demos/jumphash.c demos/demo-ehht.c \
 src/ehht.h tests/ehht-report.h
demo_ehht_LDADD=libehht.la
demo_ehht_CFLAGS=$(AM_CFLAGS) -I tests/

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
 test_ehht_collision_resize \
 test_out_of_memory


tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t -T ssize_t \
		-T uint8_t -T int8_t \
		-T uint16_t -T int16_t \
		-T uint32_t -T int32_t \
		-T uint64_t -T int64_t \
		-T ehht_s \
		-T ehht_key_s \
		-T ehht_keys_s \
		`find src tests demos -name '*.h' -o -name '*.c'`

demo: $(DEMOS)
	./libtool --mode=execute ./demo-ehht-as-array
	@echo ""
	for num_buckets in 64 128 256 512 1024 2048 4096; do \
		echo ""; echo "num buckets: $$num_buckets"; \
		./libtool --mode=execute ./demo-ehht \
			$$num_buckets | $(SSTATS) --channels=5 -; \
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
	libtool --mode=execute valgrind -q ./test_ehht_collision_resize


libehht_la_SOURCES=$(include_HEADERS) src/ehht.c

include_HEADERS=src/ehht.h

TESTS=$(check_PROGRAMS)

T_COMMON_SOURCES=\
 tests/echeck.h \
 tests/echeck.c \
 tests/test-ehht.h \
 $(include_HEADERS)
T_COMMON_LDADD=libehht.la

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

test_ehht_collision_resize_SOURCES=tests/test_ehht_collision_resize.c \
 $(T_COMMON_SOURCES)
test_ehht_collision_resize_LDADD=$(T_COMMON_LDADD)

test_out_of_memory_SOURCES=tests/test_out_of_memory.c \
 $(T_COMMON_SOURCES)
test_out_of_memory_LDADD=$(T_COMMON_LDADD)
