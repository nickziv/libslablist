/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/LIBSLABLIST.LICENSE
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/LIBSLABLIST.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2012 Nicholas Zivkovic. All rights reserved.
 * Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "slablist_impl.h"
#include "slablist_provider.h"


/*
 * Gets the slab that is either the pos'th slab from the left, or gets the slab
 * that contains the pos'th element. The flag specifies which behaviour, of the
 * two to take. When we get the slab that contains the pos'th elem, we fill
 * off_pos with the total number of elements in the slabs that precede the
 * returned slab. This way the caller can figure out the position of pos'th
 * element in the slab.
 */
slab_t *
slab_get(slablist_t *sl, uint64_t pos, uint64_t *off_pos, int flag)
{
	slab_t *slab = sl->sl_head;
	uint64_t i;
	uint64_t ecnt = 0;
	uint64_t mod;

	if (slab == NULL) {
		return (NULL);
	}

	if (sl->sl_is_small_list) {
		/*
		 * `slab` doesn't point to a slab, it points to linked list
		 * node, which we won't return (as that would be wrong).
		 */
		return (NULL);
	}

	if (flag == SLAB_VAL_POS) {
		/* get the slab that contains the value at this position */
		uint64_t e = sl->sl_elems;

		if (e < pos && !SLIST_IS_CIRCULAR(sl->sl_flags)) {
			/*
			 * If the number of elements is less than the position
			 * we want and the list is not circular, were return
			 * NULL.
			 */
			return (NULL);
		}

		if (e < pos) {
			/*
			 * If on the other hand, the number of elements is less
			 * than the desired position, but the list _is_
			 * circular, we take the mod of desired position and
			 * the number of elements. Otherwise, mod is equal to
			 * the position.
			 */
			mod = pos % e;
		} else {
			mod = pos;
		}

		i = 0;
		while (i < sl->sl_slabs) {
			/*
			 * We keep updating `ecnt` with the number of elements
			 * in all of the slabs that we have visited. As soon as
			 * `ecnt` exceeds `mod`, which is the position that our
			 * desired element is in, we return the slab that we
			 * are currently at, and set `off_pos` to the value of
			 * `ecnt` _before_ we visited the current slab.
			 */
			ecnt += slab->s_elems;

			if (ecnt >= mod) {
				*off_pos = ecnt - slab->s_elems;
				return (slab);
			}

			slab = slab->s_next;
			i++;
		}
	}

	if (flag == SLAB_IN_POS) {
		/* get the slab that is of this number */

		if (pos == 0) {
			return (sl->sl_head);
		}

		if (sl->sl_slabs < pos && !SLIST_IS_CIRCULAR(sl->sl_flags)) {
			/* a slab doesn't exist at this position */
			return (NULL);
		}

		mod = pos;

		if (sl->sl_slabs < pos) {
			mod = pos % sl->sl_slabs;
		}

		for (i = 0; i < mod; i++) {
			slab = slab->s_next;
		}

		return (slab);
	}

	return (NULL);
}

static small_list_t *
sml_node_get(slablist_t *sl, uint64_t pos)
{
	uint64_t mod;
	if (sl->sl_elems == 0) {
		return (NULL);
	}

	if (!(sl->sl_is_small_list)) {
		return (NULL);
	}

	if (pos > sl->sl_elems && !SLIST_IS_CIRCULAR(sl->sl_flags)) {
		return (NULL);
	}

	mod = pos;


	if (sl->sl_elems < pos) {
		mod = pos % sl->sl_elems;
	}

	small_list_t *s = sl->sl_head;

	uint64_t i;
	for (i = 0; i < mod; i++) {
		s = s->sml_next;
	}

	return (s);
}

/*
 * NOTE: the lowest possible value for `pos` is 0
 */
uintptr_t
slablist_get(slablist_t *sl, uint64_t pos)
{
	uintptr_t ret;
	int i;
	uint64_t off_pos;
	slab_t *s;
	small_list_t *sml = NULL;
	if (sl->sl_is_small_list) {
		sml = sml_node_get(sl, pos);
		ret = sml->sml_data;
	} else {
		s = slab_get(sl, pos, &off_pos, SLAB_VAL_POS);
		i = pos - off_pos;
		ret = s->s_arr[i];
	}

	return (ret);
}

