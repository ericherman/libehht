/* SPDX-License-Identifier: LGPL-2.1-or-later */
/* jumphash: A Fast, Minimal Memory, Consistent Hash Algorithm (from: Google) */
/* Copyright (C) 2018, 2019 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */
/* https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt */

/*
Much of this code was from 2014 by John Lamping, Eric Veach of Google
https://arxiv.org/pdf/1406.2294v1.pdf
  Google has not applied for patent protection for this algorithm, and,
  as of this writing, has no plans to. Rather, it wishes to contribute
  this algorithm to the community.
*/

#include <stdint.h>		/* int32_t int64_t uint64_t */

/* see also: http://random.mat.sbg.ac.at/results/karl/server/node5.html */
#define Linear_Congruential_Generator_64 2862933555777941757ULL

/* essentially a copy-paste from  https://arxiv.org/pdf/1406.2294v1.pdf */
int32_t jumphash(uint64_t key, int32_t num_buckets)
{
	int64_t b = -1;
	int64_t j = 0;

	while (j < num_buckets) {
		b = j;
		key = key * Linear_Congruential_Generator_64 + 1;
		j = (b + 1) * ((double)(1LL << 31) / (double)((key >> 33) + 1));
	}
	return ((int32_t)b);
}
