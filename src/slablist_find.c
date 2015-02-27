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

slab_t *
slab_get_elem_pos_old(slablist_t *sl, uint64_t pos, uint64_t *off_pos)
{
	slab_t *slab = sl->sl_head;
	uint64_t i;
	uint64_t ecnt = 0;
	uint64_t mod;
	/* get the slab that contains the value at this position */
	uint64_t e = sl->sl_elems;

	/*
	 * If the number of elements is less than the position we want and the
	 * list is not circular, were return NULL.
	 */
	if (e < pos && !SLIST_IS_CIRCULAR(sl->sl_flags)) {
		return (NULL);
	}

	/*
	 * If on the other hand, the number of elements is less than the
	 * desired position, but the list _is_ circular, we take the mod of
	 * desired position and the number of elements. Otherwise, mod is equal
	 * to the position.
	 */
	if (e < pos) {
		mod = pos % e;
	} else {
		mod = pos;
	}

	i = 0;
	/*
	 * We keep updating `ecnt` with the number of elements in all of the
	 * slabs that we have visited. As soon as `ecnt` exceeds `mod`, which
	 * is the position that our desired element is in, we return the slab
	 * that we are currently at, and set `off_pos` to the value of `ecnt`
	 * _before_ we visited the current slab.
	 */
	while (i < sl->sl_slabs) {
		ecnt += slab->s_elems;

		if (ecnt >= mod) {
			*off_pos = ecnt - mod - 1;
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
	SLABLIST_GET_POS_BEGIN(sl, pos);
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

	if (act_pos == 0) {
		*off_pos = 0;
		return (sl->sl_head);
	}

	uint64_t sum_usr_elems = 0;
	uint64_t elems_skipped = 0;
	if (!(sl->sl_sublayers)) {
		SLABLIST_GET_POS_SHALLOW();
		uint64_t i = 0;
		while (i < sl->sl_slabs) {
			sum_usr_elems += slab->s_elems;

			if (sum_usr_elems >= act_pos) {
				elems_skipped = sum_usr_elems - slab->s_elems;
				*off_pos = act_pos - elems_skipped - 1;
				return (slab);
			}

			SLABLIST_GET_POS_TOP_WALK(slab);
			slab = slab->s_next;
			i++;
		}
	}

	slablist_t *bl = sl->sl_baselayer;
	subslab_t *b = bl->sl_head;
	uint16_t layers = bl->sl_layer;
	uint16_t layer = 0;
	/*
	 * We visit the subslabs in the baselayer, adding up their ss_usr_elems
	 * members, until this sum exceeds the position we are looking for.
	 */
	while (sum_usr_elems < act_pos) {
		sum_usr_elems += b->ss_usr_elems;
		SLABLIST_GET_POS_BASE_WALK(b);
		b = b->ss_next;
	}

	/*
	 * Clearly, the slab containing the element at the desired position is
	 * reachable from subslab `b`. In `elems_skipped` we store the number
	 * of elements we had to skip before we got subslab `b`.
	 */
	elems_skipped = sum_usr_elems - b->ss_usr_elems;

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
		while (sum_usr_elems < act_pos) {
			sum_usr_elems += c->ss_usr_elems;
			SLABLIST_GET_POS_SUB_WALK(c);
			c = c->ss_next;
		}
		b = c;
		elems_skipped = sum_usr_elems - b->ss_usr_elems;
		layer++;
	}

	/*
	 * We have reached the toplayer, and we now employ the same basic
	 * algorithm as in the above inner-loop but for slab_t's instead.
	 */
	slab_t *s = GET_SUBSLAB_ELEM(b, 0);
	sum_usr_elems = elems_skipped;
	while (sum_usr_elems < act_pos) {
		sum_usr_elems += s->s_elems;
		SLABLIST_GET_POS_TOP_WALK(s);
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
	SLABLIST_GET_POS_END(s);
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

	if (!(IS_SMALL_LIST(sl))) {
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
	slablist_elem_t ret;
	uint64_t off_pos = 0;
	slab_t *s;
	small_list_t *sml = NULL;
	if (IS_SMALL_LIST(sl)) {
		sml = sml_node_get(sl, pos);
		ret = sml->sml_data;
	} else {
		s = slab_get_elem_pos(sl, pos, &off_pos);
		ret = s->s_arr[off_pos];
	}

	return (ret);
}

slablist_elem_t
slablist_head(slablist_t *sl)
{
	slablist_elem_t ret;
	if (IS_SMALL_LIST(sl)) {
		small_list_t *sh = sl->sl_head;
		ret = sh->sml_data;
	} else {
		slab_t *h = sl->sl_head;
		ret = h->s_min;
	}
	return (ret);
}

slablist_elem_t
slablist_end(slablist_t *sl)
{
	slablist_elem_t ret;
	if (IS_SMALL_LIST(sl)) {
		small_list_t *sh = sl->sl_end;
		ret = sh->sml_data;
	} else {
		slab_t *h = sl->sl_end;
		ret = h->s_max;
	}
	return (ret);
}

int
slablist_cur(slablist_t *sl, slablist_bm_t *b, slablist_elem_t *e)
{
	if (IS_SMALL_LIST(sl)) {
		if (b->sb_node != NULL) {
			small_list_t *sm = b->sb_node;
			*e = sm->sml_data;
			return (0);
		}
	}
	if (b->sb_node != NULL) {
		slab_t *s = b->sb_node;
		*e = s->s_arr[(uint64_t)(b->sb_index)];
		return (0);
	}
	return (-1);
}

/*
 * 0 is success, -1 is end.
 */
int
slablist_next(slablist_t *sl, slablist_bm_t *b, slablist_elem_t *e)
{
	b->sb_list = sl;
	slab_t *s;
	small_list_t *sml;
	int i;
	if (b->sb_node == NULL) {
		if (IS_SMALL_LIST(sl)) {
			sml = sl->sl_head;
			b->sb_node = sml;
			*e = sml->sml_data;
		} else {
			s = sl->sl_head;
			b->sb_node = s;
			b->sb_index = 0;
			*e = s->s_arr[0];
		}
		return (0);
	}
	if (IS_SMALL_LIST(sl)) {
		sml = b->sb_node;
		b->sb_node = sml->sml_next;
		if (b->sb_node == NULL) {
			return (-1);
		}
		sml = b->sb_node;
		*e = sml->sml_data;
		return (0);
	} else {
		s = b->sb_node;
		b->sb_index++;
		i = b->sb_index;
		if (b->sb_index == s->s_elems) {
			b->sb_node = s->s_next;
			s = s->s_next;
			b->sb_index = 0;
			i = 0;
			if (s == NULL) {
				return (-1);
			}
		}
		*e = s->s_arr[i];
		return (0);
	}
}

int
slablist_prev(slablist_t *sl, slablist_bm_t *b, slablist_elem_t *e)
{
	b->sb_list = sl;
	slab_t *s;
	small_list_t *prev = NULL;
	small_list_t *sml = NULL;
	int i;
	if (b->sb_node == NULL) {
		if (IS_SMALL_LIST(sl)) {
			sml = sl->sl_end;
			b->sb_node = sml;
			*e = sml->sml_data;
		} else {
			s = sl->sl_end;
			b->sb_node = s;
			b->sb_index = s->s_elems - 1;
			/*
			 * Unless we cast the index to type `int`, GCC will
			 * spew some warnings about a subscript of type 'char'.
			 * The subcript is of type int8_t, and GCC is warning
			 * us that we might end up with a negative index. We
			 * use the negative index later on in the code to
			 * determine if we've passed the beginning of a slab.
			 * So, GCC has the best intentions, but this is a
			 * non-issue. We're just tricking it into thinking that
			 * everything is OK.
			 */
			*e = s->s_arr[(int)(b->sb_index)];
		}
		return (0);
	}
	if (IS_SMALL_LIST(sl)) {
		if (sl->sl_elems == 1 && sl->sl_head == b->sb_node) {
			return (-1);
		}
		sml = sl->sl_head;
		while (sml != b->sb_node) {
			prev = sml;
			sml = sml->sml_next;
		}
		b->sb_node = prev;
		if (prev == NULL) {
			return (-1);
		}
		*e = prev->sml_data;
		return (0);
	} else {
		s = b->sb_node;
		b->sb_index--;
		i = b->sb_index;
		if (b->sb_index < 0) {
			b->sb_node = s->s_prev;
			s = s->s_prev;
			if (s == NULL) {
				return (-1);
			}
			i = s->s_elems - 1;
			b->sb_index = i;
		}
		*e = s->s_arr[i];
		return (0);
	}
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
 * This function gets the last subslab in whose range `elem` fits. It is used
 * to deal with potential duplicates, when sorting an ordered slab list.
 */
subslab_t *
list_get_last_subslab(slablist_t *sl, slablist_elem_t elem, subslab_t *s)
{
	subslab_t *tmp = s;
	while (tmp->ss_next != NULL &&
	    sl->sl_bnd_elem(elem, tmp->ss_next->ss_min,
	    tmp->ss_next->ss_max) >= 0) {
		tmp = tmp->ss_next;
	}
	return (tmp);
}

/*
 * Same as previous function but for slabs. Used on single-layer slab lists.
 */
slab_t *
list_get_last_slab(slablist_t *sl, slablist_elem_t elem, slab_t *s)
{
	slab_t *tmp = s;
	while (tmp->s_next != NULL &&
	    sl->sl_bnd_elem(elem, tmp->s_next->s_min,
	    tmp->s_next->s_max) >= 0) {
		tmp = tmp->s_next;
	}
	return (tmp);
}

/*
 * This function tries to find the last subslab into whose range `elem` falls
 * into. It is typically used by the binary search functions that operate on
 * subslabs. It is also typically used to deal with potential duplicates when
 * sorting an ordered slab list. This function works fine when bubbling up, but
 * when rippling new subslabs/slabs down, the returned value may have to be
 * incremented (in order to turn it into an insertion point).
 */
int
subslab_get_last_subslab(slablist_t *sl, slablist_elem_t elem, subslab_t *s,
    int i)
{
	int j = i;
	while (j < s->ss_elems) {
		subslab_t *ss = GET_SUBSLAB_ELEM(s, j);
		if (sl->sl_bnd_elem(elem, ss->ss_min, ss->ss_max) < 0) {
			break;
		}
		j++;
	}
	/*
	 * By the time we broke out of the loop, `j` was set to the index of
	 * the subslab that couldn't possibly hold `elem`. We have to decrement
	 * j, if possible. This must be so when used in the bubbling-up
	 * context.
	 */
	if (j > 0) {
		return (j - 1);
	}
	return (j);
}

/*
 * Same as the previous function but for subslabs that reference slabs in the
 * top layer.
 */
int
subslab_get_last_slab(slablist_t *sl, slablist_elem_t elem, subslab_t *s,
    int i)
{
	int j = i;
	while (j < s->ss_elems) {
		slab_t *ss = GET_SUBSLAB_ELEM(s, j);
		if (sl->sl_bnd_elem(elem, ss->s_min, ss->s_max) < 0) {
			break;
		}
		j++;
	}
	/*
	 * Same as in above function.
	 */
	if (j > 0) {
		return (j - 1);
	}
	return (j);
}

/*
 * Same deal as some of the above functions. Handles duplicates. It gets the
 * elem that is immediately after the last elem. This is because we expect to
 * only use this function from an insertion context, not just a search context
 * (unlike the above functions).
 */
int
slab_get_last_elem(slablist_t *sl, slablist_elem_t elem, slab_t *s, int i)
{
	int j = i;
	while (j < s->s_elems && sl->sl_cmp_elem(elem, s->s_arr[j]) >= 0) {
		j++;
	}
	return (j);
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
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	while (max >= min) {
		int mid = (min + max) >> 1;
		slablist_elem_t mid_elem = s->s_arr[mid];
		SLABLIST_SLAB_BIN_SRCH(s, mid_elem, mid);
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
			if (sorting) {
				return (slab_get_last_elem(sl, elem, s, mid));
			}
			return (mid);
		}
	}

	/*
	 * Because `min` is the insertion point, if `min` is greater than the
	 * largest possible insertion point, we have to decrease `min` back to
	 * the largest possible insertion point.
	 */
	if (min >= s->s_elems) {
		min = s->s_elems - 1;
	}

	/*
	 * If the binary search took us to an element that is smaller than
	 * `elem`, we return the index of the next element, which is likely to
	 * be larger. This is because all of our code insertion code assumes
	 * that we return the index that we want to insert `elem` _at_.
	 */
	if (sl->sl_cmp_elem(elem, s->s_arr[min]) > 0) {
		if (sorting) {
			return (slab_get_last_elem(sl, elem, s, min + 1));
		}
		return (min + 1);
	}

	/*
	 * Just in case.
	 */
	if (sorting) {
		return (slab_get_last_elem(sl, elem, s, min));
	}
	return (min);
}

int
slab_lin_srch(slablist_elem_t elem, slab_t *s)
{
	slablist_t *sl = s->s_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	int i = 0;
	while (i < s->s_elems &&
	    sl->sl_cmp_elem(elem, s->s_arr[i]) > 0) {
		i++;
	}
	if (sorting) {
		return (slab_get_last_elem(sl, elem, s, i));
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
	slablist_t *sl = s->ss_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	while (max >= min) {
		int mid = (min + max) >> 1;
		void *mid_elem = GET_SUBSLAB_ELEM(s, mid);
		SLABLIST_SUBSLAB_BIN_SRCH(s, (subslab_t *)mid_elem, mid);
		c = sl->sl_bnd_elem(elem, ((subslab_t *)mid_elem)->ss_min,
			((subslab_t *)mid_elem)->ss_max);
		if (c > 0) {
			min = mid + 1;
			continue;
		}
		if (c < 0) {
			max = mid - 1;
			continue;
		}
		if (c == 0) {
			if (sorting) {
				return (subslab_get_last_subslab(sl, elem, s,
				    mid));
			}
			return (mid);
		}
	}

	/*
	 * Because `min` is the insertion point, if `min` is greater than the
	 * largest possible insertion point, we have to decrease `min` back to
	 * the largest possible insertion point.
	 */
	if (min >= s->ss_elems) {
		min = s->ss_elems - 1;
	}

	subslab_t *minss = GET_SUBSLAB_ELEM(s, min);
	c = sl->sl_bnd_elem(elem, minss->ss_min, minss->ss_max);
	/*
	 * If the binary search took us to an element that is smaller than
	 * `elem`, we return the index of the next element, which is likely to
	 * be larger. This is because all of our code insertion code assumes
	 * that we return the index that we want to insert `elem` _at_.
	 */
	if (c > 0) {
		if (sorting) {
			return (subslab_get_last_subslab(sl, elem, s,
			    min + 1));
		}
		return (min + 1);
	}

	/*
	 * Just in case.
	 */
	if (sorting) {
		return (subslab_get_last_subslab(sl, elem, s, min));
	}
	return (min);
}

int
subslab_lin_srch(slablist_elem_t elem, subslab_t *s)
{
	int i = 0;
	slablist_elem_t e;
	e.sle_p = GET_SUBSLAB_ELEM(s, i);
	subslab_t *eptr = e.sle_p;
	slablist_t *sl = s->ss_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	while (i < s->ss_elems &&
	    sl->sl_bnd_elem(elem, eptr->ss_min, eptr->ss_max) > 0) {
		e.sle_p = GET_SUBSLAB_ELEM(s, i);
		eptr = e.sle_p;
		i++;
	}
	if (sorting) {
		i = subslab_get_last_subslab(sl, elem, s, i);
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
	slablist_t *sl = s->ss_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	void **arr = s->ss_arr->sa_data;
	if (!sorting) {
		while (max >= min) {
			int mid = (min + max) >> 1;
			void *mid_elem = arr[mid];
			slab_t *mid_slab = (slab_t *)mid_elem;
			c = sl->sl_bnd_elem(elem, mid_slab->s_min,
				mid_slab->s_max);
			if (c > 0) {
				min = mid + 1;
				continue;
			}
			if (c < 0) {
				max = mid - 1;
				continue;
			}
			return (mid);
		}
	} else {
		while (max >= min) {
			int mid = (min + max) >> 1;
			void *mid_elem = arr[mid];
			slab_t *mid_slab = (slab_t *)mid_elem;
			c = sl->sl_bnd_elem(elem, mid_slab->s_min,
				mid_slab->s_max);
			if (c > 0) {
				min = mid + 1;
				continue;
			}
			if (c < 0) {
				max = mid - 1;
				continue;
			}
			return (subslab_get_last_slab(sl, elem, s,
			    mid));
		}
	}

	/*
	 * Because `min` is the insertion point, if `min` is greater than the
	 * largest possible insertion point, we have to decrease `min` back to
	 * the largest possible insertion point.
	 */
	if (min >= s->ss_elems) {
		min = s->ss_elems - 1;
	}

	slab_t *tmp = GET_SUBSLAB_ELEM(s, min);
	/*
	 * If the binary search took us to an element that is smaller than
	 * `elem`, we return the index of the next element, which is likely to
	 * be larger. This is because all of our code insertion code assumes
	 * that we return the index that we want to insert `elem` _at_.
	 */
	if (sl->sl_bnd_elem(elem, tmp->s_min, tmp->s_max) > 0) {
		if (sorting) {
			return (subslab_get_last_slab(sl, elem, s, min + 1));
		}
		return (min + 1);
	}

	/*
	 * Just in case.
	 */
	if (sorting) {
		return (subslab_get_last_slab(sl, elem, s, min));
	}
	return (min);
}

int
subslab_lin_srch_top(slablist_elem_t elem, subslab_t *s)
{
	int i = 0;
	slablist_elem_t e;
	e.sle_p = GET_SUBSLAB_ELEM(s, i);
	slab_t *eptr = e.sle_p;
	slablist_t *sl = s->ss_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	while (i < s->ss_elems &&
	    sl->sl_bnd_elem(elem, eptr->s_min, eptr->s_max) > 0) {
		e.sle_p = GET_SUBSLAB_ELEM(s, i);
		eptr = e.sle_p;
		i++;
	}
	if (sorting) {
		i = subslab_get_last_slab(sl, elem, s, i);
	}
	return (i);
}


/*
 *  This function tries to find the slab that contains elem in slab s.  `found`
 *  points to the ptr of the subslab with the slab with `elem`, or the slab
 *  nearest to it.
 */
int
find_subslab_in_subslab(subslab_t *s, slablist_elem_t elem, subslab_t **found)
{
	int x = 0;
	x = subslab_bin_srch(elem, s);
	slablist_t *sl = s->ss_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	/*
	 * If we get an index `x` that is larger than the index of the last
	 * element, we set x to the index of the last element. This is
	 * neccessary, since slab_bin_srch always assumes that we are searching
	 * for an _insertion point_. That is, we want the index at which a new
	 * element will be inserted. In the context of this function, we really
	 * want the slab who's range is closest to that of `elem`. Hence why if
	 * we exceed the maximum index, we know that the last slab is the one
	 * we are looking for. Note that this is not a problem for an `elem`
	 * that is below the slab's range, as the insertion point (0) and the
	 * slab-index we are looking for are the same.
	 */
	if (!sorting && x > s->ss_elems - 1) {
		x = s->ss_elems - 1;
	}

	subslab_t *found2 = GET_SUBSLAB_ELEM(s, x);

	int r = sl->sl_bnd_elem(elem, found2->ss_min, found2->ss_max);
	if (sorting && r == FS_IN_RANGE) {
		if (sl->sl_cmp_elem(elem, found2->ss_max) == 0) {
			r = FS_OVER_RANGE;
		}
	}

	*found = found2;

	return (r);
}

int
find_slab_in_subslab(subslab_t *s, slablist_elem_t elem, slab_t **found)
{
	int x = 0;
	x = subslab_bin_srch_top(elem, s);
	slablist_t *sl = s->ss_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	/*
	 * If we get an index `x` that is larger than the index of the last
	 * element, we set x to the index of the last element. This is
	 * neccessary, since slab_bin_srch always assumes that we are searching
	 * for an _insertion point_. That is, we want the index at which a new
	 * element will be inserted. In the context of this function, we really
	 * want the slab who's range is closest to that of `elem`. Hence why if
	 * we exceed the maximum index, we know that the last slab is the one
	 * we are looking for. Note that this is not a problem for an `elem`
	 * that is below the slab's range, as the insertion point (0) and the
	 * slab-index we are looking for are the same.
	 */
	if (!sorting && x > s->ss_elems - 1) {
		x = s->ss_elems - 1;
	}

	slab_t *next = (slab_t *)GET_SUBSLAB_ELEM(s, x);

	*found = next;

	int r = sl->sl_bnd_elem(elem, next->s_min, next->s_max);
	if (sorting && r == FS_IN_RANGE) {
		if (sl->sl_cmp_elem(elem, next->s_max) == 0) {
			r = FS_OVER_RANGE;
		}
	}

	return (r);
}


int
sub_find_linear_scan(slablist_t *sl, slablist_elem_t elem, subslab_t **found)
{

	SLABLIST_SUB_LINEAR_SCAN_BEGIN(sl);
	uint64_t i = 0;
	subslab_t *s = sl->sl_head;
	int r = sl->sl_bnd_elem(elem, s->ss_min, s->ss_max);
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);

	/*
	 * Logically, this conditional is redundant, and can be removed.
	 * However, having it here makes average insertion performance
	 * empirically faster.
	 */
	if (!sorting && r != FS_OVER_RANGE) {
		*found = s;
		SLABLIST_SUB_LINEAR_SCAN_END(r);
		return (r);
	}

	while (i < sl->sl_slabs) {
		SLABLIST_SUB_LINEAR_SCAN(sl, s);
		r = sl->sl_bnd_elem(elem, s->ss_min, s->ss_max);
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
	/*
	 * If elem is in the range of s, we want to compensate for any
	 * duplicate runs. If, for example, e = 5, and s is filled with only
	 * 5's, we want to look ahead to make sure that  s->next is _not_
	 * filled with any 5's too. If it is, we want to return s->next (or
	 * s->next->next, or the last slab that has a five in it).
	 */
	if (sorting && r == FS_IN_RANGE) {
		s = list_get_last_subslab(sl, elem, s);
	}
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
	int r = sl->sl_bnd_elem(elem, s->s_min, s->s_max);
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);

	/*
	 * Logically, this conditional is redundant, and can be removed.
	 * However, having it here makes average insertion performance
	 * empirically faster.
	 */
	if (!sorting && r != FS_OVER_RANGE) {
		*sbptr = s;
		SLABLIST_LINEAR_SCAN_END(r);
		return (r);
	}

	while (i < sl->sl_slabs) {
		SLABLIST_LINEAR_SCAN(sl, s);
		r = sl->sl_bnd_elem(elem, s->s_min, s->s_max);
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
	/*
	 * If elem is in the range of s, we want to compensate for any
	 * duplicate runs. If, for example, e = 5, and s is filled with only
	 * 5's, we want to look ahead to make sure that  s->next is _not_
	 * filled with any 5's too. If it is, we want to return s->next (or
	 * s->next->next, or the last slab that has a five in it).
	 */
	if (sorting && r == FS_IN_RANGE) {
		s = list_get_last_slab(sl, elem, s);
		SLABLIST_LINEAR_SCAN(sl, s);
		if (sl->sl_cmp_elem(elem, s->s_max) == 0) {
			r = FS_OVER_RANGE;
		}
	}
	*sbptr = s;
	SLABLIST_LINEAR_SCAN_END(r);
	return (r);

}



/*
 * Finds the slab into which `elem` could fit, by using the base-layer as a
 * starting point.
 */
int
find_bubble_up(slablist_t *sl, slablist_elem_t elem, slab_t **sbptr)
{
	SLABLIST_BUBBLE_UP_BEGIN(sl);

	int fs = 0;
	int layers = 1;
	int f = 0;
	subslab_t *found = NULL;
#define	E_TEST_FBU_NOT_LAYERED 48
	if (sl->sl_sublayers == 0) {
		SLABLIST_TEST_FIND_BUBBLE_UP(E_TEST_FBU_NOT_LAYERED, NULL,
		    NULL, elem, 0);
	}
	if (sl->sl_sublayers > 1) {

		/* find the baseslab from which to start bubbling up */
		sub_find_linear_scan(sl->sl_baselayer, elem, &found);
		SLABLIST_BUBBLE_UP(sl, found);

		/* Bubble up through all of the sublayers */
		while (layers < sl->sl_sublayers) {

			/* We test the last subslab we found */
			if (SLABLIST_TEST_FIND_BUBBLE_UP_ENABLED()) {
				f = test_find_bubble_up(found, NULL, elem);
				SLABLIST_TEST_FIND_BUBBLE_UP(f, NULL,
				    found, elem, layers);
			}

			/* we get the appropriate subslab */
			find_subslab_in_subslab(found, elem, &found);

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
		sub_find_linear_scan(sl->sl_sublayer, elem, &found);
		fs = find_slab_in_subslab(found, elem, sbptr);
		SLABLIST_BUBBLE_UP_TOP(sl, *sbptr);
	}


	SLABLIST_BUBBLE_UP_END(fs);
	return (fs);
}

/*
 * This function, finds the smallest element within a range, and stores it in
 * `ret`, while also storing a reference to in the bookmark `bm`. This function
 * returns either -1, 0, or 1. If 0, that means that the value we found is
 * within the range. If 1 or -1, that means the value is over or under the
 * range, respectively.
 *
 * Naturally, changes to the slablist can cause an inconsistency between the
 * bookmark and the actual data in the slablist, so don't modify the slablist,
 * or you'll have to re-run this function.
 */
int
slablist_range_min(slablist_t *sl, slablist_bm_t *bm, slablist_elem_t min,
    slablist_elem_t max, slablist_elem_t *ret)
{
	if (IS_SMALL_LIST(sl)) {
		int smlret = 0;
		int smlbnd;
		while (smlret == 0) {
			smlret = slablist_next(sl, bm, ret);
			smlbnd = sl->sl_bnd_elem(*ret, min, max);
			/*
			 * We have reached the rage. And by virtue of using
			 * slablist_next, we've filled out the bookmark and the
			 * return-pointer.
			 */
			if (smlbnd == 0) {
				return (0);
			} else if (smlbnd > 0) {
				break;
			}
		}
		/*
		 * We're here because we either reached the end of the list, or
		 * because we never found anything in the range min..max.
		 */
		return (smlbnd);
	}
	slab_t *smin = NULL;
	int i;
	if (sl->sl_sublayers > 0) {
		find_bubble_up(sl, min, &smin);
	} else {
		find_linear_scan(sl, min, &smin);
	}
	i = slab_bin_srch(min, smin);
	/*
	 * Because slab_bin_srch _always_ returns the insertion point,
	 * smin->s_arr[i] is always >= the element that we are searching for.
	 *
	 * The only exceptions are if the range doesn't exist. In which case
	 * this is reflected in the non-zero return status of this function.
	 */
	bm->sb_node = smin;
	bm->sb_index = i;
	*ret = smin->s_arr[i];
	return (sl->sl_bnd_elem(smin->s_arr[i], min, max));
}

/*
 * Same as above, but it finds the _end_ of the range.
 */
int
slablist_range_max(slablist_t *sl, slablist_bm_t *bm, slablist_elem_t min,
    slablist_elem_t max, slablist_elem_t *ret)
{
	if (IS_SMALL_LIST(sl)) {
		int smlret = 0;
		int smlbnd;
		while (smlret == 0) {
			smlret = slablist_next(sl, bm, ret);
			smlbnd = sl->sl_bnd_elem(*ret, min, max);
			/*
			 * We have reached the rage. And by virtue of using
			 * slablist_next, we've filled out the bookmark and the
			 * return-pointer.
			 */
			if (smlbnd == 0) {
				return (0);
			} else if (smlbnd > 0) {
				break;
			}
		}
		/*
		 * We're here because we either reached the end of the list, or
		 * because we never found anything in the range min..max.
		 */
		return (smlbnd);
	}
	slab_t *smax = NULL;
	int i;
	if (sl->sl_sublayers > 0) {
		find_bubble_up(sl, max, &smax);
	} else {
		find_linear_scan(sl, max, &smax);
	}
	i = slab_bin_srch(max, smax);
	/*
	 * Because slab_bin_srch _always_ returns the insertion point,
	 * smax->s_arr[i] is always >= the element that we are searching for.
	 *
	 * The only exceptions are if the range doesn't exist. In which case
	 * this is reflected in the non-zero return status of this function.
	 */
	bm->sb_node = smax;
	bm->sb_index = i;
	*ret = smax->s_arr[i];
	return (sl->sl_bnd_elem(smax->s_arr[i], min, max));

}
/*
 * Function tries to find `key` in `sl`, and records the found elem into the
 * user-supplied backpointer `found`.
 */
int
slablist_find(slablist_t *sl, slablist_elem_t key, slablist_elem_t *found)
{

	SLABLIST_FIND_BEGIN(sl, key);
	slab_t *potential = NULL;
	uint64_t i = 0;
	slablist_elem_t ret;
	if (IS_SMALL_LIST(sl) && SLIST_SORTED(sl->sl_flags)) {
		small_list_t *sml = sl->sl_head;
		while (i < sl->sl_elems &&
		    sl->sl_cmp_elem(key, sml->sml_data) != 0) {
			sml = sml->sml_next;
			i++;
		}
		if (sml != NULL) {
			*found = sml->sml_data;
			SLABLIST_FIND_END(SL_SUCCESS, *found);
			return (SL_SUCCESS);
		} else {
			SLABLIST_FIND_END(SL_ENFOUND, *found);
			return (SL_ENFOUND);
		}
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

#define	NO_MATCH	0
#define	PARTIAL_MATCH	1
#define	FULL_MATCH	2
typedef struct subseq {
	char		sseq_match;
	uint64_t	sseq_matched;
	uint64_t	sseq_matchable;
	slablist_t	*sseq_sub1;
	slablist_elem_t	*sseq_sub2;
} subseq_t;

/*
 * XXX THIS code doesn't pass the static analyzer and needs some serious
 * fixing. DO NOT USE!
 */
slablist_elem_t
subseq_cb(slablist_elem_t acc, slablist_elem_t *arr, uint64_t elems)
{
	(void)acc;
	(void)arr;
	(void)elems;
/*
 * Because this code sets off clang analyzer warnings, we make is 'hidden' or
 * 'ignored' via the undefined macro below.
 */
#ifdef SUBSEQ_NO_IGNORE
	subseq_t *seq = acc.sle_p;
	slablist_t *sl = seq->sseq_sub1;
	slablist_elem_t *seq_arr = seq->sseq_sub2;
	/*
	 * If we have a full match already, we don't bother searching for more
	 * matches, and just keep returning acc until the fold finishes.
	 *
	 * TODO: There should be some way of breaking out of a fold.
	 */
	if (seq->sseq_match == FULL_MATCH) {
		return (acc);
	}
	uint64_t i = 0;
	uint64_t j = 1;
	if (seq->sseq_match == NO_MATCH) {
		/* find head of sequence */
		slablist_elem_t f;
		if (sl != NULL) {
			f = slablist_get(sl, 0);
		} else {
			f = seq_arr[0];
		}
		while (i < elems) {
			if (sl->sl_cmp_elem(arr[i], f) == 0) {
				seq->sseq_matched++;
				seq->sseq_match = PARTIAL_MATCH;
				break;
			}
			i++;
		}
	}
	if (seq->sseq_match == PARTIAL_MATCH) {
		j = seq->sseq_matched;
		/* find the tail of the sequence */
		if (sl != NULL) {
			while (j < seq->sseq_matchable) {
				slablist_elem_t c = slablist_get(sl, j);
				if (sl->sl_cmp_elem(arr[i], c) == 0) {
					seq->sseq_matched++;
				} else {
					break;
				}
				i++;
				j++;
			}
		} else {
			while (j < seq->sseq_matchable) {
				slablist_elem_t c = seq_arr[j];
				if (sl->sl_cmp_elem(arr[i], c) == 0) {
					seq->sseq_matched++;
				} else {
					break;
				}
				i++;
				j++;
			}
		}
		if (seq->sseq_matched == seq->sseq_matchable) {
			seq->sseq_match = FULL_MATCH;
			return (acc);
		}
		if (seq->sseq_matched < seq->sseq_matchable &&
		    i < elems) {
			seq->sseq_match = NO_MATCH;
		}
	}
	return (acc);
#endif
}

/*
 * This function determines if `sl` has a subsequence, which is either stored
 * in the slablist `sub1` or in the array `sub2` of length `len`.
 */
int
slablist_subseq(slablist_t *sl, slablist_t *sub1, slablist_elem_t *sub2,
    uint64_t len)
{
	if (sub1 == NULL && sub2 == NULL) {
		return (0);
	}
	subseq_t seq;
	if (sub1 != NULL) {
		seq.sseq_match = 0;
		seq.sseq_matched = 0;
		seq.sseq_matchable = slablist_get_elems(sl);
		seq.sseq_sub1 = sub1;
		seq.sseq_sub2 = NULL;
	} else {
		seq.sseq_match = 0;
		seq.sseq_matched = 0;
		seq.sseq_matchable = len;
		seq.sseq_sub1 = NULL;
		seq.sseq_sub2 = sub2;
	}
	slablist_elem_t acc;
	acc.sle_p = &seq;
	(void) slablist_foldr(sl, subseq_cb, acc);
	if (seq.sseq_match == FULL_MATCH) {
		return (1);
	}
	return (0);
}
