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
#include <strings.h>
#include "slablist_impl.h"
#include "slablist_provider.h"
#include "slablist_test.h"
#include "slablist_cons.h"

void marker1(){};
void marker2(){};
void marker3(){};
void marker4(){};
void marker5(){};

slab_t *
slab_get_elem_pos_old(slablist_t *sl, uint64_t pos, uint64_t *off_pos)
{
	slab_t *slab = sl->sl_head;
	uint64_t i;
	uint64_t ecnt = 0;
	uint64_t mod;
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
	return (slab);
}

/*
 * Gets the slab that is either the pos'th slab from the left, or gets the slab
 * that contains the pos'th element. The flag specifies which behaviour, of the
 * two to take. When we get the slab that contains the pos'th elem, we fill
 * off_pos with the index of the element.
 */
slab_t *
slab_get_elem_pos(slablist_t *sl, uint64_t pos, uint64_t *off_pos)
{
	slab_t *slab = sl->sl_head;
	/* get the slab that contains the value at this position */
	uint64_t act_pos;


	/*
	 * If the number of elements is less than the desired position, but the
	 * list _is_ _circular_, we take the mod of desired position and the
	 * number of elementsm to get the _actual position_. Otherwise, the
	 * actual poistion is equal to the position.
	 */
	if (sl->sl_elems < pos) {
		if (!SLIST_IS_CIRCULAR(sl->sl_flags)) {
			return (NULL);
		}
		act_pos = pos % sl->sl_elems;
	} else {
		act_pos = pos;
	}

	marker1();
	if (!(sl->sl_sublayers)) {
		uint64_t ecnt = sl->sl_elems;
		uint64_t i = 0;
		/*
		 * We keep updating `ecnt` with the number of elements
		 * in all of the slabs that we have visited. As soon as
		 * `ecnt` exceeds `act_pos`, which is the position that our
		 * desired element is in, we return the slab that we
		 * are currently at, and set `off_pos` to the value of
		 * `ecnt` _before_ we visited the current slab.
		 */
		while (i < sl->sl_slabs) {
			ecnt += slab->s_elems;

			if (ecnt >= act_pos) {
				*off_pos = ecnt - slab->s_elems;
				return (slab);
			}

			slab = slab->s_next;
			i++;
		}
	}

	marker2();
	slablist_t *bl = sl->sl_baselayer;
	subslab_t *b = bl->sl_head;
	uint16_t layers = bl->sl_layer;
	uint16_t layer = 0;
	uint64_t sum_usr_elems = 0;
	/*
	 * We visit the subslabs in the baselayer, adding up their ss_usr_elems
	 * members, until this sum exceeds the position we are looking for.
	 */
	while (b != NULL) {
		sum_usr_elems += b->ss_usr_elems;
		if (sum_usr_elems >= act_pos) {
			break;
		}
		b = b->ss_next;
	}

	/*
	 * Clearly, the slab containing the element at the desired position is
	 * reachable from subslab `b`. In `elems_skipped` we store the number
	 * of elements we had to skip before we got subslab `b`.
	 */
	uint64_t elems_skipped = sum_usr_elems - b->ss_usr_elems;

	marker3();
	/*
	 * We reset `sum_usr_elems` to `elems_skipped`, so that we can now work
	 * our way up the layers, using the same basic algorithm we used in the
	 * above loop. We don't reach the toplayer in this loop, because the
	 * toplayer is made of the slab_t struct instead of the subslab_t
	 * struct.
	 */
	sum_usr_elems = elems_skipped;
	while (layer < layers - 1) {
		subslab_t *c = GET_SUBSLAB_ELEM(b, 0);
		while (c != NULL) {
			sum_usr_elems += c->ss_usr_elems;
			if (sum_usr_elems >= act_pos) {
				break;
			}
			c = c->ss_next;
		}
		b = c;
		elems_skipped = sum_usr_elems - b->ss_usr_elems;
		layer++;
	}

	marker4();
	/*
	 * We have reached the toplayer, and we now employ the same basic
	 * algorithm as in the above inner-loop but for slab_t's instead.
	 */
	slab_t *s = GET_SUBSLAB_ELEM(b, 0);
	sum_usr_elems = elems_skipped;
	while (s != NULL) {
		sum_usr_elems += s->s_elems;
		if (sum_usr_elems >= act_pos) {
			break;
		}
		s = s->s_next;
	}

	elems_skipped = sum_usr_elems - s->s_elems;
	*off_pos = act_pos - elems_skipped - 1;
	if (SLABLIST_TEST_GET_ELEM_POS_ENABLED()) {
		slab_t *old_slab;
		uint64_t obptr;
		int f = test_slab_get_elem_pos(sl, s, &old_slab, pos, *off_pos,
		    &obptr);
		uint64_t deriv = pos - obptr - 1;
		SLABLIST_TEST_GET_ELEM_POS(f, s, old_slab, *off_pos, deriv);
	}
	return (s);
}

