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

/*
 * Insertion Implementation
 *
 * This file implements the procedures that add elements into Slab Lists.
 * The code in this file uses functions defined in slablist_cons.c,
 * slablist_find.c, and slablist_test.c.
 *
 * The internal architecture of libslablist is described in slablist_impl.h
 *
 * Mastery of the above files is assumed by the block comments in this file.
 */

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <strings.h>
#include <stdio.h>
#include "slablist_impl.h"
#include "slablist_provider.h"
#include "slablist_test.h"
#include "slablist_cons.h"
#include "slablist_find.h"

/*
 * This function adds an element to a slablist's linked list. Slablists use
 * linked lists to hold data, if there are fewer than 256 elements in the
 * slablist. This way, there is always 50% metadata overhead. Whereas if we
 * used slabs exclusively, there is 90% metadata overhead if we stored only 50
 * elements.
 */
int
small_list_add(slablist_t *sl, slablist_elem_t elem, int rep,
    slablist_elem_t *repd_elem)
{

	int ret = 0;
	/*
	 * If the ptr to the head of this slablist is null, we have to create
	 * the head node, store `elem` into it, update metadata, and return.
	 */
	if (sl->sl_head == NULL) {
		sl->sl_head = mk_sml_node();
		SLABLIST_SET_HEAD(sl, elem);
		sl->sl_end = sl->sl_head;
		SLABLIST_SET_END(sl, elem);
		((small_list_t *)sl->sl_head)->sml_data = elem;
		sl->sl_elems++;
		SLABLIST_SL_INC_ELEMS(sl);
		ret = SL_SUCCESS;
		goto end;
	}

	small_list_t *sml;
	small_list_t *nsml = NULL;

	/*
	 * We place the element in the position so that it is less than the
	 * element after it.
	 */
	if (SLIST_SORTED(sl->sl_flags)) {
		sml = sl->sl_head;
		small_list_t *prev = NULL;
		uint64_t i = 0;
		while (i < sl->sl_elems) {
			/*
			 * If `elem` is less than the data in the current
			 * sml_node, we add `elem` before it.
			 */
			if (sl->sl_cmp_elem(elem, sml->sml_data) < 0) {
				nsml = mk_sml_node();
				nsml->sml_data = elem;
				link_sml_node(sl, prev, nsml);
				ret = SL_SUCCESS;
				goto end;
			}
			/*
			 * If `elem` is equal to the current sml_node, we
			 * either replace its data with `elem` or, we error out
			 * depending on the user's preference.
			 */
			if (sl->sl_cmp_elem(elem, sml->sml_data) == 0) {
				if (rep) {
					if (repd_elem != NULL) {
						*repd_elem = sml->sml_data;
					}
					sml->sml_data = elem;
					SLABLIST_SLAB_AR(sl, NULL, elem, 1);
				} else {
					SLABLIST_SLAB_AR(sl, NULL, elem, 0);
					ret = SL_EDUP;
					goto end;
				}
			}
			/*
			 * If we are not <= to sml, we go to the next node,
			 * until we reach the end.
			 */
			prev = sml;
			sml = sml->sml_next;
			i++;
		}

		/*
		 * We've reached the end of the list, and make elem the last
		 * element.
		 */
		if (sml == NULL) {
			nsml = mk_sml_node();
			sl->sl_end = nsml;
			SLABLIST_SET_END(sl, elem);
			nsml->sml_data = elem;
			link_sml_node(sl, prev, nsml);
			ret = SL_SUCCESS;
			goto end;
		}


	}

	/*
	 * We place the element at the end of the list.
	 */
	uint64_t i = 0;
	sml = sl->sl_head;
	while (i < (sl->sl_elems - 1)) {
		sml = sml->sml_next;
		i++;
	}
	nsml = mk_sml_node();
	nsml->sml_data = elem;
	link_sml_node(sl, sml, nsml);
	sl->sl_end = nsml;
	SLABLIST_SET_END(sl, elem);

end:;

	/*
	 * Now that we have modifies the list, we run some tests, if DTrace has
	 * enabled those probes.
	 */
	if (SLABLIST_TEST_IS_SML_LIST_ENABLED()) {
		SLABLIST_TEST_IS_SML_LIST(!(IS_SMALL_LIST(sl)));
	}

	/*
	 * If test probe is enabled, we verify that the elems are sorted.
	 */
	if (SLABLIST_TEST_SMLIST_ELEMS_SORTED_ENABLED()) {
		int f = test_smlist_elems_sorted(sl);
		SLABLIST_TEST_SMLIST_ELEMS_SORTED(f);
	}


	return (ret);
}

/*
 * Inserts an `elem` to add into slab `s` at index `i`.
 */
static void
add_elem(slab_t *s, slablist_elem_t elem, int i)
{
	/*
	 * Test the consistency of the slab before addition.
	 */
	if (SLABLIST_TEST_ADD_ELEM_ENABLED()) {
		int f = test_add_elem(s, elem, i);
		if (f) {
			SLABLIST_TEST_ADD_ELEM(f, s, elem, i);
		}
	}

	int ip = 0;		/* insertion-point */

	ip = i;

	SLABLIST_FWDSHIFT_BEGIN(s->s_list, s, i);
	size_t shiftsz = (s->s_elems - (size_t)i) * 8;
	bcopy(&(s->s_arr[i]), &(s->s_arr[(i+1)]), shiftsz);
	SLABLIST_FWDSHIFT_END();
	s->s_arr[i] = elem;

	/*
	 * If we added at the beginning of the slab, we have to change the
	 * minimum.
	 */
	if (ip == 0) {
		s->s_min = s->s_arr[0];
		SLABLIST_SLAB_SET_MIN(s);
	}

	/*
	 * If we added at the end of the slab, we have to change the
	 * maximum.
	 */
	if (ip == (s->s_elems)) {
		s->s_max = s->s_arr[(s->s_elems)];
		SLABLIST_SLAB_SET_MAX(s);
	}

	/*
	 * Sometimes we are adding into a slab that is full, with the intention
	 * of moving the last element into the next slab (which is not full).
	 * In these situations, we don't want to increment anything, as all the
	 * neccessary increments will occur when we move the elem into the next
	 * slab (i.e. the next time we call this function). But we do want to
	 * update the max value.
	 */
	if (s->s_elems < SELEM_MAX) {
		s->s_elems++;
		SLABLIST_SLAB_INC_ELEMS(s);
	} else {
		s->s_max = s->s_arr[(SELEM_MAX - 1)];
		s->s_min = s->s_arr[0];
		SLABLIST_SLAB_SET_MAX(s);
		SLABLIST_SLAB_SET_MIN(s);
	}

	if (SLABLIST_TEST_ADD_ELEM_ENABLED()) {
		int f = test_slab_extrema(s);
		if (f) {
			SLABLIST_TEST_ADD_ELEM(f, s, elem, i);
		}
	}
}

