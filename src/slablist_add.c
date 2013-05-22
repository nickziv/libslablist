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
 * This file implements the procedures that insert elements into Slab Lists.
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
small_list_add(slablist_t *sl, uintptr_t elem, int rep, uintptr_t *repd_elem)
{
	lock_list(sl);

	int ret;
	if (sl->sl_head == NULL) {
		/*
		 * If the ptr to the head of this slablist is null, we have
		 * to create the head node, store `elem` into it, update
		 * metadata, and return.
		 */
		sl->sl_head = mk_sml_node();
		SLABLIST_ADD_HEAD(sl);
		((small_list_t *)sl->sl_head)->sml_data = elem;
		sl->sl_elems++;
		SLABLIST_SL_INC_ELEMS(sl);
		ret = SL_SUCCESS;
		goto end;
	}

	small_list_t *sml;
	small_list_t *nsml = NULL;

	if (SLIST_SORTED(sl->sl_flags)) {
		/*
		 * We place the element in the position so that it is less than
		 * the element after it.
		 */
		sml = sl->sl_head;
		small_list_t *prev = NULL;
		uint64_t i = 0;
		while (i < sl->sl_elems) {
			if (sl->sl_cmp_elem(elem, sml->sml_data) < 0) {
				/*
				 * If `elem` is less than the data in the
				 * current sml_node, we insert `elem` before
				 * it.
				 */
				nsml = mk_sml_node();
				nsml->sml_data = elem;
				link_sml_node(sl, prev, nsml);
				ret = SL_SUCCESS;
				goto end;
			}
			if (sl->sl_cmp_elem(elem, sml->sml_data) == 0) {
				/*
				 * If `elem` is equal to the current sml_node,
				 * we either replace its data with `elem` or,
				 * we error out depending on the user's
				 * preference.
				 */
				if (rep) {
					if (repd_elem != NULL) {
						*repd_elem = sml->sml_data;
					}
					sml->sml_data = elem;
					SLABLIST_SLAB_ADD_REPLACE(sl, NULL, elem, 1);
				} else {
					/*
					 * We don't want to insert duplicates.
					 */
					SLABLIST_SLAB_ADD_REPLACE(sl, NULL, elem, 0);
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

		if (sml == NULL) {
			/*
			 * we've reached the end of the list, and make elem the
			 * last element.
			 */
			nsml = mk_sml_node();
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

end:;

	/*
	 * Now that we have modifies the list, we run some tests, if DTrace has
	 * enabled those probes.
	 */
	if (SLABLIST_TEST_IS_SML_LIST_ENABLED()) {
		SLABLIST_TEST_IS_SML_LIST(!(sl->sl_is_small_list));
	}

	if (SLABLIST_TEST_SMLIST_ELEMS_SORTED_ENABLED()) {
		/*
		 * If test probe is enabled, we verify that the elems
		 * are sorted.
		 */
		int f = test_smlist_elems_sorted(sl);
		SLABLIST_TEST_SMLIST_ELEMS_SORTED(f);
	}

	unlock_list(sl);

	return (ret);
}

/*
 * Inserts an `elem` to insert into slab `s` at index `i`.
 */
static void
insert_elem(slab_t *s, uintptr_t elem, uint64_t i)
{
	/*
	 * Test the consistency of the slab before insertion.
	 */
	if (SLABLIST_TEST_INSERT_ELEM_ENABLED()) {
		int f = test_insert_elem(s, elem, i);
		if (f) {
			SLABLIST_TEST_INSERT_ELEM(f, s, elem, i);
		}
	}

	int ip = 0;		/* insert-point */
	slablist_t *sl;
	sl = s->s_list;

	ip = i;

	SLABLIST_FWDSHIFT_BEGIN(s->s_list, s, i);
	size_t shiftsz = (s->s_elems - i) * 8;
	bcopy(&(s->s_arr[i]), &(s->s_arr[(i+1)]), shiftsz);
	s->s_arr[i] = elem;
	SLABLIST_FWDSHIFT_END();

	/*
	 * If we inserted at the beginning of the slab, we have to change the
	 * minimum.
	 */
	if (ip == 0) {
		s->s_min = s->s_arr[0];
		SLABLIST_SLAB_SET_MIN(s);
	}

	/*
	 * If we inserted at the end of the slab, we have to change the
	 * maximum.
	 */
	if (ip == (s->s_elems)) {
		s->s_max = s->s_arr[(s->s_elems)];
		SLABLIST_SLAB_SET_MAX(s);
	}

	if (s->s_elems < SELEM_MAX) {
		/*
		 * Sometimes we are inserting into a slab that is full, with
		 * the intention of moving the last element into the next slab
		 * (which is not full). In these situations, we don't want to
		 * increment anything, as all the neccessary increments will
		 * occur when we move the elem into the next slab (i.e. the
		 * next time we call this function).
		 */
		s->s_elems++;
		SLABLIST_SLAB_INC_ELEMS(s);
	} else {
		/*
		 * We've over-written the old max (with the intention of
		 * inserting it into the next slab). We have to update the max,
		 * as a result.
		 */
		s->s_max = s->s_arr[(SELEM_MAX - 1)];
		s->s_min = s->s_arr[0];
		SLABLIST_SLAB_SET_MAX(s);
		SLABLIST_SLAB_SET_MIN(s);
	}

	if (SLABLIST_TEST_INSERT_ELEM_ENABLED()) {
		int f = test_slab_extrema(s);
		if (f) {
			SLABLIST_TEST_INSERT_ELEM(f, s, elem, i);
		}
	}
}

/*
 * This function inserts either slab `s1` or subslab `s2` into subslab `s`, at
 * index `i`.
 */
static void
insert_slab(subslab_t *s, slab_t *s1, subslab_t *s2, uint64_t i)
{
	/*
	 * Test the consistency of the slab before insertion.
	 */
	if (SLABLIST_TEST_INSERT_SLAB_ENABLED() && s1 != NULL) {
		int f = test_insert_slab(s, s1, s2, i);
		if (f) {
			SLABLIST_TEST_INSERT_SLAB(f, s, s1, s2, i);
		}
	}


	int ip = 0;		/* insert-point */
	slablist_t *sl;
	sl = s->ss_list;
	slab_t *stmp_lst;
	slab_t *stmp_fst;
	subslab_t *sstmp_lst;
	subslab_t *sstmp_fst;

	ip = i;

	SLABLIST_SUBFWDSHIFT_BEGIN(s->ss_list, s, i);
	size_t shiftsz = (s->ss_elems - i) * 8;
	int ixi = i + 1;
	bcopy(&(GET_SUBSLAB_ELEM(s, i)), &(GET_SUBSLAB_ELEM(s, ixi)), shiftsz);

	uintptr_t max;
	uintptr_t min;

	if (s1 != NULL) {
		SET_SUBSLAB_ELEM(s, s1, i);
		max = s1->s_max;
		min = s1->s_min;
	} else {
		SET_SUBSLAB_ELEM(s, s2, i);
		max = s2->ss_max;
		min = s2->ss_min;
	}

	SLABLIST_SUBFWDSHIFT_END();

	/*
	 * If we inserted at the beginning of the slab, we have to change the
	 * minimum.
	 */
	if (ip == 0) {
		s->ss_min = min;
		SLABLIST_SUBSLAB_SET_MIN(s);
	}

	/*
	 * If we inserted at the end of the slab, we have to change the
	 * maximum.
	 */
	if (ip == (s->ss_elems)) {
		s->ss_max = max;
		SLABLIST_SUBSLAB_SET_MAX(s);
	}

	if (s->ss_elems < SUBELEM_MAX) {
		/*
		 * Sometimes we are inserting into a slab that is full, with
		 * the intention of moving the last element into the next slab
		 * (which is not full). In these situations, we don't want to
		 * increment anything, as all the neccessary increments will
		 * occur when we move the elem into the next slab (i.e. the
		 * next time we call this function).
		 */
		s->ss_elems++;
		SLABLIST_SUBSLAB_INC_ELEMS(s);
	} else {
		/*
		 * We've over-written the old max (with the intention of
		 * inserting it into the next slab). We have to update the max,
		 * as a result.
		 */
		if (s1 != NULL) {
			stmp_lst = (slab_t *)GET_SUBSLAB_ELEM(s, SUBELEM_MAX - 1);
			stmp_fst = (slab_t *)GET_SUBSLAB_ELEM(s, 0);
			s->ss_max = stmp_lst->s_max;
			s->ss_min = stmp_lst->s_min;
		} else {
			sstmp_lst = (subslab_t *)GET_SUBSLAB_ELEM(s, SUBELEM_MAX - 1);
			sstmp_fst = (subslab_t *)GET_SUBSLAB_ELEM(s, 0);
			s->ss_max = sstmp_lst->ss_max;
			s->ss_min = sstmp_lst->ss_min;
		}
		SLABLIST_SUBSLAB_SET_MAX(s);
		SLABLIST_SUBSLAB_SET_MIN(s);
	}

	if (SLABLIST_TEST_INSERT_SLAB_ENABLED()) {
		int f = test_subslab_ref(s);
		SLABLIST_TEST_INSERT_SLAB(f, s, s1, s2, i);
		f = test_subslab_extrema(s);
		SLABLIST_TEST_INSERT_SLAB(f, s, s1, s2, i);
	}
}

/*
 * Inserts `elem` into `s`, which is a full slab, but moves the maximum element
 * into the slab next to `s`.
 */
static void
insert_spill_next(slab_t *s, uintptr_t elem)
{
	uintptr_t lst_elem = s->s_arr[(s->s_elems - 1)];
	uintptr_t b4_lst_elem = s->s_arr[(s->s_elems - 2)];
	slab_t *snx = s->s_next;
	s->s_elems--;
	SLABLIST_SLAB_DEC_ELEMS(s);

	s->s_max = b4_lst_elem;
	SLABLIST_SLAB_SET_MAX(s);

	int q = slab_bin_srch(elem, s);
	insert_elem(snx, lst_elem, 0);
	insert_elem(s, elem, q);
}

/*
 * Inserts slab `s1` or subslab `s2` into `s`, which is a full subslab, but
 * moves the maximum element into the subslab next to `s`.
 */
static void
sub_insert_spill_next(subslab_t *s, slab_t *s1, subslab_t *s2)
{
	uint16_t lst_index = s->ss_elems - 1;
	uint16_t b4_lst_index = s->ss_elems - 2;
	slab_t *lst_slab = NULL;
	slab_t *b4_lst_slab = NULL;
	subslab_t *lst_subslab = NULL;
	subslab_t *b4_lst_subslab = NULL;
	if (s1 != NULL) {
		lst_slab = GET_SUBSLAB_ELEM(s, lst_index);
		b4_lst_slab = GET_SUBSLAB_ELEM(s, b4_lst_index);
		s->ss_max = b4_lst_slab->s_max;
	} else {
		lst_subslab = GET_SUBSLAB_ELEM(s, lst_index);
		b4_lst_subslab = GET_SUBSLAB_ELEM(s, b4_lst_index);
		s->ss_max = b4_lst_subslab->ss_max;
	}
	SLABLIST_SUBSLAB_SET_MAX(s);
	subslab_t *snx = s->ss_next;
	s->ss_elems--;
	SLABLIST_SUBSLAB_DEC_ELEMS(s);

	int q = 0;
	if (s1 != NULL) {
		q = subslab_bin_srch_top(s1->s_max, s);
	} else {
		q = subslab_bin_srch(s2->ss_max, s);
	}

	insert_slab(snx, lst_slab, lst_subslab, 0);
	insert_slab(s, s1, s2, q);
}

/*
 * Inserts `elem` into `s`, which is a full slab, but moves the minimum element
 * into the slab previous to `s`.
 */
static void
insert_spill_prev(slab_t *s, uintptr_t elem)
{
	/*
	 * Whenever this function gets called we assume the `s` is FULL.
	 */
	uintptr_t fst_elem = s->s_arr[0];
	slab_t *spv = s->s_prev;
	s->s_elems--;
	SLABLIST_SLAB_DEC_ELEMS(s);

	SLABLIST_BWDSHIFT_BEGIN(s->s_list, s, 1);
	bcopy(&(s->s_arr[1]), s->s_arr, ((SELEM_MAX - 1)*8));
	SLABLIST_BWDSHIFT_END();

	s->s_min = s->s_arr[0];
	SLABLIST_SLAB_SET_MIN(s);

	int j = 0;
	if (spv->s_elems) {
		j = slab_bin_srch(fst_elem, spv);
	}
	int q = slab_bin_srch(elem, s);

	insert_elem(spv, fst_elem, j);
	insert_elem(s, elem, q);
}

/*
 * Inserts slab `s1` or subslab `s2` into `s`, which is a full slab, but moves
 * the minimum element into the slab previous to `s`.
 */
static void
sub_insert_spill_prev(subslab_t *s, slab_t *s1, subslab_t *s2)
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
	/*
	bcopy(&(GET_SUBSLAB_ELEM(s, 1)), s->ss_arr->sa_data,
	    ((s->ss_elems - 1) * sizeof (void *)));
	*/
	SLABLIST_SUBBWDSHIFT_END();

	s->ss_elems--;
	SLABLIST_SUBSLAB_DEC_ELEMS(s);

	if (s1 != NULL) {
		new_fst_slab = GET_SUBSLAB_ELEM(s, 0);
		s->ss_min = new_fst_slab->s_min;
	} else {
		new_fst_subslab = GET_SUBSLAB_ELEM(s, 0);
		s->ss_min = new_fst_subslab->ss_min;
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
	int q;
	if (s1 != NULL) {
		q = subslab_bin_srch_top(s1->s_max, s);
	} else {
		q = subslab_bin_srch(s2->ss_max, s);
	}

	insert_slab(spv, fst_slab, fst_subslab, j);
	insert_slab(s, s1, s2, q);
}

/*
 * Assuming that `elem` is in the range of `s`, we insert into `s`. If `rep` is
 * not 0, we can replace an elem that has the same key as `elem` with `elem`.
 * The replaced elem is saved in `repd_elem`. If we can't replace but find a
 * duplicate we notify the caller, by setting `edup`.
 */
static slab_t *
gen_insert_ira(slablist_t *sl, slab_t *s, uintptr_t elem, int rep,
	uintptr_t *repd_elem, int *edup)
{
	/*
	 * If we are inserting into a subslab, then we are inserting an
	 * `elem` that is a pointer to a slab.
	 */
	slab_t *ns = NULL;

	int i = 0;
	/*
	 * If the slab `s` is not full, or if there is the possibility of
	 * replacing an existing element we try to insert into the slab. On the
	 * other hand, if the slab is full, we have to try to insert the elem
	 * into the slab `s` while moving elems between the adjacent slabs.
	 */
	if (s->s_elems < SELEM_MAX || rep) {
		i = slab_bin_srch(elem, s);
		if (sl->sl_cmp_elem(s->s_arr[i], elem) == 0) {
			/*
			 * We don't store dups, so we either bail or replace,
			 * based on preference. Only if `elem` is in the slab's
			 * range can it be a duplicate.
			 */
			if (rep) {
				if (repd_elem != NULL) {
					*repd_elem = s->s_arr[i];
				}
				s->s_arr[i] = elem;
				SLABLIST_SLAB_ADD_REPLACE(sl, NULL, elem, 1);
				return (NULL);
			} else {
				SLABLIST_SLAB_ADD_REPLACE(sl, NULL, elem, 0);
				*edup = SL_EDUP;
				return (NULL);
			}
		}
		SLABLIST_SLAB_ADD_INTO(sl, s, elem);
		insert_elem(s, elem, i);

	} else {

		slab_t *snx = s->s_next;
		slab_t *spv = s->s_prev;
		if (snx != NULL && snx->s_elems < SELEM_MAX) {
			SLABLIST_SLAB_ADD_INTO_SPILL_NEXT(sl, s, elem);
			insert_spill_next(s, elem);
			return (ns);
		}
		if (spv != NULL && spv->s_elems < SELEM_MAX) {
			SLABLIST_SLAB_ADD_INTO_SPILL_PREV(sl, s, elem);
			insert_spill_prev(s, elem);
			return (ns);
		}
		if (snx == NULL || snx->s_elems == SELEM_MAX) {
			SLABLIST_SLAB_ADD_INTO_SPILL_NEXT_MK(sl, s, elem);
			ns = mk_slab();
			SLABLIST_SLAB_MK(sl);
			link_slab(ns, s, SLAB_LINK_AFTER);
			insert_spill_next(s, elem);
			return (ns);
		}
		if (spv == NULL || spv->s_elems == SELEM_MAX) {
			SLABLIST_SLAB_ADD_INTO_SPILL_PREV_MK(sl, s, elem);
			ns = mk_slab();
			SLABLIST_SLAB_MK(sl);
			link_slab(ns, s, SLAB_LINK_BEFORE);
			insert_spill_prev(s, elem);
			return (ns);
		}
	}
	return (NULL);
}

static subslab_t *
sub_gen_insert_ira(slablist_t *sl, subslab_t *s, slab_t *s1, subslab_t *s2)
{
	/*
	 * If we are inserting into a subslab, then we are inserting an
	 * `elem` that is a pointer to a slab.
	 */
	subslab_t *ns = NULL;

	int i = 0;
	/*
	 * If the slab `s` is not full, or if there is the possibility of
	 * replacing an existing element we try to insert into the slab. On the
	 * other hand, if the slab is full, we have to try to insert the elem
	 * into the slab `s` while moving elems between the adjacent slabs.
	 */
	if (s->ss_elems < SUBELEM_MAX) {

		if (s1 != NULL) {
			i = subslab_bin_srch_top(s1->s_max, s);
		} else {
			i = subslab_bin_srch(s2->ss_max, s);
		}

		SLABLIST_SUBSLAB_ADD_INTO(sl, s, s1, s2);
		insert_slab(s, s1, s2, i);

	} else {

		subslab_t *snx = s->ss_next;
		subslab_t *spv = s->ss_prev;
		if (snx != NULL && snx->ss_elems < SUBELEM_MAX) {
			SLABLIST_SUBSLAB_ADD_INTO_SPILL_NEXT(sl, s, s1, s2);
			sub_insert_spill_next(s, s1, s2);
			return (ns);
		}
		if (spv != NULL && spv->ss_elems < SUBELEM_MAX) {
			SLABLIST_SUBSLAB_ADD_INTO_SPILL_PREV(sl, s, s1, s2);
			sub_insert_spill_prev(s, s1, s2);
			return (ns);
		}
		if (snx == NULL || snx->ss_elems == SUBELEM_MAX) {
			SLABLIST_SUBSLAB_ADD_INTO_SPILL_NEXT_MK(sl, s, s1, s2);
			ns = mk_subslab();
			ns->ss_arr = mk_subarr();
			SLABLIST_SUBSLAB_MK(sl);
			link_subslab(ns, s, SLAB_LINK_AFTER);
			sub_insert_spill_next(s, s1, s2);
			return (ns);
		}
		if (spv == NULL || spv->ss_elems == SUBELEM_MAX) {
			SLABLIST_SUBSLAB_ADD_INTO_SPILL_PREV_MK(sl, s, s1, s2);
			ns = mk_subslab();
			ns->ss_arr = mk_subarr();
			SLABLIST_SUBSLAB_MK(sl);
			link_subslab(ns, s, SLAB_LINK_BEFORE);
			sub_insert_spill_prev(s, s1, s2);
			return (ns);
		}
	}
	return (NULL);
}

/*
 * Assuming that `elem` is under the range of `s`, we have the following
 * possible way of inserting `elem` into the slablist.
 *
 * We can:
 *
 *	Insert into `s` if not full.
 *	Insert into `s->s_prev` if not full.
 *	Spill max of `s` into next, insert into `s`.
 *	Create new previous slab, insert into that.
 */
static slab_t *
gen_insert_ura(slablist_t *sl, slab_t *s, uintptr_t elem)
{
	/*
	 * If we are inserting into a subslab, then we are inserting an
	 * `elem` that is a pointer to a slab.
	 */
	int i = slab_bin_srch(elem, s);
	slab_t *ns = NULL;
	if (s->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_ADD_INTO(sl, s, elem);
		insert_elem(s, elem, i);
		return (ns);
	}
	if (s->s_prev != NULL && s->s_prev->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_ADD_BEFORE(sl, s, elem);
		i = slab_bin_srch(elem, s->s_prev);
		insert_elem(s->s_prev, elem, i);
		return (ns);
	}
	if (s->s_next != NULL && s->s_next->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_ADD_INTO_SPILL_NEXT(sl, s, elem);
		insert_spill_next(s, elem);
		return (ns);
	}
	SLABLIST_SLAB_ADD_BEFORE_MK(sl, s, elem);
	ns = mk_slab();
	SLABLIST_SLAB_MK(sl);
	link_slab(ns, s, SLAB_LINK_BEFORE);
	insert_elem(s->s_prev, elem, 0);
	return (ns);
}

static subslab_t *
sub_gen_insert_ura(slablist_t *sl, subslab_t *s, slab_t *s1, subslab_t *s2)
{
	/*
	 * If we are inserting into a subslab, then we are inserting a slab or
	 * subslab pointer.
	 */
	int i;
	if (s1 != NULL) {
		i = subslab_bin_srch_top(s1->s_max, s);
	} else {
		i = subslab_bin_srch(s2->ss_max, s);
	}
	subslab_t *ns = NULL;
	if (s->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_ADD_INTO(sl, s, s1, s2);
		insert_slab(s, s1, s2, i);
		return (ns);
	}
	if (s->ss_prev != NULL && s->ss_prev->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_ADD_BEFORE(sl, s, s1, s2);
		if (s1 != NULL) {
			i = subslab_bin_srch_top(s1->s_max, s->ss_prev);
		} else {
			i = subslab_bin_srch(s2->ss_max, s->ss_prev);
		}
		insert_slab(s->ss_prev, s1, s2, i);
		return (ns);
	}
	if (s->ss_next != NULL && s->ss_next->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_ADD_INTO_SPILL_NEXT(sl, s, s1, s2);
		sub_insert_spill_next(s, s1, s2);
		return (ns);
	}
	SLABLIST_SUBSLAB_ADD_BEFORE_MK(sl, s, s1, s2);
	ns = mk_subslab();
	ns->ss_arr = mk_subarr();
	SLABLIST_SUBSLAB_MK(sl);
	link_subslab(ns, s, SLAB_LINK_BEFORE);
	insert_slab(s->ss_prev, s1, s2, 0);
	return (ns);
}

/*
 * Assuming that `elem` is under the range of `s`, we have the following
 * possible way of inserting `elem` into the slablist.
 *
 * We can:
 *
 *	Insert into `s` if not full.
 *	Insert into `s->s_next` if not full.
 *	Spill min of `s` into prev, insert into `s`.
 *	Create new next slab, insert into that.
 */
static slab_t *
gen_insert_ora(slablist_t *sl, slab_t *s, uintptr_t elem)
{
	int i = slab_bin_srch(elem, s);
	slab_t *ns = NULL;
	if (s->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_ADD_INTO(sl, s, elem);
		insert_elem(s, elem, i);
		return (ns);
	}
	if (s->s_next != NULL && s->s_next->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_ADD_AFTER(sl, s, elem);
		i = slab_bin_srch(elem, s->s_next);
		insert_elem(s->s_next, elem, i);
		return (ns);
	}
	if (s->s_prev != NULL && s->s_prev->s_elems < SELEM_MAX) {
		SLABLIST_SLAB_ADD_INTO_SPILL_PREV(sl, s, elem);
		insert_spill_prev(s, elem);
		return (ns);
	}
	SLABLIST_SLAB_ADD_AFTER_MK(sl, s, elem);
	ns = mk_slab();
	SLABLIST_SLAB_MK(sl);
	link_slab(ns, s, SLAB_LINK_AFTER);
	insert_elem(s->s_next, elem, 0);
	return (ns);
}

static subslab_t *
sub_gen_insert_ora(slablist_t *sl, subslab_t *s, slab_t *s1, subslab_t *s2)
{
	int i = 0;
	if (s1 != NULL) {
		i = subslab_bin_srch_top(s1->s_max, s);
	} else {
		i = subslab_bin_srch(s2->ss_max, s);
	}
	subslab_t *ns = NULL;
	if (s->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_ADD_INTO(sl, s, s1, s2);
		insert_slab(s, s1, s2, i);
		return (ns);
	}
	if (s->ss_next != NULL && s->ss_next->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_ADD_AFTER(sl, s, s1, s2);
		if (s1 != NULL) {
			i = subslab_bin_srch_top(s1->s_max, s->ss_next);
		} else {
			i = subslab_bin_srch(s2->ss_max, s->ss_next);
		}
		insert_slab(s->ss_next, s1, s2, i);
		return (ns);
	}
	if (s->ss_prev != NULL && s->ss_prev->ss_elems < SUBELEM_MAX) {
		SLABLIST_SUBSLAB_ADD_INTO_SPILL_PREV(sl, s, s1, s2);
		sub_insert_spill_prev(s, s1, s2);
		return (ns);
	}
	SLABLIST_SUBSLAB_ADD_AFTER_MK(sl, s, s1, s2);
	ns = mk_subslab();
	ns->ss_arr = mk_subarr();
	SLABLIST_SUBSLAB_MK(sl);
	link_subslab(ns, s, SLAB_LINK_AFTER);
	insert_slab(s->ss_next, s1, s2, 0);
	return (ns);
}

/*
 * This procedure is invoked after a slab is found. Given the status it finds a
 * way to insert elem into the slablist. It inserts the element into a slab of
 * appropriate range, doing this may require changing the range of the slab,
 * creating new adjacent slabs, or shifting the min/max of the current slab
 * into adjacent slabs. Or it creates a new slab with the appropriate range.
 * All of those possibilities depend on whether a slab with appropriate range
 * was found, and on whether the slab (and the slabs adjacent to it) are at
 * capacity. See the above three functions for more details.
 */
static slab_t *
slab_gen_insert(int status, uintptr_t elem, slab_t *s, int rep,
    uintptr_t *repd_elem, int *edup)
{
	slab_t *ns = NULL;
	slablist_t *sl = s->s_list;
	*edup = 0;

	if (status == FS_IN_RANGE) {
		ns = gen_insert_ira(sl, s, elem, rep, repd_elem, edup);
		return (ns);
	}

	if (status == FS_OVER_RANGE) {
		ns = gen_insert_ora(sl, s, elem);
		return (ns);
	}

	if (status == FS_UNDER_RANGE) {
		ns = gen_insert_ura(sl, s, elem);
		return (ns);
	}

	return (NULL);
}

static subslab_t *
subslab_gen_insert(int status, slab_t *s1, subslab_t *s2, subslab_t *s)
{
	subslab_t *ns = NULL;
	slablist_t *sl = s->ss_list;

	if (status == FS_IN_RANGE) {
		ns = sub_gen_insert_ira(sl, s, s1, s2);
		return (ns);
	}

	if (status == FS_OVER_RANGE) {
		ns = sub_gen_insert_ora(sl, s, s1, s2);
		return (ns);
	}

	if (status == FS_UNDER_RANGE) {
		ns = sub_gen_insert_ura(sl, s, s1, s2);
		return (ns);
	}

	return (NULL);
}

int
subslab_update_extrema(subslab_t *s)
{
	subslab_t *sub = s;
	slab_t *sup1;
	subslab_t *sup2;
	int ret = 0;
	int last = sub->ss_elems - 1;
	if (sub->ss_list->sl_layer == 1) {
		sup1 = GET_SUBSLAB_ELEM(sub, 0);
		if (sub->ss_min != sup1->s_min) {
			sub->ss_min = sup1->s_min;
			SLABLIST_SUBSLAB_SET_MIN(sub);
			ret |= 1;
		}
		sup1 = GET_SUBSLAB_ELEM(sub, last);
		int last = sup1->s_elems - 1;
		if (sub->ss_max != sup1->s_max) {
			sub->ss_max = sup1->s_max;
			SLABLIST_SUBSLAB_SET_MAX(sub);
			ret |= 2;
		}
		last = sup1->s_elems - 1;
		return (ret);
	} else {
		sup2 = GET_SUBSLAB_ELEM(sub, 0);
		if (sub->ss_min != sup2->ss_min) {
			sub->ss_min = sup2->ss_min;
			SLABLIST_SUBSLAB_SET_MIN(sub);
			ret |= 1;
		}
		sup2 = GET_SUBSLAB_ELEM(sub, last);
		if (sub->ss_max != sup2->ss_max) {
			sub->ss_max = sup2->ss_max;
			SLABLIST_SUBSLAB_SET_MAX(sub);
			ret |= 2;
		}
		return (ret);
	}
}

void
ripple_update_extrema(bc_t *crumbs, int i)
{
	/*
	 * This loop ripples any changed extrema to the sublayers. We update
	 * crumbs[bc] and adjacent slabs (if any).
	 */
	int bc = i;
	subslab_t *sub;
	while (bc > 0) {
		sub = retrieve_subslab(crumbs, (bc - 1));
		subslab_update_extrema(sub);
		
		if (sub->ss_next != NULL) {
			subslab_update_extrema(sub->ss_next);
		}

		if (sub->ss_prev != NULL) {
			subslab_update_extrema(sub->ss_prev);
		}
		bc--;
	}
}

/*
 * If a new slab was added to the slablist, in order to complete an insertion,
 * this function "ripples" this change through the sublayers, so that the new
 * slab is potentially reachable from the baselayer.
 */
static void
ripple_add_to_sublayers(slablist_t *sl, slab_t *new, bc_t *crumbs)
{
	subslab_t *baseslab = retrieve_subslab(crumbs, 0);
	subslab_t *s = NULL;
	subslab_t *new_subslab = NULL;
	slablist_t *baselayer = baseslab->ss_list;
	uint8_t superlayers = baselayer->sl_layer;
	slablist_t *csl = sl;
	slablist_t *psl;
	int fs;
	int layer = 0;
	int bc = 0;
	int maxupdate = 0;
	int minupdate = 0;

	bc += (superlayers);
	ripple_update_extrema(crumbs, bc);


	/*
	 * First, we ripple the new slab to the immediate sublayer. Then we
	 * ripple, any new subslabs to the sublayers below the immediate
	 * sublayer.
	 */
	bc = (superlayers - 1);
	if (new != NULL) {
		s = retrieve_subslab(crumbs, bc);
		if (SLABLIST_TEST_RIPPLE_ADD_SLAB_ENABLED()) {
			int f = test_ripple_add_slab(new, crumbs, bc);
			SLABLIST_TEST_RIPPLE_ADD_SLAB(f, new, s, crumbs, bc);
		}
		SLABLIST_RIPPLE_ADD_SLAB(csl, new, s);
		fs = sub_is_elem_in_range(new->s_min, s);
		new_subslab = subslab_gen_insert(fs, new, NULL, s);
	}
	layer++;
	bc--;

	/*
	 * This loop ripples any new subslabs to the sublayers.
	 */
	bc = (superlayers - 2);
	while ((new_subslab != NULL || maxupdate || minupdate) &&
	    layer < superlayers) {
		s = retrieve_subslab(crumbs, bc);
		int last = s->ss_elems - 1;
		if (new_subslab != NULL &&
		    SLABLIST_TEST_RIPPLE_ADD_SUBSLAB_ENABLED()) {
			int f = test_ripple_add_subslab(new_subslab, crumbs, bc);
			SLABLIST_TEST_RIPPLE_ADD_SUBSLAB(f, new_subslab, s,
				crumbs, bc);
		}

		psl = csl;
		csl = csl->sl_sublayer;
		SLABLIST_RIPPLE_ADD_SUBSLAB(csl, new_subslab, s);
		if (new_subslab != NULL) {
			fs = sub_is_elem_in_range(new_subslab->ss_min, s);
		}

		uintptr_t old_max = s->ss_max;
		uintptr_t old_min = s->ss_min;

		if (new_subslab != NULL) {
			new_subslab = subslab_gen_insert(fs, NULL, new_subslab, s);
		}

		uintptr_t max = s->ss_max;
		uintptr_t min = s->ss_min;
		subslab_t *subslab = crumbs->bc_ssarr[(bc - 1)].ssbc_subslab;

#define	LAST_ELEM(s) GET_SUBSLAB_ELEM(s, last)

		if (bc == 0) {
			goto skip_extrema_ripple;
		}
		if (s == (subslab_t *)LAST_ELEM(subslab) &&
		    sl->sl_cmp_elem(old_max, max) != 0) {
			subslab->ss_max = max;
			maxupdate = 1;
		} else {
			maxupdate = 0;
		}

		if (s == (subslab_t *)GET_SUBSLAB_ELEM(subslab, 0) &&
		    sl->sl_cmp_elem(old_min, min) != 0) {
			subslab->ss_min = min;
			minupdate = 1;
		} else {
			minupdate = 0;
		}

skip_extrema_ripple:;

		csl->sl_elems++;
		SLABLIST_SL_INC_ELEMS(csl);
		layer++;
		bc--;
	}
}

/*
 * This function adds an element to a slablist. `rep` indicates if an
 * already-inserted element with an identical key should to be replaced.
 * This function is an entry point into libslablist.
 */
int
slablist_add(slablist_t *sl, uintptr_t elem, int rep, uintptr_t *repd_elem)
{
	lock_list(sl);


	int ret;
	if (sl->sl_is_small_list && sl->sl_elems <= (SMELEM_MAX - 1)) {
		/*
		 * The number of elements is too small to justify the use of
		 * slabs. So we store the data in a singly linked list.
		 */
		SLABLIST_ADD_BEGIN(sl, elem, rep);
		ret = small_list_add(sl, elem, 0,  NULL);
		SLABLIST_ADD_END(ret);
		return (ret);
	}

	if (sl->sl_is_small_list && sl->sl_elems == SMELEM_MAX) {
		/*
		 * We convert the small_list into a slab.
		 */
		small_list_to_slab(sl);
	}



	slab_t *s;

	int edup = SL_SUCCESS;

	if (SLIST_SORTED(sl->sl_flags)) {
		/*
		 * If the slablist is sorted, we find the slab with the
		 * appropriate range and place the element there.
		 */

		if (SLABLIST_TEST_IS_SLAB_LIST_ENABLED()) {
			SLABLIST_TEST_IS_SLAB_LIST(sl->sl_is_small_list);
		}

		SLABLIST_ADD_BEGIN(sl, elem, rep);

		int fs;
		bc_t bc_path;
		bzero(&bc_path, sizeof (bc_t));
		if (sl->sl_sublayers) {
			/*
			 * If this slablist has sublayers, we create a buffer
			 * of slab-ptrs. find_bubble_up populates this
			 * buffer, with the slab that it walked over from each
			 * layer. This way, the buffer can be used to add
			 * elements to the sublayers, if we add a new slab to
			 * the overlier. If we don't have sublayers, then we
			 * do a linear srch to find the appropriate slab.
			 */
			fs = find_bubble_up(sl, elem, &bc_path);
			s = bc_path.bc_top.sbc_slab;

			if (SLABLIST_TEST_BREAD_CRUMBS_ENABLED()) {
				uint64_t bcn = sl->sl_sublayers;
				int l;
				int f = test_breadcrumbs(&bc_path, &l, bcn);
				SLABLIST_TEST_BREAD_CRUMBS(f, l);
			}

		} else {
			fs = find_linear_scan(sl, elem, &s);
		}

		slab_t *new = slab_gen_insert(fs, elem, s, rep, repd_elem,
				&edup);

		if (sl->sl_sublayers) {
			/*
			 * If this slablist has sublayers we need to update the
			 * sublayers with new extrema and a possibly a new
			 * reference. Then we remove the buffer containing the
			 * bread crumbs.
			 */
			ripple_add_to_sublayers(sl, new, &bc_path);
		}

		slablist_t *usl = NULL;

		if (sl->sl_sublayer == NULL) {
			usl = sl;
		} else {
			usl = sl->sl_baselayer;
		}

		if (sl->sl_req_sublayer &&
		    usl->sl_slabs >= sl->sl_req_sublayer) {
			/*
			 * If we have sl_req_sublayer slabs at the baselayer
			 * (or, if we have no sublayers, in the slablist in
			 * general) we map the slabs in the slablist/baselayer
			 * to a newly created baselayer.
			 */
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
			s->s_elems++;
			SLABLIST_SLAB_INC_ELEMS(s);
		} else {
			slab_t *ns = mk_slab();
			link_slab(ns, s, SLAB_LINK_AFTER);
			SLABLIST_SLAB_MK(sl);
			ns->s_arr[0] = elem;
			ns->s_elems++;
			SLABLIST_SLAB_INC_ELEMS(ns);
		}
	}

	try_reap_all(sl);

	if (edup != SL_SUCCESS) {
		ret = SL_EDUP;
	} else {
		sl->sl_elems++;
		SLABLIST_SL_INC_ELEMS(sl);
		ret = SL_SUCCESS;
	}

	unlock_list(sl);

	SLABLIST_ADD_END(ret);
	return (ret);

}