slab_t *
slab_get_pos(slablist_t *sl, uint64_t pos)
{
	slab_t *slab = sl->sl_head;
	uint64_t i;
	uint64_t mod;

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
slablist_elem_t
slablist_get(slablist_t *sl, uint64_t pos)
{
	lock_list(sl);
	slablist_elem_t ret;
	uint64_t off_pos;
	slab_t *s;
	small_list_t *sml = NULL;
	if (sl->sl_is_small_list) {
		sml = sml_node_get(sl, pos);
		ret = sml->sml_data;
	} else {
		s = slab_get_elem_pos(sl, pos, &off_pos);
		ret = s->s_arr[off_pos];
	}

	unlock_list(sl);
	return (ret);
}

int
sub_is_elem_in_range(slablist_elem_t elem, subslab_t *s)
{
	int eq_min;
	int eq_max;
	slablist_t *sl = s->ss_list;
	slablist_elem_t max = s->ss_max;
	slablist_elem_t min = s->ss_min;
	eq_min = (sl->sl_cmp_elem(elem, min));
	eq_max = (sl->sl_cmp_elem(elem, max));


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

int
is_elem_in_range(slablist_elem_t elem, slab_t *s)
{
	int eq_min;
	int eq_max;
	slablist_t *sl = s->s_list;
	slablist_elem_t max = s->s_max;
	slablist_elem_t min = s->s_min;
	eq_min = (sl->sl_cmp_elem(elem, min));
	eq_max = (sl->sl_cmp_elem(elem, max));

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
 * If we need to find a slab-ptr in an sublayer who's data has been freed,
 * we can not use and of the other srch functions because they dereference
 * members of the slab-ptr (which is fine in all contexts, except when that
 * slab-ptrs underlying data may have been freed). So we do a simple
 * comparison against all elems until we get one that matches. If nothing
 * matches, we return -1. Otherwise we return the index.
 */
int
sublayer_slab_ptr_srch(void *elem, subslab_t *s)
{
	uint32_t i = 0;
	while (i < (s->ss_elems)) {
		if ((uint64_t)GET_SUBSLAB_ELEM(s, i) == (uint64_t)elem) {
			return ((int)i);
		}
		i++;
	}
	return ((int)-1);
}

/*
 * Binary search for `elem` in slab `s`.
 */
int
slab_bin_srch(slablist_elem_t elem, slab_t *s)
{
	int min = 0;
	int max = s->s_elems - 1;
	int c = 0;
	slablist_t *sl = s->s_list;
	while (max >= min) {
		int mid = (min + max) >> 1;
		slablist_elem_t mid_elem = s->s_arr[mid];
		SLABLIST_SLAB_BIN_SRCH(s, mid_elem);
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

int
slab_lin_srch(slablist_elem_t elem, slab_t *s)
{
	slablist_t *sl = s->s_list;
	int i = 0;
	while (i < s->s_elems
	    && sl->sl_cmp_elem(elem, s->s_arr[i]) > 0) {
		i++;
	}
	return (i);
}

/*
 * Does a binary search on a subslab that points to other subslabs.
 */
int
subslab_bin_srch(slablist_elem_t elem, subslab_t *s)
{
	int min = 0;
	int max = s->ss_elems - 1;
	int c = 0;
	while (max >= min) {
		int mid = (min + max) >> 1;
		void *mid_elem = GET_SUBSLAB_ELEM(s, mid);
		SLABLIST_SUBSLAB_BIN_SRCH(s, (subslab_t *)mid_elem);
		c = sub_is_elem_in_range(elem, (subslab_t *)mid_elem);
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

	if (min >= s->ss_elems) {
		/*
		 * Because `min` is the insertion point, if `min` is greater
		 * than the largest possible insertion point, we have to
		 * decrease `min` back to the largest possible insertion point.
		 */
		min = s->ss_elems - 1;
	}

	if (sub_is_elem_in_range(elem,
	    (subslab_t *)GET_SUBSLAB_ELEM(s, min)) > 0) {
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

int
subslab_lin_srch(slablist_elem_t elem, subslab_t *s)
{
	int i = 0;
	while (i < s->ss_elems
	    && sub_is_elem_in_range(elem,
	        (subslab_t *)GET_SUBSLAB_ELEM(s, i)) > 0) {
		i++;
	}
	return (i);
}

/*
 * Does a binary search on a subslab that points to other slabs.
 */
int
subslab_bin_srch_top(slablist_elem_t elem, subslab_t *s)
{
	int min = 0;
	int max = s->ss_elems - 1;
	int c = 0;
	while (max >= min) {
		int mid = (min + max) >> 1;
		void *mid_elem = GET_SUBSLAB_ELEM(s, mid);
		SLABLIST_SUBSLAB_BIN_SRCH_TOP(s, (slab_t *)mid_elem);
		c = is_elem_in_range(elem, (slab_t *)mid_elem);
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

	if (min >= s->ss_elems) {
		/*
		 * Because `min` is the insertion point, if `min` is greater
		 * than the largest possible insertion point, we have to
		 * decrease `min` back to the largest possible insertion point.
		 */
		min = s->ss_elems - 1;
	}

	if (is_elem_in_range(elem, (slab_t *)GET_SUBSLAB_ELEM(s, min)) > 0) {
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

int
subslab_lin_srch_top(slablist_elem_t elem, subslab_t *s)
{
	int i = 0;
	while (i < s->ss_elems
	    && is_elem_in_range(elem, (slab_t *)GET_SUBSLAB_ELEM(s, i)) > 0) {
		i++;
	}
	return (i);
}



/*
 *  This function tries to find the slab that contains elem in slab s.
 *  `crumbs` points to the ptr, that will hold either the bread crumb with the
 *  slab with `elem`, or the slab nearest to it.
 */
int
find_subslab_in_subslab(subslab_t *s, slablist_elem_t elem, subslab_t **found)
{
	int x = 0;
	x = subslab_bin_srch(elem, s);
	if (x > s->ss_elems - 1) {
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
		x = s->ss_elems - 1;
	}

	subslab_t *found2 = GET_SUBSLAB_ELEM(s, x);

	int r = sub_is_elem_in_range(elem, found2);

	*found = found2;

	return (r);
}

int
find_slab_in_subslab(subslab_t *s, slablist_elem_t elem, slab_t **found)
{
	int x = 0;
	x = subslab_bin_srch_top(elem, s);
	if (x > s->ss_elems - 1) {
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
		x = s->ss_elems - 1;
	}

	slab_t *next = (slab_t *)GET_SUBSLAB_ELEM(s, x);

	*found = next;

	int r = is_elem_in_range(elem, next);

	return (r);
}


int
sub_find_linear_scan(slablist_t *sl, slablist_elem_t elem, subslab_t **found)
{

	SLABLIST_SUB_LINEAR_SCAN_BEGIN(sl);
	uint64_t i = 0;
	subslab_t *s = sl->sl_head;
	int r = sub_is_elem_in_range(elem, s);

	/*
	 * Logically, this conditional is redundant, and can be removed.
	 * However, having it here makes average insertion performance
	 * empirically faster.
	 */
	if (r != FS_OVER_RANGE) {
		*found = s;
		SLABLIST_SUB_LINEAR_SCAN_END(r);
		return (r);
	}

	while (i < sl->sl_slabs) {
		SLABLIST_SUB_LINEAR_SCAN(sl, s);
		r = sub_is_elem_in_range(elem, s);
		if (r != FS_OVER_RANGE) {
			goto end;
		} else {
			if (s->ss_next != NULL) {
				s = s->ss_next;
			} else {
				goto end;
			}
		}
		i++;
	}
end:;
	*found = s;
	SLABLIST_SUB_LINEAR_SCAN_END(r);
	return (r);
}

int
find_linear_scan(slablist_t *sl, slablist_elem_t elem, slab_t **sbptr)
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
		r = is_elem_in_range(elem, s);
		if (r != FS_OVER_RANGE) {
			goto end;
		} else {
			if (s->s_next != NULL) {
				s = s->s_next;
			} else {
				goto end;
			}
		}
		i++;
	}
end:;
	*sbptr = s;
	SLABLIST_LINEAR_SCAN_END(r);
	return (r);
}

/*
 * Finds the slab into which `elem` could fit, by using the base-layer as a
 * starting point. Records all subslabs that were walked over into the `crumbs`.
 */
int
find_bubble_up(slablist_t *sl, slablist_elem_t elem, slab_t **sbptr)
{
	SLABLIST_BUBBLE_UP_BEGIN(sl);

	int fs;
	int layers = 1;
	int f = 0;
	subslab_t *found = NULL;
	if (sl->sl_sublayers > 1) {

		/* find the baseslab from which to start bubbling up */
		fs = sub_find_linear_scan(sl->sl_baselayer, elem, &found);
		SLABLIST_BUBBLE_UP(sl, found);

		/* Bubble up throught all of the sublayers */
		while (layers < sl->sl_sublayers) {

			/* We test the last subslab we found */
			if (SLABLIST_TEST_FIND_BUBBLE_UP_ENABLED()) {
				f = test_find_bubble_up(found, NULL, elem);
				SLABLIST_TEST_FIND_BUBBLE_UP(f, NULL,
				    found, elem, layers);
			}

			/* we get the appropriate subslab */
			fs = find_subslab_in_subslab(found, elem, &found);

			SLABLIST_BUBBLE_UP(sl, found);
			layers++;
		}

		/* We test the last subslab we found */
		if (SLABLIST_TEST_FIND_BUBBLE_UP_ENABLED()) {
			f = test_find_bubble_up(found, NULL, elem);
			SLABLIST_TEST_FIND_BUBBLE_UP(f, NULL, found, elem,
			    layers);
		}
		
		/* Find the target slab */
		fs = find_slab_in_subslab(found, elem, sbptr);

		/* We test the last slab we found */
		if (SLABLIST_TEST_FIND_BUBBLE_UP_ENABLED()) {
			f = test_find_bubble_up(NULL, *sbptr, elem);
			SLABLIST_TEST_FIND_BUBBLE_UP(f,
			    *sbptr, NULL, elem, layers);
		}

		SLABLIST_BUBBLE_UP_TOP(sl, *sbptr);
	}
	if (sl->sl_sublayers == 1) {
		fs = sub_find_linear_scan(sl->sl_sublayer, elem, &found);
		fs = find_slab_in_subslab(found, elem, sbptr);
		SLABLIST_BUBBLE_UP_TOP(sl, *sbptr);
	}


	SLABLIST_BUBBLE_UP_END(fs);
	return (fs);
}

/*
 * Function tries to find `key` in `sl`, and records the found elem into the
 * user-supplied backpointer `found`.
 */
int
slablist_find(slablist_t *sl, slablist_elem_t key, slablist_elem_t *found)
{
	lock_list(sl);

	SLABLIST_FIND_BEGIN(sl, key);
	slab_t *potential;
	uint64_t i = 0;
	slablist_elem_t ret;
	if (sl->sl_is_small_list) {
		small_list_t *sml = sl->sl_head;
		while (i < sl->sl_elems &&
		    sl->sl_cmp_elem(key, sml->sml_data) != 0) {
			sml = sml->sml_next;
			i++;
		}
		*found = sml->sml_data;
		SLABLIST_FIND_END(SL_SUCCESS, *found);
		unlock_list(sl);
		return (SL_SUCCESS);
	}

	if (SLIST_SORTED(sl->sl_flags)) {

		if (sl->sl_sublayers) {
			find_bubble_up(sl, key, &potential);
		} else {
			find_linear_scan(sl, key, &potential);
		}

		i = slab_bin_srch(key, potential);
		ret = potential->s_arr[i];

		*found  = ret;
		if (sl->sl_cmp_elem(key, ret) == 0) {
			SLABLIST_FIND_END(SL_SUCCESS, *found);
			unlock_list(sl);
			return (SL_SUCCESS);
		} else {
			SLABLIST_FIND_END(SL_ENFOUND, *found);
			unlock_list(sl);
			return (SL_ENFOUND);
		}
	} else {
		SLABLIST_FIND_END(SL_ARGORD, *found);
		unlock_list(sl);
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
slablist_elem_t
slablist_get_rand(slablist_t *sl)
{
	lock_list(sl);
	char *rfn = "/dev/urandom";
	int rfd = open(rfn, O_RDONLY);
	uint64_t r = get_rand_num(rfd);

	uint64_t op = 0;

	slablist_elem_t ret;
	ret.sle_u = 0;

	if (sl->sl_elems == 0) {
		return (ret);
	}
	r = r % sl->sl_elems;


	small_list_t *sml;
	slab_t *s;


	if (sl->sl_is_small_list) {
		sml = sml_node_get(sl, r);
		ret = sml->sml_data;
	} else {
		s = slab_get_elem_pos(sl, r, &op);
		ret = s->s_arr[(op)];
	}

	close(rfd);
	SLABLIST_GET_RAND(sl, ret);
	unlock_list(sl);
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