/*
 * This function adds either slab `s1` or subslab `s2` into subslab `s`, at
 * index `i`.
 */
static void
add_slab(subslab_t *s, slab_t *s1, subslab_t *s2, uint64_t i)
{

	slablist_t *sl;
	sl = s->ss_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);

	/*
	 * If we are adding a sub/slab to a slab list that is used temporarily
	 * for sorting, we _may_ have to incremement `i`. The reasoning for
	 * this is that 'plain' slab lists don't support duplicates, which
	 * means we can get away with a lot of short cuts --- like checking
	 * where to insert a sub/slab S into a subslab_t SS by doing a bin_srch
	 * for S->max in SS. This works because S->max is a unique,
	 * non-duplicate element. It is not possible for S->max to be equal to
	 * any other maximum or minimum, in a plain slab list. When we are
	 * using non-plain slab lists, we may run into duplicates. This means
	 * that doing a bin_srch for S->max can return an index that is too
	 * small.
	 *
	 * To get around this, we check how the ranges of S and s[i] overlap,
	 * and depending on how they overlap we either increment or do nothing.
	 * If there is no overlap, S->max may still be over the range of s[i],
	 * in which case we increment, just the same. Below are the 8 cases of
	 * overlap, and whether an increment happens:
	 *
	 *	1: S->max == s[i]->min.
	 *	2: S->max == s[i]->max == s[i]->min.
	 *
	 *	3: S->min == s[i]->max.
	 *	4: S->min == s[i]->max == s[i]->min.
	 *
	 *	5: S->max == S->min == s[i]->max.
	 *	6: S->max == S->min == s[i]->min.
	 *	7: S->max == S->min == s[i]->max == s[i]->min.
	 *
	 *	8: S->max >= S->min > s[i]->max
	 *
	 *	1: No Inc
	 *	2: No Inc
	 *	3: Inc
	 *	4: Inc
	 *	5: Inc
	 *	6: No Inc
	 *	7: Inc
	 *	8: Inc
	 */
	if (sorting && i < s->ss_elems) {
		slablist_elem_t max_ck;
		slablist_elem_t min_ck;
		slablist_elem_t max_cmp;
		slablist_elem_t min_cmp;
		if (s1 != NULL) {
			slab_t *si = GET_SUBSLAB_ELEM(s, i);
			max_ck = s1->s_max;
			min_ck = s1->s_min;
			min_cmp = si->s_min;
			max_cmp = si->s_max;
		} else if (s2 != NULL) {
			subslab_t *ssi = GET_SUBSLAB_ELEM(s, i);
			max_ck = s2->ss_max;
			min_ck = s2->ss_min;
			min_cmp = ssi->ss_min;
			max_cmp = ssi->ss_max;
		}

#define	EQ(x, y) (sl->sl_cmp_elem(x, y) == 0)

		int eq_max_ck_min_ck = EQ(max_ck, min_ck);
		int eq_min_ck_max_cmp = EQ(min_ck, max_cmp);
		int eq_max_cmp_min_cmp = EQ(max_cmp, min_cmp);
		/* cases 7, 5, 4, 3 and 8 */
		if ((eq_max_ck_min_ck && eq_min_ck_max_cmp &&
			eq_max_cmp_min_cmp) ||
		    (eq_max_ck_min_ck && eq_min_ck_max_cmp) ||
		    (eq_min_ck_max_cmp && eq_max_cmp_min_cmp) ||
		    (eq_min_ck_max_cmp && eq_max_cmp_min_cmp) ||
		    (eq_min_ck_max_cmp) ||
		    (sl->sl_bnd_elem(max_ck, min_cmp, max_cmp) > 0)) {
			i++;
		}

	}

	/*
	 * Test the consistency of the slab before addition.
	 */
	if (SLABLIST_TEST_ADD_SLAB_ENABLED() && s1 != NULL) {
		int f = test_add_slab(s, s1, s2, i);
		if (f) {
			SLABLIST_TEST_ADD_SLAB(f, s, s1, s2, i);
		}
	}


	int ip = 0;		/* add-point */
	slab_t *stmp_lst;
	// slab_t *stmp_fst;
	subslab_t *sstmp_lst;
	// subslab_t *sstmp_fst;

	ip = i;


	SLABLIST_SUBFWDSHIFT_BEGIN(s->ss_list, s, i);
	size_t shiftsz = (s->ss_elems - i) * 8;
	int ixi = i + 1;
	bcopy(&(GET_SUBSLAB_ELEM(s, i)), &(GET_SUBSLAB_ELEM(s, ixi)), shiftsz);

	slablist_elem_t max;
	slablist_elem_t min;

	if (s1 != NULL) {
		s1->s_below = s;
		SLABLIST_SLAB_SET_BELOW(s1);
		SET_SUBSLAB_ELEM(s, s1, i);
		max = s1->s_max;
		min = s1->s_min;
	} else {
		s2->ss_below = s;
		SLABLIST_SUBSLAB_SET_BELOW(s2);
		SET_SUBSLAB_ELEM(s, s2, i);
		max = s2->ss_max;
		min = s2->ss_min;
	}

	SLABLIST_SUBFWDSHIFT_END();

	/*
	 * If we added at the beginning of the slab, we have to change the
	 * minimum.
	 */
	if (ip == 0) {
		s->ss_min = min;
		SLABLIST_SUBSLAB_SET_MIN(s);
	}

	/*
	 * If we added at the end of the slab, we have to change the
	 * maximum.
	 */
	if (ip == (s->ss_elems)) {
		s->ss_max = max;
		SLABLIST_SUBSLAB_SET_MAX(s);
	}

	/*
	 * Same deal as the slab_t equivalent of this function. [add_elem()]
	 */
	if (s->ss_elems < SUBELEM_MAX) {
		s->ss_elems++;
		SLABLIST_SUBSLAB_INC_ELEMS(s);
	} else {
		if (s1 != NULL) {
			stmp_lst = (slab_t *)GET_SUBSLAB_ELEM(s,
			    SUBELEM_MAX - 1);
			s->ss_max = stmp_lst->s_max;
		} else {
			sstmp_lst = (subslab_t *)GET_SUBSLAB_ELEM(s,
			    SUBELEM_MAX - 1);
			s->ss_max = sstmp_lst->ss_max;
		}
		SLABLIST_SUBSLAB_SET_MAX(s);
	}

	if (SLABLIST_TEST_ADD_SLAB_ENABLED()) {
		int f = test_subslab_ref(s);
		SLABLIST_TEST_ADD_SLAB(f, s, s1, s2, i);
		f = test_subslab_extrema(s);
		SLABLIST_TEST_ADD_SLAB(f, s, s1, s2, i);
	}
}