int
is_elem_in_range(uintptr_t elem, slab_t *s)
{
	int eq_min;
	int eq_max;
	slablist_t *sl = s->s_list;
	uintptr_t max = s->s_max;
	uintptr_t min = s->s_min;
	if (sl->sl_layer) {
		/*
		 * If we are checking to see if an element is in the range of a
		 * _subslab_, we have to use the comparison function saved in
		 * the superlayer (defined by user). Using the comparison
		 * function in the current layer will result in using
		 * is_elem_in_range as a comparison function, which will cause
		 * a crash.
		 */
		eq_min = (sl->sl_cmp_super(elem, min));
		eq_max = (sl->sl_cmp_super(elem, max));
	} else {
		eq_min = (sl->sl_cmp_elem(elem, min));
		eq_max = (sl->sl_cmp_elem(elem, max));
	}

	if ((eq_max == 0 || eq_max == -1) && (eq_min == 1 || eq_min == 0)) {
		return (FS_IN_RANGE);
	}

	if (eq_max == 1) {
		return (FS_OVER_RANGE);
	}

	if (eq_min == -1) {
		return (FS_UNDER_RANGE);
	}

	return (404);

}
/*
 * This function determines if the element can fit in a slabs existing range.
 */
int
old_is_elem_in_range(uintptr_t elem, slab_t *s)
{
	int eq_min;
	int eq_max;
	slablist_t *sl = s->s_list;
	if (sl->sl_layer) {
		/*
		 * If we are checking to see if an element is in the range of a
		 * _subslab_, we have to use the comparison function saved in
		 * the superlayer (defined by user). Using the comparison
		 * function in the current layer will result in using
		 * is_elem_in_range as a comparison function, which will cause
		 * a crash.
		 */
		eq_min = (sl->sl_cmp_super(elem, s->s_min));
		eq_max = (sl->sl_cmp_super(elem, s->s_max));
	} else {
		eq_min = (sl->sl_cmp_elem(elem, s->s_min));
		eq_max = (sl->sl_cmp_elem(elem, s->s_max));
	}

	if ((eq_max == 0 || eq_max == -1) && (eq_min == 1 || eq_min == 0)) {
		return (FS_IN_RANGE);
	}

	if (eq_max == 1) {
		return (FS_OVER_RANGE);
	}

	if (eq_min == -1) {
		return (FS_UNDER_RANGE);
	}

	return (404);

}

/*
 * This function determines if the element can replace a slabs current extrema.
 */
int
is_elem_extrema(uintptr_t elem, slab_t *s)
{
	slablist_t *sl = s->s_list;
	int eq_min = (sl->sl_cmp_elem(elem, s->s_min));
	int eq_max = (sl->sl_cmp_elem(elem, s->s_max));

	if (eq_min == -1) {
		return (-1);
	}

	if (eq_max == 1) {
		return (1);
	}

	return (0);
}

extern void link_slab(slab_t *, slab_t *, int);

/*
 * Searching A Slab
 *
 * These are all of the functions used for searching an individual slab.
 */


static int
slab_linear_srch(uintptr_t elem, slab_t *s, int start)
{
	int i = start;
	slablist_t *sl = s->s_list;
	while (i < (s->s_elems) && sl->sl_cmp_elem(elem, s->s_arr[i]) > 0) {
		i++;
	}
	return (i);
}

static int
sublayer_slab_linear_srch(uintptr_t elem, slab_t *s, int start)
{
	int i = start;
	slab_t *ei = (slab_t *)elem;
	slablist_t *sl = s->s_list;

	if (SLABLIST_TEST_SLAB_SUBLAYER_ENABLED()) {
		SLABLIST_TEST_SLAB_SUBLAYER(!(SLIST_SUBLAYER(sl->sl_flags)));
	}

	while (i < s->s_elems && sl->sl_cmp_elem(ei->s_min, s->s_arr[i]) > 0) {
		i++;
	}
	return (i);
}

