LIB_NAME=ehht

AUX_INCLUDES=-I ../libecheck
AUX_A_FILES=../libecheck/libecheck.a
AUX_LDFLAGS=-L../libecheck -lecheck
AUX_LD_LIBRARY_PATHS=:../libecheck


UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
SHAREDFLAGS = -dynamiclib
SHAREDEXT = dylib
else
SHAREDFLAGS = -shared
SHAREDEXT = so
endif

LIB_SRC=$(LIB_NAME).c
LIB_HDR=$(LIB_NAME).h
LIB_OBJ=$(LIB_SRC:.c=.o)
SO_OBJS=$(LIB_NAME).o
SO_NAME=lib$(LIB_NAME).$(SHAREDEXT)
ifneq ($(UNAME), Darwin)
    SHAREDFLAGS += -Wl,-soname,$(SO_NAME)
endif

A_NAME=lib$(LIB_NAME).a

INCLUDES=-I.
TEST_SRC=test-$(LIB_NAME).c
TEST_OBJ=test-$(LIB_NAME).o
TEST=test-$(LIB_NAME)

CSTD_CFLAGS=-std=c89
#DEBUG_CFLAGS=-ggdb -O0
DEBUG_CFLAGS=-O3 -DNDEBUG -Wno-unused-parameter
NOISY_CFLAGS=-Wall -Wextra -pedantic -Werror

CFLAGS += $(CSTD_CFLAGS) $(DEBUG_CFLAGS) $(NOISY_CFLAGS)
LDFLAGS += -L. -l$(LIB_NAME)
CC=gcc

ifeq ("$(PREFIX)", "")
PREFIX=/usr/local
endif

ifeq ("$(LIBDIR)", "")
LIBDIR=$(PREFIX)/lib
endif

ifeq ("$(INCDIR)", "")
INCDIR=$(PREFIX)/include
endif

LD_LIBRARY_PATH=.$(AUX_LD_LIBRARY_PATHS)

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

# https://github.com/ericherman/simple_stats
SSTATS=../simple_stats/sstats

default: library

.c.o:
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

$(SO_NAME): $(LIB_OBJ)
	$(CC) $(SHAREDFLAGS) -o $(SO_NAME).1.0 $(SO_OBJS)
	ln -sf ./$(SO_NAME).1.0 ./$(SO_NAME).1
	ln -sf ./$(SO_NAME).1.0 ./$(SO_NAME)

$(A_NAME): $(LIB_OBJ)
	ar -r $(A_NAME) $(SO_OBJS)

library: $(SO_NAME) $(A_NAME)

$(TEST): library
	$(CC) -c $(INCLUDES) $(AUX_INCLUDES) $(CFLAGS) \
		$(TEST_SRC) -o $(TEST_OBJ)
	$(CC) $(TEST_OBJ) $(A_NAME) $(AUX_A_FILES) -o $(TEST)-static
	$(CC) $(TEST_OBJ) $(LDFLAGS) $(AUX_LDFLAGS) -o $(TEST)-dynamic

check: $(TEST)
	./$(TEST)-static
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) ./$(TEST)-dynamic

valgrind: $(TEST)
	valgrind ./$(TEST)-static

tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t \
		*.h *.c

demo: library
	$(CC) -c $(INCLUDES) $(CFLAGS) \
		-o leveldb_util_hash.o leveldb_util_hash.c
	$(CC) -c $(INCLUDES) $(CFLAGS) -o demo-ehht.o demo-ehht.c
	$(CC) leveldb_util_hash.o demo-ehht.o $(A_NAME) -o demo-ehht
	for num_buckets in 64 128 256 512 1024 2048 4096; do \
		echo ""; echo "num buckets: $$num_buckets"; \
		./demo-ehht $$num_buckets | $(SSTATS) --channels=2 -; \
	done

clean:
	rm -f *.o *.a *.$(SHAREDEXT) \
		$(SO_NAME).* \
		$(TEST)-static $(TEST)-dynamic demo-ehht

install: library
	@echo "Installing libraries in $(LIBDIR)"
	mkdir -pv $(LIBDIR)
	cp -pv $(A_NAME) $(LIBDIR)/
	cp -Rv $(SO_NAME)* $(LIBDIR)/
	@echo "Installing headers in $(INCDIR)"
	mkdir -pv $(INCDIR)
	cp -pv $(LIB_HDR) $(INCDIR)/