/*
 * Inserts `elem` into `s`, which is a full slab, but moves the maximum element
 * into the slab next to `s`.
 */
static void
addsn(slab_t *s, slablist_elem_t elem, int q)
{
	slablist_elem_t lst_elem = s->s_arr[(s->s_elems - 1)];
	slablist_elem_t b4_lst_elem = s->s_arr[(s->s_elems - 2)];
	slab_t *snx = s->s_next;
	s->s_elems--;
	SLABLIST_SLAB_DEC_ELEMS(s);

	s->s_max = b4_lst_elem;
	SLABLIST_SLAB_SET_MAX(s);

	/*
	 * We have to adjust the value of `q` if it was originally set to be
	 * `s->s_elems` (i.e. after the last elem).
	 */
	if (q > s->s_elems) {
		q--;
	}
	add_elem(snx, lst_elem, 0);
	add_elem(s, elem, q);
}

/*
 * This function increments the ss_usr_elems of all of the subslabs below
 * `found`.
 */
static void
ripple_inc_usr_elems(subslab_t *found)
{
	subslab_t *q = found;
	while (q != NULL) {
		q->ss_usr_elems++;
		SLABLIST_SET_USR_ELEMS(q);
		q = q->ss_below;
	}
}


/*
 * Inserts slab `s1` or subslab `s2` into `s`, which is a full subslab, but
 * moves the maximum element into the subslab next to `s`.
 */
static subslab_t *
sub_addsn(subslab_t *s, slab_t *s1, subslab_t *s2, int mk)
{
	uint16_t lst_index = s->ss_elems - 1;
	uint16_t b4_lst_index = s->ss_elems - 2;
	slab_t *lst_slab = NULL;
	slab_t *b4_lst_slab = NULL;
	subslab_t *lst_subslab = NULL;
	subslab_t *b4_lst_subslab = NULL;
	uint64_t diff = 0;
	if (s1 != NULL) {
		lst_slab = GET_SUBSLAB_ELEM(s, lst_index);
		b4_lst_slab = GET_SUBSLAB_ELEM(s, b4_lst_index);
		s->ss_max = b4_lst_slab->s_max;
		diff = lst_slab->s_elems;
	} else {
		lst_subslab = GET_SUBSLAB_ELEM(s, lst_index);
		b4_lst_subslab = GET_SUBSLAB_ELEM(s, b4_lst_index);
		s->ss_max = b4_lst_subslab->ss_max;
		diff = lst_subslab->ss_usr_elems;
	}
	SLABLIST_SUBSLAB_SET_MAX(s);
	subslab_t *snx = s->ss_next;
	s->ss_elems--;
	SLABLIST_SUBSLAB_DEC_ELEMS(s);

	int k = 0;
	if (s1 != NULL) {
		k = subslab_bin_srch_top(s1->s_max, s);
	} else {
		k = subslab_bin_srch(s2->ss_max, s);
	}

	add_slab(snx, lst_slab, lst_subslab, 0);
	add_slab(s, s1, s2, k);
	subslab_t *p = s;
	subslab_t *q = snx;
	while (p != q) {
		p->ss_usr_elems -= diff;
		SLABLIST_SET_USR_ELEMS(p);
		q->ss_usr_elems += diff;
		SLABLIST_SET_USR_ELEMS(q);
		p = p->ss_below;
		q = q->ss_below;
	}
	return (p);
}

/*
 * Inserts `elem` into `s`, which is a full slab, but moves the minimum element
 * into the slab previous to `s`.
 */
static void
addsp(slab_t *s, slablist_elem_t elem, int q)
{
	/*
	 * Whenever this function gets called we assume the `s` is FULL.
	 */
	slablist_elem_t fst_elem = s->s_arr[0];
	slab_t *spv = s->s_prev;
	s->s_elems--;
	SLABLIST_SLAB_DEC_ELEMS(s);

	SLABLIST_BWDSHIFT_BEGIN(s->s_list, s, 1);
	bcopy(&(s->s_arr[1]), s->s_arr, ((SELEM_MAX - 1)*8));
	SLABLIST_BWDSHIFT_END();

	s->s_min = s->s_arr[0];
	SLABLIST_SLAB_SET_MIN(s);

	int j = 0;
	j = spv->s_elems;

	add_elem(spv, fst_elem, j);
	/*
	 * We calculated the position `q` before we did the bwdshift. To
	 * compensate for the changes to the array, we insert at position `q -
	 * 1` if `q` is not 0.
	 */
	if (q > 0) {
		q -= 1;
	}
	add_elem(s, elem, q);
}

/*
 * Inserts slab `s1` or subslab `s2` into `s`, which is a full slab, but moves
 * the minimum element into the slab previous to `s`.
 */