static int
gen_lin_srch(uintptr_t elem, slab_t *s, int is_slab)
{
	int i = 0;
	slablist_t *sl = s->s_list;
	int sublayer = SLIST_SUBLAYER(sl->sl_flags);
	uintptr_t srch_for = elem;

	SLABLIST_FIND_SLAB_POS_BEGIN(sublayer);
	if (sublayer && is_slab) {
		slab_t *e2s = (slab_t *)elem;
		srch_for = e2s->s_min;
	}
	if (s->s_elems > 0) {
		i = slab_linear_srch(srch_for, s, 0);
	} else {
		i = 0;
	}
	SLABLIST_FIND_SLAB_POS_END(i);
	return (i);
}

/*
 * If we need to find a slab-ptr in an sublayer who's data has been freed,
 * we can not use and of the other srch functions because they dereference
 * members of the slab-ptr (which is fine in all contexts, except when that
 * slab-ptrs underlying data may have been freed). So we do a simple
 * comparison against all elems until we get one that matches. If nothing
 * matches, we return -1. Otherwise we return the index.
 */
int
sublayer_slab_ptr_srch(uintptr_t elem, slab_t *s, int start)
{
	int i = start;
	while (i < (s->s_elems)) {
		if (s->s_arr[i] == elem) {
			return (i);
		}
		i++;
	}
	return (-1);
}

static int
slab_bin_srch(uintptr_t elem, slab_t *s)
{
	int min = 0;
	int max = s->s_elems - 1;
	int c = 0;
	slablist_t *sl = s->s_list;
	uint64_t lyr = sl->sl_layer;
	while (max >= min) {
		int mid = (min + max)/2;
		uintptr_t mid_elem = s->s_arr[mid];
		if (lyr) {
			SLABLIST_SUBSLAB_BIN_SRCH(s, (slab_t *)mid_elem);
		} else {
			SLABLIST_SLAB_BIN_SRCH(s, mid_elem);
		}
		c = sl->sl_cmp_elem(elem, mid_elem);
		if (c == 1) {
			min = mid + 1;
			continue;
		}
		if (c == -1) {
			max = mid - 1;
			continue;
		}
		if (c == 0) {
			return (mid);
		}
		if (c < -1 || c > 1) {
			fprintf(stderr,
				"libslablist: cmp func invalid return value\n");
			abort();
		}
	}

	if (min >= s->s_elems) {
		/*
		 * End of array.
		 */
		min = s->s_elems - 1;
	}

	if (sl->sl_cmp_elem(elem, s->s_arr[min]) == 1) {
		/*
		 * If the binary search took us to an element that is smaller
		 * than `elem`, we return the index of the next element, which
		 * is likely to be larger. This is because all of our code
		 * insertion code assumes that we return the index that we
		 * want to insert `elem` _at_.
		 */
		return (min + 1);
	}

	/*
	 * Just in case.
	 */
	return (min);
}



static int
gen_bin_srch(uintptr_t elem, slab_t *s, int is_slab)
{
	int i = 0;
	slablist_t *sl = s->s_list;
	uintptr_t srch_for = elem;
	int sublayer = SLIST_SUBLAYER(sl->sl_flags);

	if (sublayer && is_slab) {
		slab_t *e2s = (slab_t *)elem;
		srch_for = e2s->s_min;
	}

	SLABLIST_FIND_SLAB_POS_BEGIN(sublayer);
	if (s->s_elems > 0) {
		i = slab_bin_srch(srch_for, s);
	} else {
		i = 0;
	}
	SLABLIST_FIND_SLAB_POS_END(i);

	return (i);
}

/*
 * This function is a generic search function. It uses the `brk` property of a
 * slablist to determine when to use binary search instead of linear search.
 */
int
slab_srch(uintptr_t elem, slab_t *s, int is_slab)
{
	if (s->s_elems == 0) {
		return (0);
	}

	slablist_t *sl = s->s_list;
	int i;

	if (sl->sl_layer == 0 && s->s_elems < sl->sl_brk) {
		i = gen_lin_srch(elem, s, is_slab);
		return (i);
	}
	i = gen_bin_srch(elem, s, is_slab);
	return (i);
}

