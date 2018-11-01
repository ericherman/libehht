/*
Copyright (c) 2011 The LevelDB Authors. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
   * Neither the name of Google Inc. nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
  original C++ code here: https://github.com/google/leveldb
  in this form, converted to C
  Eric Herman 2016-02-20
*/

#include <stddef.h>		/* size_t */

/* https://github.com/google/leveldb/blob/master/util/coding.h#L58 */
unsigned int leveldb_decode_fixed_32(const char *ptr)
{
	return (((unsigned int)((unsigned char)(ptr[0])))
		| ((unsigned int)((unsigned char)(ptr[1])) << 8)
		| ((unsigned int)((unsigned char)(ptr[2])) << 16)
		| ((unsigned int)((unsigned char)(ptr[3])) << 24));
}

/*
  seed taken from hash_test.cc -Eric Herman
  https://github.com/google/leveldb/blob/master/util/hash_test.cc#L32
*/
const unsigned int leveldb_seed = 0xbc9f1d34;

/* https://github.com/google/leveldb/blob/master/util/hash.cc#L18 */
unsigned int leveldb_hash(const char *data, size_t n)
{
	/* Similar to murmur hash */
	const unsigned int m = 0xc6a4a793;
	const unsigned int r = 24;
	const char *limit = data + n;
	unsigned int h = leveldb_seed ^ (n * m);

	/* Pick up four bytes at a time */
	while (data + 4 <= limit) {
		unsigned int w = leveldb_decode_fixed_32(data);
		data += 4;
		h += w;
		h *= m;
		h ^= (h >> 16);
	}

	/* Pick up remaining bytes */
	switch (limit - data) {
	case 3:
		h += ((unsigned int)((unsigned char)(data[2]))) << 16;
		/* fallthrough */
	case 2:
		h += ((unsigned int)((unsigned char)(data[1]))) << 8;
		/* fallthrough */
	case 1:
		h += ((unsigned int)((unsigned char)(data[0])));
		h *= m;
		h ^= (h >> r);
		break;
	}
	return h;
}
