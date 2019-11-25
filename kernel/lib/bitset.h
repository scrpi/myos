#ifndef __KERNEL_LIB_BITSET_H_
#define __KERNEL_LIB_BITSET_H_

#include <stdint.h>

struct bitset {
	uint32_t *set;
	uint32_t size;
	uint32_t bitlen;
	uint32_t next_free_idx;
};

#define INDEX_FROM_BIT(bit) ((bit) / (8 * 4))
#define OFFSET_FROM_BIT(bit) ((bit) % (8 * 4))

#define BITSET_INIT(set, bitlen) { set, sizeof(set) / 4, bitlen, 0 }

#define BITSET_NO_FREE  ((uint32_t)-1)

static inline uint32_t _bs_free_from_idx(struct bitset *bitset, uint32_t idx_start)
{
	uint32_t idx = idx_start;
	int i;

	while (bitset->set[idx] == 0xFFFFFFFF) {
		++idx;

		if (idx > bitset->bitlen / 32)
			idx = 0;

		if (idx == bitset->bitlen / 32 && bitset->bitlen % 32 != 0)
			/* Lazy mask unused bits */
			bitset->set[idx] |= (0xFFFFFFFF << (bitset->bitlen % 32));

		if (idx == idx_start)
			break;
	}

	if (idx == idx_start && bitset->set[idx] == 0xFFFFFFFF)
		goto out_no_free;

	for (i = 0; i < 32; ++i) {
		if (!(bitset->set[idx] & (0x1 << i)))
			return idx * 32 + i;
	}

out_no_free:
	return BITSET_NO_FREE;
}

static inline uint32_t bs_next_free(struct bitset *bitset)
{
	return _bs_free_from_idx(bitset, bitset->next_free_idx);
}

static inline uint32_t bs_first_free(struct bitset *bitset)
{
	return _bs_free_from_idx(bitset, 0);
}

static inline void bs_set(struct bitset *bitset, uint32_t bit)
{
	uint32_t idx = INDEX_FROM_BIT(bit);

	if (bit >= bitset->bitlen)
		panic("%s: bit idx %d >= max bits (%d)\n", __func__, bit, bitset->bitlen);

	bitset->set[idx] |= (0x1 << OFFSET_FROM_BIT(bit));

	if (bitset->next_free_idx == idx && bitset->set[idx] == 0xFFFFFFFF) {
		uint32_t bit = bs_next_free(bitset);

		if (bit != BITSET_NO_FREE)
			bitset->next_free_idx = bit / 32;
	}
}

static inline void bs_clear(struct bitset *bitset, uint32_t bit)
{
	uint32_t idx = INDEX_FROM_BIT(bit);

	if (bit >= bitset->bitlen)
		panic("%s: bit idx %d >= bitset len (%d)\n", __func__, bit, (bitset->bitlen) - 1);

	bitset->set[idx] &= ~(0x1 << OFFSET_FROM_BIT(bit));

	if (bitset->next_free_idx > idx)
		bitset->next_free_idx = idx;
}

static inline int bs_test(struct bitset *bitset, uint32_t bit)
{
	int ret;

	if (bit >= bitset->bitlen)
		panic("%s: bit idx %d >= bitset len (%d)\n", __func__, bit, (bitset->bitlen) - 1);

	ret = !!(bitset->set[INDEX_FROM_BIT(bit)] & (0x1 << OFFSET_FROM_BIT(bit)));

	return ret;
}

#endif