int
slab_stride_srch(uintptr_t elem, slab_t *s)
{
	int sd[8];
	sd[0] = 63;
	sd[1] = 127;
	sd[2] = 191;
	sd[3] = 255;
	sd[4] = 319;
	sd[5] = 383;
	sd[6] = 454;
	sd[7] = 511;

	int i = 0;
	slablist_t *sl = s->s_list;
	while (i < 8) {
		if (sl->sl_cmp_elem(elem, s->s_arr[sd[i]]) == -1) {
			/*
			 * Clearly, elem is too small to be in this stride, so
			 * we revert to an older stride (or zero) and do a
			 * linear srch from there.
			 */
			if (i) {
				i = slab_linear_srch(elem, s, sd[(i - 1)]);
			} else {
				i = slab_linear_srch(elem, s, 0);
			}
			return (i);
		}
		if (sl->sl_cmp_elem(elem, s->s_arr[sd[i]]) == 0) {
			return (sd[i]);
		}
		i++;
	}

	return (404);
}

int
sublayer_slab_stride_srch(uintptr_t elem, slab_t *s)
{
	int sd[8];
	sd[0] = 63;
	sd[1] = 127;
	sd[2] = 191;
	sd[3] = 255;
	sd[4] = 319;
	sd[5] = 383;
	sd[6] = 454;
	sd[7] = 511;

	int i = 0;
	slablist_t *sl = s->s_list;
	slab_t *ei = (slab_t *)elem;

	if (SLABLIST_TEST_SLAB_SUBLAYER_ENABLED()) {
		SLABLIST_TEST_SLAB_SUBLAYER(!(SLIST_SUBLAYER(sl->sl_flags)));
	}

	while (i < 8) {
		slab_t *e = (slab_t *)s->s_arr[sd[i]];
		if (sl->sl_cmp_elem(ei->s_min, e->s_min) == -1) {
			/*
			 * Clearly, elem is too small to be in this stride, so
			 * we revert to an older stride (or zero) and do a
			 * linear srch from there.
			 */
			if (i) {
				i = sublayer_slab_linear_srch(elem, s,
					sd[(i - 1)]);
			} else {
				i = sublayer_slab_linear_srch(elem, s, 0);
			}
			return (i);
		}
		if (sl->sl_cmp_elem(ei->s_min, e->s_min) == 0) {
			return (sd[i]);
		}
		i++;
	}

	return (404);
}


/*
 *  This function tries to find the slab that contains elem in slab s.  `l`
 *  points to the ptr, that will hold either the slab with `elem`, or the
 *  slab nearest to it.
 */
static int
find_slab_in_slab(slab_t *s, uintptr_t elem, slab_t **l)
{
	slab_t *sb = (slab_t *)s->s_arr[0];
	int r = is_elem_in_range(elem, sb);

	if (r != FS_OVER_RANGE) {
		*l = sb;
	}

	if (r == FS_OVER_RANGE) {
		int x = 0;
		x = slab_srch(elem, s, 0);
		if (x > s->s_elems - 1) {
			/*
			 * If we get an index `x` that is larger than the index
			 * of the last element, we set x to the index of the
			 * last element. This is neccessary, since
			 * slab_bin_srch always assumes that we are searching
			 * for an _insertion point_. That is, we want the index
			 * at which a new element will be inserted. In the
			 * context of this function, we really want the slab
			 * who's range is closest to that of `elem`. Hence why
			 * if we exceed the maximum index, we know that the
			 * last slab is the one we are looking for. Note that
			 * this is not a problem for an `elem` that is below
			 * the slab's range, as the insertion point (0) and the
			 * slab-index we are looking for are the same.
			 */
			x = s->s_elems - 1;
		}
		r = is_elem_in_range(elem, (slab_t *)(s->s_arr[x]));
		*l = (slab_t *)s->s_arr[x];
	}

	return (r);
}



slablist_t *
get_lowest_sublayer(slablist_t *sl)
{
	int l = sl->sl_sublayers;

	slablist_t *cur_layer = sl;
	int i = 0;
	while (i < l) {
		cur_layer = cur_layer->sl_sublayer;
		i++;
	}
	return (cur_layer);
}

/*
 * TODO
 * Replace this with min cmp version.
 * If we are > min we keep going right.
 * If we are < min, we check the previous slab's max.
 * We either fit in prev slab (inrange), or can go into either slab.
 * If we are at end and > min, we append.
 */