static subslab_t *
sub_addsp(subslab_t *s, slab_t *s1, subslab_t *s2, int mk)
{
	/*
	 * Whenever this function gets called we assume the `s` is FULL.
	 */
	slab_t *fst_slab = NULL;
	slab_t *new_fst_slab = NULL;
	subslab_t *fst_subslab = NULL;
	subslab_t *new_fst_subslab = NULL;
	if (s1 != NULL) {
		fst_slab = (slab_t *)(GET_SUBSLAB_ELEM(s, 0));
	} else {
		fst_subslab = (subslab_t *)(GET_SUBSLAB_ELEM(s, 0));
	}
	subslab_t *spv = s->ss_prev;


	SLABLIST_SUBBWDSHIFT_BEGIN(s->ss_list, s, 1);
	bcopy(&(GET_SUBSLAB_ELEM(s, 1)), &(GET_SUBSLAB_ELEM(s, 0)),
	    ((s->ss_elems - 1) * sizeof (void *)));
	SLABLIST_SUBBWDSHIFT_END();

	s->ss_elems--;
	SLABLIST_SUBSLAB_DEC_ELEMS(s);

	uint64_t diff = 0;
	if (s1 != NULL) {
		new_fst_slab = GET_SUBSLAB_ELEM(s, 0);
		s->ss_min = new_fst_slab->s_min;
		diff = fst_slab->s_elems;
	} else {
		new_fst_subslab = GET_SUBSLAB_ELEM(s, 0);
		s->ss_min = new_fst_subslab->ss_min;
		diff = fst_subslab->ss_usr_elems;
	}
	SLABLIST_SUBSLAB_SET_MIN(s);

	int j = 0;
	if (spv->ss_elems) {
		if (s1 != NULL) {
			j = subslab_bin_srch_top(fst_slab->s_max, spv);
		} else {
			j = subslab_bin_srch(fst_subslab->ss_max, spv);
		}
	}
	int k;
	if (s1 != NULL) {
		k = subslab_bin_srch_top(s1->s_max, s);
	} else {
		k = subslab_bin_srch(s2->ss_max, s);
	}

	add_slab(spv, fst_slab, fst_subslab, j);
	add_slab(s, s1, s2, k);
	subslab_t *p = s;
	subslab_t *q = spv;
	if (0 && p == q && s1 != NULL) {
		ripple_inc_usr_elems(s1->s_below);
		return (NULL);
	}
	while (p != q) {
		p->ss_usr_elems -= diff;
		SLABLIST_SET_USR_ELEMS(p);
		q->ss_usr_elems += diff;
		SLABLIST_SET_USR_ELEMS(q);
		p = p->ss_below;
		q = q->ss_below;
	}
	return (p);
}

static void
subslab_update_extrema(subslab_t *p)
{
	/*
	 * Sometimes, when calling `ripple_update_extrema` from the
	 * range-removal code, we pass an empty subslab to this function. We
	 * proceed with the extrema-update only if the subslab is not empty.
	 */
	if (p->ss_elems == 0) {
		return;
	}
	int last = p->ss_elems - 1;
	subslab_t *ssf;
	subslab_t *ssl;
	/* update extrema of `p` */
	if (p->ss_list->sl_layer == 1) {
		slab_t *f = GET_SUBSLAB_ELEM(p, 0);
		slab_t *l = GET_SUBSLAB_ELEM(p, last);
		p->ss_min = f->s_min;
		p->ss_max = l->s_max;
		SLABLIST_SUBSLAB_SET_MIN(p);
		SLABLIST_SUBSLAB_SET_MAX(p);
	} else {
		ssf = GET_SUBSLAB_ELEM(p, 0);
		ssl = GET_SUBSLAB_ELEM(p, last);
		p->ss_min = ssf->ss_min;
		p->ss_max = ssl->ss_max;
		SLABLIST_SUBSLAB_SET_MIN(p);
		SLABLIST_SUBSLAB_SET_MAX(p);
	}
}

void
ripple_update_extrema(slablist_t *sl, subslab_t *p)
{
	while (p != NULL) {
		subslab_update_extrema(p);
		if (p->ss_next != NULL) {
			subslab_update_extrema(p->ss_next);
		}
		if (p->ss_prev != NULL) {
			subslab_update_extrema(p->ss_prev);
		}
		p = p->ss_below;
	}
}


/* EASY */
void
ripple_ai(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	subslab_t *p = s->s_below;
	ripple_update_extrema(s->s_list, p);
	ripple_inc_usr_elems(s->s_below);
}

void
ripple_aa(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	subslab_t *p = s->s_below;
	ripple_update_extrema(s->s_list, p);
	ripple_inc_usr_elems(s->s_next->s_below);
}

void
ripple_ab(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	subslab_t *p = s->s_below;
	ripple_update_extrema(s->s_list, p);
	ripple_inc_usr_elems(s->s_prev->s_below);
}

void
ripple_aisn(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	subslab_t *p = s->s_next->s_below;
	ripple_update_extrema(s->s_list, p);
	ripple_inc_usr_elems(s->s_next->s_below);
}

void
ripple_aisp(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	subslab_t *p = s->s_prev->s_below;
	ripple_update_extrema(s->s_list, p);
	ripple_inc_usr_elems(s->s_prev->s_below);
}


/* fwd declaration for ripple funcs below */
static add_ctx_t *subslab_gen_add(int, slab_t *, subslab_t *, subslab_t *);

void
ripple_common(slab_t *s, subslab_t *p, slab_t *n, subslab_t *nn)
{
	add_ctx_t *t = NULL;
	int status;
	int broke = 0;
	int skip_loop = 1;
	slablist_t *sl = s->s_list;
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);
	/*
	 * We update the extrema.
	 */
	ripple_update_extrema(s->s_list, p);
	if (n->s_below != NULL) {
		status = sl->sl_bnd_elem(n->s_min, n->s_below->ss_min,
				n->s_below->ss_max);
		if (sorting && status == FS_IN_RANGE) {
			if (sl->sl_cmp_elem(n->s_min,
			    n->s_below->ss_max) == 0) {
				status  = FS_OVER_RANGE;
			}
		}
		t = subslab_gen_add(status, n, nn, n->s_below);
		while (t->ac_subslab_new != NULL) {
			skip_loop = 0;
			nn = t->ac_subslab_new;
			rm_add_ctx(t);
			/*
			 * If there is no subslab, there is no point in
			 * continueing.
			 */
			if (nn->ss_below == NULL) {
				broke = 1;
				break;
			}
			status = sl->sl_bnd_elem(nn->ss_min,
			    nn->ss_below->ss_min, nn->ss_below->ss_max);
			t = subslab_gen_add(status, NULL, nn, nn->ss_below);
		}
		/*
		 * We update the extrema again (who knows what may have
		 * changed).
		 */
		ripple_update_extrema(s->s_list, p);
		ripple_inc_usr_elems(n->s_below);
		if (!broke || skip_loop) {
			rm_add_ctx(t);
		}
	}
}

void
ripple_aam(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	slab_t *n = s->s_next; /* new slab */
	subslab_t *nn = NULL; /* new subslab */
	subslab_t *p = s->s_next->s_below;
	ripple_common(s, p, n, nn);
}

void
ripple_abm(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	slab_t *n = s->s_prev; /* new slab */
	subslab_t *nn = NULL; /* new subslab */
	subslab_t *p = s->s_prev->s_below;
	ripple_common(s, p, n, nn);
}

