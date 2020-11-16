# SPDX-License-Identifier: LGPL-3.0-or-later
# Makefile.am: build instructions for a simple OO hashtable
# Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org>
# https://github.com/ericherman/libehht

SHELL=/bin/bash
BROWSER=firefox

lib_LTLIBRARIES=libehht.la

CSTD_CFLAGS=-std=c89 -Wno-long-long
NOISY_CFLAGS=-Wall -Wextra -pedantic -Werror

if DEBUG
# ASAN_OPTIONS='allocator_may_return_null=1'
BUILD_CFLAGS=-ggdb -O0 \
	-fno-inline-small-functions \
	-fkeep-inline-functions \
	-fkeep-static-functions \
	--coverage
BUILD_LDFLAGS="--coverage"
else
BUILD_CFLAGS=-ggdb -O2 -DNDEBUG -fomit-frame-pointer
BUILD_LDFLAGS=
endif

# ./configure --enable-fauxfreestanding
if FAUX_FREESTANDING
BUILD_ENV_CFLAGS=-DFAUX_FREESTANDING=1 -DEEMBED_HOSTED=0
else
BUILD_ENV_CFLAGS=
endif

AM_CFLAGS=\
 $(CSTD_CFLAGS) \
 $(BUILD_ENV_CFLAGS) \
 $(BUILD_CFLAGS) \
 $(NOISY_CFLAGS) \
 -I./src \
 -I./submodules/libecheck/src/ \
 -fPIC -I src/ -pipe

AM_LDFLAGS=$(BUILD_LDFLAGS)

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

# https://github.com/ericherman/simple_stats
SSTATS=../simple_stats/sstats

ACLOCAL_AMFLAGS=-I m4 --install

EXTRA_DIST=COPYING COPYING.LESSER \
	submodules/libecheck/COPYING \
	submodules/libecheck/COPYING.LESSER \
	submodules/libecheck/src/eembed.h \
	submodules/libecheck/src/eembed.c \
	submodules/libecheck/src/echeck.h \
	submodules/libecheck/src/echeck.c

DEMOS=$(bin_PROGRAMS)
bin_PROGRAMS=demo-ehht demo-ehht-as-array

demo_ehht_SOURCES=demos/leveldb_util_hash.c demos/djb2_hash.c \
 demos/jumphash.c demos/demo-ehht.c tests/ehht-report.c \
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
 test_flyweight \
 test_out_of_memory

line-cov: check
	lcov    --checksum \
		--capture \
		--base-directory . \
		--directory src \
		--output-file coverage.info

html-report: line-cov
	mkdir -pv ./coverage_html
	genhtml coverage.info --output-directory coverage_html

coverage: html-report
	$(BROWSER) ./coverage_html/src/index.html


tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t -T ssize_t \
		-T uint8_t -T int8_t \
		-T uint16_t -T int16_t \
		-T uint32_t -T int32_t \
		-T uint64_t -T int64_t \
		-T ehht \
		-T ehht_key \
		-T ehht_keys \
		-T ehht_log \
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
	pushd submodules/libecheck && \
		rm -rf `cat .gitignore | sed -e 's/#.*//'`; \
		popd

vg-test_ehht_new: test_ehht_new
	./libtool --mode=execute valgrind -q ./test_ehht_new

vg-test_ehht_put_get_remove: test_ehht_put_get_remove
	./libtool --mode=execute valgrind -q ./test_ehht_put_get_remove

vg-test_ehht_clear: test_ehht_clear
	./libtool --mode=execute valgrind -q ./test_ehht_clear

vg-test_ehht_foreach_element: test_ehht_foreach_element
	./libtool --mode=execute valgrind -q ./test_ehht_foreach_element

vg-test_ehht_keys: test_ehht_keys
	./libtool --mode=execute valgrind -q ./test_ehht_keys

vg-test_ehht_resize: test_ehht_resize
	ASAN_OPTIONS='allocator_may_return_null=1' \
	./libtool --mode=execute valgrind -q ./test_ehht_resize

vg-test_ehht_collision_resize: test_ehht_collision_resize
	./libtool --mode=execute valgrind -q ./test_ehht_collision_resize

vg-test_flyweight: test_flyweight
	./libtool --mode=execute valgrind -q ./test_flyweight

vg-test_out_of_memory: test_out_of_memory
	./libtool --mode=execute valgrind -q ./test_out_of_memory

valgrind: \
	vg-test_ehht_new \
	vg-test_ehht_put_get_remove \
	vg-test_ehht_clear \
	vg-test_ehht_foreach_element \
	vg-test_ehht_keys \
	vg-test_ehht_resize \
	vg-test_ehht_collision_resize \
	vg-test_flyweight \
	vg-test_out_of_memory


libehht_la_SOURCES=$(include_HEADERS) \
		submodules/libecheck/src/eembed.c \
		src/ehht.c

include_HEADERS=src/ehht.h submodules/libecheck/src/eembed.h

TESTS=$(check_PROGRAMS)

T_COMMON_SOURCES=\
 submodules/libecheck/src/echeck.h \
 submodules/libecheck/src/echeck.c \
 tests/ehht-report.h \
 tests/ehht-report.c \
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
test_ehht_resize_CFLAGS=$(AM_CFLAGS) -std=gnu89

test_ehht_collision_resize_SOURCES=tests/test_ehht_collision_resize.c \
 $(T_COMMON_SOURCES)
test_ehht_collision_resize_LDADD=$(T_COMMON_LDADD)

test_flyweight_SOURCES=tests/test_flyweight.c \
 $(T_COMMON_SOURCES)
test_flyweight_LDADD=$(T_COMMON_LDADD)

test_out_of_memory_SOURCES=tests/test_out_of_memory.c \
 $(T_COMMON_SOURCES)
test_out_of_memory_LDADD=$(T_COMMON_LDADD)
