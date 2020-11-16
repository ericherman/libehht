/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehht-report.h: reporting interface for a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#ifndef EHHT_REPORT_H
#define EHHT_REPORT_H

#ifdef __cplusplus
#define Ehht_report_begin_C_functions extern "C" {
#define Ehht_report_end_C_functions }
#else
#define Ehht_report_begin_C_functions
#define Ehht_report_end_C_functions
#endif

#include "ehht.h"

Ehht_report_begin_C_functions
#undef Ehht_report_begin_C_functions
/** reports table's key.hashcode values distributed over sizes_len buckets
 * returns total items across all buckets  */
size_t ehht_distribution_report(struct ehht *table, size_t *sizes,
				size_t sizes_len);

Ehht_report_end_C_functions
#undef Ehht_report_end_C_functions
#endif /* EHHT_REPORT_H */
