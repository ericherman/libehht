#ifndef EHHT_REPORT_H
#define EHHT_REPORT_H
/** reports table's key.hashcode values distributed over sizes_len buckets
 * returns total items across all buckets  */
size_t ehht_distribution_report(struct ehht_s *table, size_t *sizes,
				size_t sizes_len)
{
	struct ehht_keys_s *keys;
	int copy_keys;
	size_t i, bucket;

	for (i = 0; i < sizes_len; ++i) {
		sizes[i] = 0;
	}

	copy_keys = 0;
	keys = table->keys(table, copy_keys);
	for (i = 0; i < keys->len; ++i) {
		bucket = keys->keys[i].hashcode % sizes_len;
		++sizes[bucket];
	}
	table->free_keys(table, keys);

	return i;
}
#endif /* EHHT_REPORT_H */