int
find_linear_scan(slablist_t *sl, uintptr_t elem, slab_t **l)
{


	uint64_t i = 0;
	slab_t *s = sl->sl_head;
	int r = is_elem_in_range(elem, s);
	if (r != FS_OVER_RANGE) {
		*l = s;
		return (r);
	}

	while (i < sl->sl_slabs) {
		SLABLIST_LINEAR_SCAN(sl, s);
		int r = is_elem_in_range(elem, s);
		if (r != FS_OVER_RANGE) {
			*l = s;
			return (r);
		} else {
			if (s->s_next != NULL) {
				s = s->s_next;
			} else {
				*l = s;
				return (r);
			}
		}
		i++;
	}

	/*
	 * Easily recognizable error code. We should never get here. But if we
	 * do, it will stand out in the DTrace output.
	 */
	return (404);
}

int
find_bubble_up(slablist_t *sl, uintptr_t elem, slab_t *l[])
{
	/* `l` is used as the bread crumb array */
	slablist_t *u = get_lowest_sublayer(sl);
	int nu = sl->sl_sublayers;
	int cu = 0;
	int fs = find_linear_scan(u, elem, l);
	int bc = 0;
	bc++;
	while (cu < nu) {
		fs = find_slab_in_slab(l[(bc-1)], elem, &(l[bc]));
		SLABLIST_BUBBLE_UP(sl, l[bc]);
		bc++;
		cu++;
	}
	return (fs);
}

int
slablist_find(slablist_t *sl, uintptr_t key, uintptr_t *found)
{
	SLABLIST_FIND_BEGIN(sl, key);
	slab_t **bc;
	slab_t *s;
	int i = 0;
	uintptr_t ret;
	if (sl->sl_is_small_list) {
		small_list_t *sml = sl->sl_head;
		while (i < sl->sl_elems &&
		    sl->sl_cmp_elem(key, sml->sml_data) != 0) {
			sml = sml->sml_next;
			i++;
		}
		*found = sml->sml_data;
		SLABLIST_FIND_END(SL_SUCCESS, *found);
		return (SL_SUCCESS);
	}

	if (SLIST_SORTED(sl->sl_flags)) {

		if (sl->sl_sublayers) {
			bc = mk_buf(8 * 256);
			find_bubble_up(sl, key, bc);
			s = bc[(sl->sl_sublayers)];
		} else {
			find_linear_scan(sl, key, &s);
		}

		i = slab_srch(key, s, 0);
		ret = s->s_arr[i];

		if (sl->sl_sublayers) {
			rm_buf(bc, 8 * 256);
		}
		*found  = ret;
		if (sl->sl_cmp_elem(key, ret) == 0) {
			SLABLIST_FIND_END(SL_SUCCESS, *found);
			return (SL_SUCCESS);
		} else {
			SLABLIST_FIND_END(SL_ENFOUND, *found);
			return (SL_ENFOUND);
		}
	} else {
		SLABLIST_FIND_END(SL_ARGORD, *found);
		return (SL_ARGORD);
	}
}

static uint64_t
get_rand_num(int rfd)
{
	uint64_t rv;
	char *rvp = (char *)&rv;
	size_t sz = sizeof (rv);
	int red = 0;
	int toread = sizeof (rv);
	int i = 0;
	while (i < toread) {
		red = read(rfd, rvp, sz);
		i += red;
		sz -= red;
		rvp += red;
	}

	return (rv);
}

uintptr_t
slablist_get_rand(slablist_t *sl)
{
	char *rfn = "/dev/urandom";
	int rfd = open(rfn, O_RDONLY);
	uint64_t r = get_rand_num(rfd);

	uint64_t op = 0;

	uintptr_t ret;

	if (sl->sl_elems == 0) {
		return (0);
	}
	r = r % sl->sl_elems;


	small_list_t *sml;
	slab_t *s;


	if (sl->sl_is_small_list) {
		sml = sml_node_get(sl, r);
		ret = sml->sml_data;
	} else {
		s = slab_get(sl, r, &op, SLAB_VAL_POS);
		ret = s->s_arr[(r - op - 1)];
	}

	close(rfd);
	SLABLIST_GET_RAND(sl, ret);
	return (ret);
}

uint64_t
slablist_get_rand_pos(slablist_t *sl)
{
	if (sl->sl_is_small_list) {
		return (NULL);
	}
	char *rfn = "/dev/urandom";
	int rfd = open(rfn, O_RDONLY);
	uint64_t r = get_rand_num(rfd);
	r = r % sl->sl_elems;
	close(rfd);
	return (r);
}