void
ripple_aisnm(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	slab_t *n = s->s_next; /* new slab */
	subslab_t *nn = NULL; /* new subslab */
	subslab_t *p = s->s_next->s_below;
	ripple_common(s, p, n, nn);
}

void
ripple_aispm(slab_t *s)
{
	if (s->s_below == NULL) {
		return;
	}
	slab_t *n = s->s_prev; /* new slab */
	subslab_t *nn = NULL; /* new subslab */
	subslab_t *p = s->s_prev->s_below;
	ripple_common(s, p, n, nn);
}

/*
 * Assuming that `elem` is in the range of `s`, we add into `s`. If `rep` is
 * not 0, we can replace an elem that has the same key as `elem` with `elem`.
 * The replaced elem is saved in `repd_elem`. If we can't replace but find a
 * duplicate we notify the caller, by returning an add_ctx_t with an EDUP
 * error.
 */
static add_ctx_t *
gen_add_ira(slablist_t *sl, slab_t *s, slablist_elem_t elem, int rep)
{
	/*
	 * If we are adding into a subslab, then we are adding an
	 * `elem` that is a pointer to a slab.
	 */
	slab_t *ns = NULL;
	add_ctx_t *ctx = mk_add_ctx();
	int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);

	int i = slab_bin_srch(elem, s);
	if (!sorting && !rep && sl->sl_cmp_elem(elem, s->s_arr[i]) == 0) {
		SLABLIST_SLAB_AR(sl, NULL, elem, 0);
		ctx->ac_how = AC_HOW_EDUP;
		return (ctx);
	}
	/*
	 * If the slab `s` is not full, or if there is the possibility of
	 * replacing an existing element we try to add into the slab. On the
	 * other hand, if the slab is full, we have to try to add the elem
	 * into the slab `s` while moving elems between the adjacent slabs.
	 */
	if (s->s_elems < SELEM_MAX || rep) {
		/*
		 * If this slablist is being used as an intermediary for
		 * sorting an unsorted slab list, we have to adjust for the
		 * possiblity of duplicates. The bin srch could have dumped us
		 * in the middle of a run of duplicates. We try to get to the
		 * end of the run of duplicates. We move i to the element that
		 * is either _after_ the run or at the end of the slab. We know
		 * for a fact that this slab is the _LAST_ slab that contains
		 * the duplicates (see bubble up and linear scan code).
		 */
		if (sorting) {
			goto skip_rep;
		}
		if (sl->sl_cmp_elem(s->s_arr[i], elem) == 0) {
			ctx->ac_repd_elem = s->s_arr[i];
			ctx->ac_how = AC_HOW_INTO;
			s->s_arr[i] = elem;
			SLABLIST_SLAB_AR(sl, NULL, elem, 1);
			return (ctx);
		}
skip_rep:;
		SLABLIST_SLAB_AI(sl, s, elem);
		add_elem(s, elem, i);
		ripple_ai(s);
		ctx->ac_how = AC_HOW_INTO;

	} else {
		/*
		 * If we are inserting into a slablist that is temporarily used
		 * for sorting, and if we are inserting an element that is a
		 * duplicate of the _last_ element in the slab, we have to swap
		 * `elem` with the last element in the slab, before we
		 * continue. If we don't, the last element will be moved to
		 * `s->s_next` and `elem` will go into `s`. This would make the
		 * sort unstable, which is undesireable.
		 *
		 * For Illustration:
		 *
		 *	Insert E
		 *	E = X
		 *	[X X X X X Y Y Y] [Y Y Y ...
		 *	[X X X X X E Y Y] [Y Y Y Y ...
		 *	   No problems here
		 *
		 *	E = X
		 *	[X X X X X X X X] [Y Y Y ...
		 *	[X X X X X X X E] [X Y Y Y...
		 *	   E comes before X, making the sort unstable.
		 *
		 * So we just have to do this:
		 *	Insert E
		 *	E = X
		 *	[X X X X X X X X] [Y Y Y ...
		 *	Swap E with the Last X
		 *	Insert X
		 *	[X X X X X X X E] [Y Y Y...
		 *	[X X X X X X X X] [E Y Y Y...
		 *	   E comes after X, making the sort stable.
		 */
		if (sorting &&
		    sl->sl_cmp_elem(elem, s->s_arr[i]) == 0) {
			slablist_elem_t tmp = elem;
			elem = s->s_arr[i];
			s->s_arr[i] = tmp;
		}
		slab_t *snx = s->s_next;
		slab_t *spv = s->s_prev;
		if (snx != NULL && snx->s_elems < SELEM_MAX) {
			SLABLIST_SLAB_AISN(sl, s, elem);
			addsn(s, elem, i);
			ripple_aisn(s);
			ctx->ac_how = AC_HOW_SP_NX;
			return (ctx);
		}
		if (spv != NULL && spv->s_elems < SELEM_MAX) {
			SLABLIST_SLAB_AISP(sl, s, elem);
			addsp(s, elem, i);
			ripple_aisp(s);
			ctx->ac_how = AC_HOW_SP_PV;
			return (ctx);
		}
		if (snx == NULL || snx->s_elems == SELEM_MAX) {
			SLABLIST_SLAB_AISNM(sl, s, elem);
			ns = mk_slab();
			SLABLIST_SLAB_MK(sl);
			link_slab(ns, s, SLAB_LINK_AFTER);
			addsn(s, elem, i);
			ripple_aisnm(s);
			ctx->ac_how = AC_HOW_SP_NX;
			ctx->ac_slab_new = ns;
			return (ctx);
		}
		if (spv == NULL || spv->s_elems == SELEM_MAX) {
			SLABLIST_SLAB_AISPM(sl, s, elem);
			ns = mk_slab();
			SLABLIST_SLAB_MK(sl);
			link_slab(ns, s, SLAB_LINK_BEFORE);
			addsp(s, elem, i);
			ripple_aispm(s);
			ctx->ac_how = AC_HOW_SP_PV;
			ctx->ac_slab_new = ns;
			return (ctx);
		}
	}

	return (ctx);
}

