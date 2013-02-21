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

	if ((eq_max <= 0) && (eq_min >= 0)) {
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


extern void link_slab(slab_t *, slab_t *, int);

/*
 * Searching A Slab
 *
 * These are all of the functions used for searching an individual slab.
 */


/*
 * This function finds the the index at which `elem` _should_ be located in the
 * slab.
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

/*
 * This function uses linear search to find `elem`. `is_slab` indicates whether
 * or not `elem` is just an element or if it is a pointer to a slab. Recall
 * that elems in subslabs are pointers to superslabs.
 */
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

/*
 * Binary search for `elem` in slab `s`.
 */
static int
slab_bin_srch(uintptr_t elem, slab_t *s)
{
	int min = 0;
	int max = s->s_elems - 1;
	int c = 0;
	slablist_t *sl = s->s_list;
	uint64_t lyr = sl->sl_layer;
	while (max >= min) {
		int mid = (min + max) >> 1;
		uintptr_t mid_elem = s->s_arr[mid];
		if (lyr) {
			SLABLIST_SUBSLAB_BIN_SRCH(s, (slab_t *)mid_elem);
		} else {
			SLABLIST_SLAB_BIN_SRCH(s, mid_elem);
		}
		c = sl->sl_cmp_elem(elem, mid_elem);
		if (c > 0) {
			min = mid + 1;
			continue;
		}
		if (c < 0) {
			max = mid - 1;
			continue;
		}
		if (c == 0) {
			return (mid);
		}
	}

	if (min >= s->s_elems) {
		/*
		 * Because `min` is the insertion point, if `min` is greater
		 * than the largest possible insertion point, we have to
		 * decrease `min` back to the largest possible insertion point.
		 */
		min = s->s_elems - 1;
	}

	if (sl->sl_cmp_elem(elem, s->s_arr[min]) > 0) {
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



/*
 * We do a binary search for `elem` in a slab `s`. `is_slab` indicates that
 * `elem` is a pointer to a slab (all elems in subslabs are pointers to
 * superslabs).
 */
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
	i = slab_bin_srch(srch_for, s);
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
	slablist_t *sl = s->s_list;
	int i;

	if (!(sl->sl_layer == 0 && s->s_elems < sl->sl_brk)) {
		i = gen_bin_srch(elem, s, is_slab);
		return (i);
	}
	i = gen_lin_srch(elem, s, is_slab);
	return (i);
}

/*
 *  This function tries to find the slab that contains elem in slab s.
 *  `crumbs` points to the ptr, that will hold either the bread crumb with the
 *  slab with `elem`, or the slab nearest to it.
 */
int
find_slab_in_slab(slab_t *s, uintptr_t elem, bc_t *crumbs)
{
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

	if (!x) {
		(*crumbs).bc_on_edge = ON_LEFT_EDGE;
	} else if (x == s->s_elems - 1) {
		(*crumbs).bc_on_edge = ON_RIGHT_EDGE;
	}

	int r = is_elem_in_range(elem, (slab_t *)(s->s_arr[x]));
	(*crumbs).bc_slab = (slab_t *)s->s_arr[x];

	return (r);
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
find_linear_scan(slablist_t *sl, uintptr_t elem, slab_t **sbptr)
{

	SLABLIST_LINEAR_SCAN_BEGIN(sl);
	uint64_t i = 0;
	slab_t *s = sl->sl_head;
	int r = is_elem_in_range(elem, s);

	/*
	 * Logically, this conditional is redundant, and can be removed.
	 * However, having it here makes average insertion performance
	 * empirically faster.
	 */
	if (r != FS_OVER_RANGE) {
		*sbptr = s;
		SLABLIST_LINEAR_SCAN_END(r);
		return (r);
	}

	while (i < sl->sl_slabs) {
		SLABLIST_LINEAR_SCAN(sl, s);
		int r = is_elem_in_range(elem, s);
		if (r != FS_OVER_RANGE) {
			*sbptr = s;
			SLABLIST_LINEAR_SCAN_END(r);
			return (r);
		} else {
			if (s->s_next != NULL) {
				s = s->s_next;
			} else {
				*sbptr = s;
				SLABLIST_LINEAR_SCAN_END(r);
				return (r);
			}
		}
		i++;
	}
}

/*
 * Finds the slab into which `elem` could fit, by using the base-layer as a
 * starting point. Records all subslabs that were walked over into the `crumbs`.
 */
int
find_bubble_up(slablist_t *sl, uintptr_t elem, bc_t *crumbs)
{
	SLABLIST_BUBBLE_UP_BEGIN(sl);
	/* `crumbs` is used as the bread crumb array */
	slablist_t *u = sl->sl_baselayer;
	int nu = sl->sl_sublayers;
	int cu = 0;
	slab_t *s = (crumbs[0].bc_slab);
	int fs = find_linear_scan(u, elem, &s);
	crumbs[0].bc_slab = s;
	int r = is_elem_in_range(elem, s) ;
	int bc = 0;
	bc++;
	while (cu < nu) {
		fs = find_slab_in_slab(crumbs[(bc-1)].bc_slab, elem,
			&(crumbs[bc]));
		SLABLIST_BUBBLE_UP(sl, crumbs[bc].bc_slab);
		bc++;
		cu++;
	}
	SLABLIST_BUBBLE_UP_END(fs);
	return (fs);
}

/*
 * Function tries to find `key` in `sl`, and records the found elem into the
 * user-supplied backpointer `found`.
 */
int
slablist_find(slablist_t *sl, uintptr_t key, uintptr_t *found)
{
	SLABLIST_FIND_BEGIN(sl, key);
	bc_t bc_path[MAX_LYRS];
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
			find_bubble_up(sl, key, bc_path);
			s = bc_path[(sl->sl_sublayers)].bc_slab;
		} else {
			find_linear_scan(sl, key, &s);
		}

		i = slab_srch(key, s, 0);
		ret = s->s_arr[i];

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

/*
 * Gets random element from `sl`. Unfortunately this function does not have a
 * uniform distribution of returned positions, as a result of being written in
 * a rush.
 */
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

/*
 * Gets random position from `sl`. Unfortunately this function does not have a
 * uniform distribution of returned positions, as a result of being written in
 * a rush.
 */
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