static add_ctx_t *
sub_gen_add_ira(slablist_t *sl, subslab_t *s, slab_t *s1, subslab_t *s2)
{
	/*
	 * If we are adding into a subslab, then we are adding an
	 * `elem` that is a pointer to a slab.
	 */
	subslab_t *ns = NULL;
	add_ctx_t *ctx = mk_add_ctx();
	// int sorting = SLIST_IS_SORTING_TEMP(sl->sl_flags);

	int i = 0;
	/*
	 * If the slab `s` is not full, or if there is the possibility of
	 * replacing an existing element we try to add into the slab. On the
	 * other hand, if the slab is full, we have to try to add the elem
	 * into the slab `s` while moving elems between the adjacent slabs.
	 */
	if (s->ss_elems < SUBELEM_MAX) {

		if (s1 != NULL) {
			i = subslab_bin_srch_top(s1->s_max, s);
		} else {
			i = subslab_bin_srch(s2->ss_max, s);
		}

		SLABLIST_SUBSLAB_AI(sl, s, s1, s2);
		add_slab(s, s1, s2, i);
		ctx->ac_how = AC_HOW_INTO;
	} else {

		subslab_t *snx = s->ss_next;
		subslab_t *spv = s->ss_prev;
		subslab_t *common = NULL;
		if (snx != NULL && snx->ss_elems < SUBELEM_MAX) {
			SLABLIST_SUBSLAB_AISN(sl, s, s1, s2);
			common = sub_addsn(s, s1, s2, 0);
			ctx->ac_how = AC_HOW_SP_NX;
			ctx->ac_subslab_common = common;
			return (ctx);
		}
		if (spv != NULL && spv->ss_elems < SUBELEM_MAX) {
			SLABLIST_SUBSLAB_AISP(sl, s, s1, s2);
			common = sub_addsp(s, s1, s2, 0);
			ctx->ac_how = AC_HOW_SP_PV;
			ctx->ac_subslab_common = common;
			return (ctx);
		}
		if (snx == NULL || snx->ss_elems == SUBELEM_MAX) {
			SLABLIST_SUBSLAB_AISNM(sl, s, s1, s2);
			ns = mk_subslab();
			ns->ss_arr = mk_subarr();
			SLABLIST_SUBSLAB_MK(sl);
			link_subslab(ns, s, SLAB_LINK_AFTER);
			common = sub_addsn(s, s1, s2, 1);
			ctx->ac_how = AC_HOW_SP_NX;
			ctx->ac_subslab_new = ns;
			ctx->ac_subslab_common = common;
			return (ctx);
		}
		if (spv == NULL || spv->ss_elems == SUBELEM_MAX) {
			SLABLIST_SUBSLAB_AISPM(sl, s, s1, s2);
			ns = mk_subslab();
			ns->ss_arr = mk_subarr();
			SLABLIST_SUBSLAB_MK(sl);
			link_subslab(ns, s, SLAB_LINK_BEFORE);
			common = sub_addsp(s, s1, s2, 1);
			ctx->ac_how = AC_HOW_SP_PV;
			ctx->ac_subslab_new = ns;
			ctx->ac_subslab_common = common;
			return (ctx);
		}
	}
	return (ctx);
}

/*
 * Assuming that `elem` is under the range of `s`, we have the following
 * possible way of adding `elem` into the slablist.
 *
 * We can:
 *
 *	Insert into `s` if not full.
 *	Insert into `s->s_prev` if not full.
 *	Spill max of `s` into next, add into `s`.
 *	Create new previous slab, add into that.
 */
static add_ctx_t *
gen_add_ura(slablist_t *sl, slab_t *s, slablist_elem_t elem)
{
	/*
	 * If we are adding into a subslab, then we are adding an
	 * `elem` that is a pointer to a slab.
	 */
	int i = slab_bin_srch(elem, s);
	slab_t *ns = NULL;
	add_ctx_t *ctx = mk_add_ctx();
	if (s->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_AI(sl, s, elem);
		add_elem(s, elem, i);
		ripple_ai(s);
		ctx->ac_how = AC_HOW_INTO;
		return (ctx);
	}
	if (s->s_prev != NULL && s->s_prev->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_AB(sl, s, elem);
		i = slab_bin_srch(elem, s->s_prev);
		add_elem(s->s_prev, elem, i);
		ripple_ab(s);
		ctx->ac_how = AC_HOW_BEFORE;
		return (ctx);
	}
	if (s->s_next != NULL && s->s_next->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_AISN(sl, s, elem);
		addsn(s, elem, i);
		ripple_aisn(s);
		ctx->ac_how = AC_HOW_SP_NX;
		return (ctx);
	}
	SLABLIST_SLAB_ABM(sl, s, elem);
	ns = mk_slab();
	SLABLIST_SLAB_MK(sl);
	link_slab(ns, s, SLAB_LINK_BEFORE);
	add_elem(s->s_prev, elem, 0);
	ripple_abm(s);
	ctx->ac_how = AC_HOW_BEFORE;
	ctx->ac_slab_new = ns;
	return (ctx);
}

static add_ctx_t *
sub_gen_add_ura(slablist_t *sl, subslab_t *s, slab_t *s1, subslab_t *s2)
{
	/*
	 * If we are adding into a subslab, then we are adding a slab or
	 * subslab pointer.
	 */
	int i;
	add_ctx_t *ctx = mk_add_ctx();

	if (s1 != NULL) {
		i = subslab_bin_srch_top(s1->s_max, s);
	} else {
		i = subslab_bin_srch(s2->ss_max, s);
	}
	subslab_t *ns = NULL;
	if (s->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_AI(sl, s, s1, s2);
		add_slab(s, s1, s2, i);
		ctx->ac_how = AC_HOW_INTO;
		return (ctx);
	}
	if (s->ss_prev != NULL && s->ss_prev->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_AB(sl, s, s1, s2);
		if (s1 != NULL) {
			i = subslab_bin_srch_top(s1->s_max, s->ss_prev);
		} else {
			i = subslab_bin_srch(s2->ss_max, s->ss_prev);
		}
		add_slab(s->ss_prev, s1, s2, i);
		ctx->ac_how = AC_HOW_BEFORE;
		return (ctx);
	}
	subslab_t *common = NULL;
	if (s->ss_next != NULL && s->ss_next->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_AISN(sl, s, s1, s2);
		common = sub_addsn(s, s1, s2, 0);
		ctx->ac_how = AC_HOW_SP_NX;
		ctx->ac_subslab_common = common;
		return (ctx);
	}
	SLABLIST_SUBSLAB_ABM(sl, s, s1, s2);
	ns = mk_subslab();
	ns->ss_arr = mk_subarr();
	SLABLIST_SUBSLAB_MK(sl);
	link_subslab(ns, s, SLAB_LINK_BEFORE);
	add_slab(s->ss_prev, s1, s2, 0);
	ctx->ac_how = AC_HOW_BEFORE;
	ctx->ac_subslab_new = ns;
	return (ctx);
}

/*
 * Assuming that `elem` is under the range of `s`, we have the following
 * possible way of adding `elem` into the slablist.
 *
 * We can:
 *
 *	Insert into `s` if not full.
 *	Insert into `s->s_next` if not full.
 *	Spill min of `s` into prev, add into `s`.
 *	Create new next slab, add into that.
 */
static add_ctx_t *
gen_add_ora(slablist_t *sl, slab_t *s, slablist_elem_t elem)
{
	int i = slab_bin_srch(elem, s);
	slab_t *ns = NULL;
	add_ctx_t *ctx = mk_add_ctx();

	if (s->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_AI(sl, s, elem);
		add_elem(s, elem, i);
		ripple_ai(s);
		ctx->ac_how = AC_HOW_INTO;
		return (ctx);
	}
	if (s->s_next != NULL && s->s_next->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_AA(sl, s, elem);
		i = slab_bin_srch(elem, s->s_next);
		add_elem(s->s_next, elem, i);
		ripple_aa(s);
		ctx->ac_how = AC_HOW_AFTER;
		return (ctx);
	}
	if (s->s_prev != NULL && s->s_prev->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_AISP(sl, s, elem);
		addsp(s, elem, i);
		ripple_aisp(s);
		ctx->ac_how = AC_HOW_SP_PV;
		return (ctx);
	}
	SLABLIST_SLAB_AAM(sl, s, elem);
	ns = mk_slab();
	SLABLIST_SLAB_MK(sl);
	link_slab(ns, s, SLAB_LINK_AFTER);
	add_elem(s->s_next, elem, 0);
	ripple_aam(s);
	ctx->ac_how = AC_HOW_AFTER;
	ctx->ac_slab_new = ns;
	return (ctx);
}

static add_ctx_t *
sub_gen_add_ora(slablist_t *sl, subslab_t *s, slab_t *s1, subslab_t *s2)
{
	int i = 0;
	add_ctx_t *ctx = mk_add_ctx();

	if (s1 != NULL) {
		i = subslab_bin_srch_top(s1->s_max, s);
	} else {
		i = subslab_bin_srch(s2->ss_max, s);
	}
	subslab_t *ns = NULL;
	if (s->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_AI(sl, s, s1, s2);
		add_slab(s, s1, s2, i);
		ctx->ac_how = AC_HOW_INTO;
		return (ctx);
	}
	if (s->ss_next != NULL && s->ss_next->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_AA(sl, s, s1, s2);
		if (s1 != NULL) {
			i = subslab_bin_srch_top(s1->s_max, s->ss_next);
		} else {
			i = subslab_bin_srch(s2->ss_max, s->ss_next);
		}
		add_slab(s->ss_next, s1, s2, i);
		ctx->ac_how = AC_HOW_AFTER;
		return (ctx);
	}
	subslab_t *common = NULL;
	if (s->ss_prev != NULL && s->ss_prev->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_AISP(sl, s, s1, s2);
		common = sub_addsp(s, s1, s2, 0);
		ctx->ac_how = AC_HOW_SP_PV;
		ctx->ac_subslab_common = common;
		return (ctx);
	}
	SLABLIST_SUBSLAB_AAM(sl, s, s1, s2);
	ns = mk_subslab();
	ns->ss_arr = mk_subarr();
	SLABLIST_SUBSLAB_MK(sl);
	link_subslab(ns, s, SLAB_LINK_AFTER);
	add_slab(s->ss_next, s1, s2, 0);
	ctx->ac_how = AC_HOW_AFTER;
	ctx->ac_subslab_new = ns;
	return (ctx);
}

/*
 * This procedure is invoked after a slab is found. Given the status it finds a
 * way to add elem into the slablist. It adds the element into a slab of
 * appropriate range, doing this may require changing the range of the slab,
 * creating new adjacent slabs, or shifting the min/max of the current slab
 * into adjacent slabs. Or it creates a new slab with the appropriate range.
 * All of those possibilities depend on whether a slab with appropriate range
 * was found, and on whether the slab (and the slabs adjacent to it) are at
 * capacity. See the above three functions for more details.
 */
static add_ctx_t *
slab_gen_add(int status, slablist_elem_t elem, slab_t *s, int rep)
{
	add_ctx_t *ctx = NULL;
	slablist_t *sl = s->s_list;

	if (status == FS_IN_RANGE) {
		ctx = gen_add_ira(sl, s, elem, rep);
		return (ctx);
	}

	if (status == FS_OVER_RANGE) {
		ctx = gen_add_ora(sl, s, elem);
		return (ctx);
	}

	if (status == FS_UNDER_RANGE) {
		ctx = gen_add_ura(sl, s, elem);
		return (ctx);
	}

	return (ctx);
}

static add_ctx_t *
subslab_gen_add(int status, slab_t *s1, subslab_t *s2, subslab_t *s)
{
	slablist_t *sl = s->ss_list;
	add_ctx_t *ctx = NULL;

	if (status == FS_IN_RANGE) {
		ctx = sub_gen_add_ira(sl, s, s1, s2);
		return (ctx);
	}

	if (status == FS_OVER_RANGE) {
		ctx = sub_gen_add_ora(sl, s, s1, s2);
		return (ctx);
	}

	if (status == FS_UNDER_RANGE) {
		ctx = sub_gen_add_ura(sl, s, s1, s2);
		return (ctx);
	}

	return (ctx);
}

/*
 * This function adds an element to a slablist. `rep` indicates if an
 * already-added element with an identical key should to be replaced.
 * This function is an entry point into libslablist.
 */
int
slablist_add_impl(slablist_t *sl, slablist_elem_t elem, int rep)
{


	int ret;
	/*
	 * The number of elements is too small to justify the use of
	 * slabs. So we store the data in a singly linked list.
	 */
	if (IS_SMALL_LIST(sl) && sl->sl_elems <= (SMELEM_MAX - 1)) {
		SLABLIST_ADD_BEGIN(sl, elem, rep);
		ret = small_list_add(sl, elem, 0,  NULL);
		SLABLIST_ADD_END(ret);
		return (ret);
	}

	/*
	 * If the number of elems has grown to an acceptable level, we turn the
	 * list into a slab.
	 */
	if (IS_SMALL_LIST(sl) && sl->sl_elems == SMELEM_MAX) {
		small_list_to_slab(sl);
	}



	slab_t *s;

	int edup = 0;

	if (SLIST_SORTED(sl->sl_flags)) {
		/*
		 * If the slablist is sorted, we find the slab with the
		 * appropriate range and place the element there.
		 */

		if (SLABLIST_TEST_IS_SLAB_LIST_ENABLED()) {
			SLABLIST_TEST_IS_SLAB_LIST(IS_SMALL_LIST(sl));
		}

		SLABLIST_ADD_BEGIN(sl, elem, rep);

		int fs;
		slab_t *found;
		/*
		 * If we have sublayers, we do a binary search from the
		 * base-slab to the candidate topslab. Otherwise, we just do a
		 * linear search on our slab list. See the find_bubble_up()
		 * implementation in slablist_find.c for details.
		 */
		if (sl->sl_sublayers) {
			fs = find_bubble_up(sl, elem, &found);
			s = found;
		} else {
			fs = find_linear_scan(sl, elem, &s);
		}

		add_ctx_t *ctx = slab_gen_add(fs, elem, s, rep);
		if (ctx->ac_how == AC_HOW_EDUP) {
			edup++;
		}
		rm_add_ctx(ctx);

		slablist_t *usl = NULL;

		if (sl->sl_sublayer == NULL) {
			usl = sl;
		} else {
			usl = sl->sl_baselayer;
		}

		/*
		 * If we have sl_req_sublayer slabs at the baselayer (or, if we
		 * have no sublayers, in the slablist in general) we map the
		 * slabs in the slablist/baselayer to a newly created
		 * baselayer.
		 */
		if (sl->sl_req_sublayer &&
		    usl->sl_slabs >= sl->sl_req_sublayer) {
			attach_sublayer(usl);
		}


	} else {

		SLABLIST_ADD_BEGIN(sl, elem, rep);
		/*
		 * If the slablist is ordered, we place the element at the end
		 * of the list which is at the end of the last slab.
		 */
		s = (slab_t *)sl->sl_end;

		if (s->s_elems < SELEM_MAX) {
			s->s_arr[s->s_elems] = elem;
			s->s_max = elem;
			s->s_elems++;
			SLABLIST_SET_END(sl, elem);
			SLABLIST_SLAB_INC_ELEMS(s);
		} else {
			slab_t *ns = mk_slab();
			SLABLIST_SLAB_MK(sl);
			ns->s_arr[0] = elem;
			ns->s_min = elem;
			ns->s_max = elem;
			link_slab(ns, s, SLAB_LINK_AFTER);
			ns->s_elems++;
			SLABLIST_SLAB_INC_ELEMS(ns);
		}
	}

	try_reap_all(sl);

	if (edup) {
		ret = SL_EDUP;
	} else {
		sl->sl_elems++;
		SLABLIST_SL_INC_ELEMS(sl);
		ret = SL_SUCCESS;
	}


	SLABLIST_ADD_END(ret);
	return (ret);

}

/*
 * This function is the interface function. We sometimes use the
 * slablist_add_impl() internally, and would like to distinguish between
 * user-induced calls and library-induced calls, when tracing.
 */
int
slablist_add(slablist_t *sl, slablist_elem_t elem, int rep)
{
	int ret = slablist_add_impl(sl, elem, rep);
	return (ret);
}

/*
 * This function takes an unsorted slab list and sorts it. To do so, it uses
 * the sorted slab list as its sorting algorithm. It drains elements from `sl`
 * one by one and inserts them into `tmp`. When `sl` is empty, the function
 * transplants the head slab/node from `tmp` to `sl`. Then it destroys the
 * remains of `tmp`. `tmp` has a sepcial flag changes parts of the insertion
 * path to tolerate duplicate elements.
 */
int
slablist_sort(slablist_t *sl, slablist_cmp_t cmp, slablist_bnd_t bnd)
{
	/*
	 * We create a special kind of sorted slab list that we will use to
	 * sort the elements in `sl`.
	 */
	slablist_t *tmp = slablist_create("temp_sorting", cmp,
	    bnd, SL_SORTED);
	SLIST_SET_SORTING_TEMP(tmp->sl_flags);
	uint64_t i = 0;
	uint64_t j = 0;
	if (IS_SMALL_LIST(sl)) {
		small_list_t *prev_node = NULL;
		small_list_t *node = sl->sl_head;
		while (i < sl->sl_elems) {
			slablist_add_impl(tmp, node->sml_data, 0);
			prev_node = node;
			node = node->sml_next;
			rm_sml_node(prev_node);
			i++;
		}
	} else {
		slab_t *prev_slab = NULL;
		slab_t *slab = sl->sl_head;
		while (i < sl->sl_slabs) {
			j = 0;
			while (j < slab->s_elems) {
				slablist_add_impl(tmp, slab->s_arr[j], 0);
				j++;
			}
			prev_slab = slab;
			slab = slab->s_next;
			rm_slab(prev_slab);
			i++;
		}
		sl->sl_head = tmp->sl_head;
	}
	tmp->sl_head = NULL;
	slablist_destroy(tmp);
	return (SL_SUCCESS);
}

/*
 * This function takes an ordered slablist and reverses the order of elements,
 * in-place.
 */
void
slablist_reverse(slablist_t *sl)
{
	if (SLIST_SORTED(sl->sl_flags)) {
		return;
	}
	void *head = sl->sl_head;
	sl->sl_head = sl->sl_end;
	sl->sl_end = head;
	if (IS_SMALL_LIST(sl)) {
		small_list_t *n = head;
		small_list_t *n_tmp;
		small_list_t *n_prev = NULL;
		while (n != NULL) {
			n_tmp = n->sml_next;
			n->sml_next = n_prev;
			n_prev = n;
			n = n_tmp;
		}
		return;
	}
	slab_t *s = head;
	slab_t *tmp;
	while (s != NULL) {
		tmp = s->s_next;
		s->s_next = s->s_prev;
		s->s_prev = tmp;
		uint8_t i = 0;
		uint8_t j = s->s_elems - 1;
		slablist_elem_t t;
		while (i < j) {
			t = s->s_arr[i];
			s->s_arr[i] = s->s_arr[j];
			s->s_arr[j] = t;
			i++;
			j--;
		}
		s = tmp;
	}
}
