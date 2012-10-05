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
#include <thread.h>
#include <synch.h>
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

	/*
	 * Here are the dtrace-driven tests for this function.
	 */
	if (SLABLIST_TEST_NULLARG_ENABLED()) {
		if (sl == NULL) {
			SLABLIST_TEST_NULLARG(1, 1);
		} else {
			if (sl->sl_obj_sz > 8 && (void *)elem == NULL) {
				SLABLIST_TEST_NULLARG(1, 2);
			}
		}
	}

	if (SLABLIST_TEST_IS_SML_LIST_ENABLED()) {
		SLABLIST_TEST_IS_SML_LIST(!(sl->sl_is_small_list));
	}

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
		return (SL_SUCCESS);
	}

	small_list_t *sml;
	small_list_t *nsml = NULL;

	if (SLIST_SORTED(sl->sl_flags)) {
		/*
		 * We place the element in the position so that it is less than
		 * the element after it.
		 */
		if (SLABLIST_TEST_SMLIST_ELEMS_SORTED_ENABLED()) {
			/*
			 * If test probe is enabled, we verify that the elems
			 * are sorted.
			 */
			int f = test_smlist_elems_sorted(sl);
			SLABLIST_TEST_SMLIST_ELEMS_SORTED(f);
		}

		sml = sl->sl_head;
		small_list_t *prev = NULL;
		int i = 0;
		while (i < sl->sl_elems) {
			if (sl->sl_cmp_elem(elem, sml->sml_data) == -1) {
				/*
				 * If `elem` is less than the data in the
				 * current sml_node, we insert `elem` before
				 * it.
				 */
				nsml = mk_sml_node();
				nsml->sml_data = elem;
				link_sml_node(sl, prev, nsml);
				return (SL_SUCCESS);
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
					SLABLIST_ADD_REPLACE(sl, NULL, elem, 1);
				} else {
					/*
					 * We don't want to insert duplicates.
					 */
					SLABLIST_ADD_REPLACE(sl, NULL, elem, 0);
					return (SL_EDUP);
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
			return (SL_SUCCESS);
		}


	}

	/*
	 * We place the element at the end of the list.
	 */
	int i = 0;
	sml = sl->sl_head;
	while (i < (sl->sl_elems - 1)) {
		sml = sml->sml_next;
		i++;
	}
	nsml = mk_sml_node();
	nsml->sml_data = elem;
	link_sml_node(sl, sml, nsml);
	return (SL_SUCCESS);
}

/*
 * Inserts an `elem` to insert into slab `s` at index `i`.
 */
static void
insert_elem(uintptr_t elem, slab_t *s, int i)
{
	int ip = 0;		/* insert-point */
	slablist_t *sl;
	sl = s->s_list;
	int sublayer = SLIST_SUBLAYER(sl->sl_flags);

	ip = i;

	SLABLIST_FWDSHIFT_BEGIN(s->s_list, s, i);
	size_t shiftsz = (s->s_elems - i) * 8;
	bcopy(&(s->s_arr[i]), &(s->s_arr[(i+1)]), shiftsz);
	s->s_arr[i] = elem;
	SLABLIST_FWDSHIFT_END();

end:;
	if (ip == 0) {
		if (sublayer) {
			slab_t *sm = (slab_t *)elem;
			s->s_min = sm->s_min;
		} else {
			s->s_min = s->s_arr[0];
		}
		SLABLIST_SLAB_SET_MIN(s);
	}

	if (ip == (s->s_elems)) {
		if (sublayer) {
			slab_t *sm = (slab_t *)elem;
			s->s_max = sm->s_max;
		} else {
			s->s_max = s->s_arr[(s->s_elems)];
		}
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
}

/*
 * Inserts `elem` into `s`, which is a full slab, but moves the maximum element
 * into the slab next to `s`.
 */
static void
insert_spill_next(uintptr_t elem, slab_t *s)
{
	slablist_t *sl = s->s_list;
	uintptr_t lst_elem = s->s_arr[(s->s_elems - 1)];
	uintptr_t b4_lst_elem = s->s_arr[(s->s_elems - 2)];
	slab_t *snx = s->s_next;
	slab_t *p;
	s->s_elems--;
	SLABLIST_SLAB_DEC_ELEMS(s);

	if (sl->sl_layer) {
		p = (slab_t *)b4_lst_elem;
		s->s_max = p->s_max;
	} else {
		s->s_max = b4_lst_elem;
	}
	SLABLIST_SLAB_SET_MAX(s);

	int j = 0;
	if (snx->s_elems) {
		j = slab_srch(lst_elem, snx, sl->sl_layer);
	}
	int q = slab_srch(elem, s, sl->sl_layer);
	insert_elem(lst_elem, snx, j);
	insert_elem(elem, s, q);
}

/*
 * Inserts `elem` into `s`, which is a full slab, but moves the minimum element
 * into the slab previous to `s`.
 */
static void
insert_spill_prev(uintptr_t elem, slab_t *s)
{
	/*
	 * Whenever this function gets called we assume the `s` is FULL.
	 */
	slablist_t *sl = s->s_list;
	uintptr_t fst_elem = s->s_arr[0];
	slab_t *spv = s->s_prev;
	slab_t *p;
	s->s_elems--;
	SLABLIST_SLAB_DEC_ELEMS(s);

	SLABLIST_BWDSHIFT_BEGIN(s->s_list, s, 1);
	bcopy(&(s->s_arr[1]), s->s_arr, ((SELEM_MAX - 1)*8));
	SLABLIST_BWDSHIFT_END();

	if (sl->sl_layer) {
		p = (slab_t *)(s->s_arr[0]);
		s->s_min = p->s_min;
	} else {
		s->s_min = s->s_arr[0];
	}
	SLABLIST_SLAB_SET_MIN(s);

	int j = 0;
	if (spv->s_elems) {
		j = slab_srch(fst_elem, spv, sl->sl_layer);
	}
	int q = slab_srch(elem, s, sl->sl_layer);
	insert_elem(fst_elem, spv, j);
	insert_elem(elem, s, q);
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
	int is_slab = sl->sl_layer;
	slab_t *ns = NULL;

	int i = 0;
	if (s->s_elems < SELEM_MAX || rep) {
		i = slab_srch(elem, s, is_slab);
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
				SLABLIST_ADD_REPLACE(sl, NULL, elem, 1);
				return (NULL);
			} else {
				SLABLIST_ADD_REPLACE(sl, NULL, elem, 0);
				*edup = SL_EDUP;
				return (NULL);
			}
		}
		SLABLIST_ADD_INTO(sl, s, elem);
		insert_elem(elem, s, i);

	} else {

		slab_t *snx = s->s_next;
		slab_t *spv = s->s_prev;
		if (snx != NULL && snx->s_elems < SELEM_MAX) {
			SLABLIST_ADD_INTO_SPILL_NEXT(sl, s, elem);
			insert_spill_next(elem, s);
			return (ns);
		}
		if (spv != NULL && spv->s_elems < SELEM_MAX) {
			SLABLIST_ADD_INTO_SPILL_PREV(sl, s, elem);
			insert_spill_prev(elem, s);
			return (ns);
		}
		if (snx == NULL || snx->s_elems == SELEM_MAX) {
			SLABLIST_ADD_INTO_SPILL_NEXT_MK(sl, s, elem);
			ns = mk_slab();
			SLABLIST_SLAB_MK(sl);
			link_slab(ns, s, SLAB_LINK_AFTER);
			insert_spill_next(elem, s);
			return (ns);
		}
		if (spv == NULL || spv->s_elems == SELEM_MAX) {
			SLABLIST_ADD_INTO_SPILL_PREV_MK(sl, s, elem);
			ns = mk_slab();
			SLABLIST_SLAB_MK(sl);
			link_slab(ns, s, SLAB_LINK_BEFORE);
			insert_spill_prev(elem, s);
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
	int is_slab = sl->sl_layer;
	int i = slab_srch(elem, s, is_slab);
	slab_t *ns = NULL;
	if (s->s_elems < SELEM_MAX) {
		SLABLIST_ADD_INTO(sl, s, elem);
		insert_elem(elem, s, i);
		return (ns);
	}
	if (s->s_prev != NULL && s->s_prev->s_elems < SELEM_MAX) {
		SLABLIST_ADD_BEFORE(sl, s, elem);
		i = slab_srch(elem, s->s_prev, is_slab);
		insert_elem(elem, s->s_prev, i);
		return (ns);
	}
	if (s->s_next != NULL && s->s_next->s_elems < SELEM_MAX) {
		SLABLIST_ADD_INTO_SPILL_NEXT(sl, s, elem);
		insert_spill_next(elem, s);
		return (ns);
	}
	SLABLIST_ADD_BEFORE_MK(sl, s, elem);
	ns = mk_slab();
	SLABLIST_SLAB_MK(sl);
	link_slab(ns, s, SLAB_LINK_BEFORE);
	insert_elem(elem, s->s_prev, 0);
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
	int is_slab = sl->sl_layer;
	int i = slab_srch(elem, s, is_slab);
	slab_t *ns = NULL;
	if (s->s_elems < SELEM_MAX) {
		SLABLIST_ADD_INTO(sl, s, elem);
		insert_elem(elem, s, i);
		return (ns);
	}
	if (s->s_next != NULL && s->s_next->s_elems < SELEM_MAX) {
		SLABLIST_ADD_AFTER(sl, s, elem);
		i = slab_srch(elem, s->s_next, is_slab);
		insert_elem(elem, s->s_next, i);
		return (ns);
	}
	if (s->s_prev != NULL && s->s_prev->s_elems < SELEM_MAX) {
		SLABLIST_ADD_INTO_SPILL_PREV(sl, s, elem);
		insert_spill_prev(elem, s);
		return (ns);
	}
	SLABLIST_ADD_AFTER_MK(sl, s, elem);
	ns = mk_slab();
	SLABLIST_SLAB_MK(sl);
	link_slab(ns, s, SLAB_LINK_AFTER);
	insert_elem(elem, s->s_next, 0);
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

void
ripple_update_extrema(slab_t *l[], int i)
{
	/*
	 * This loop ripples any changed extrema to the sublayers. We update
	 * l[bc] and adjacent slabs (if any).
	 */
	slab_t *sub_s;
	slab_t *sup_s;
	int bc = i;
	while (bc > 0) {
		sub_s = l[(bc - 1)];
		sup_s = (slab_t *)sub_s->s_arr[(sub_s->s_elems - 1)];
		if (sub_s->s_max != sup_s->s_max) {
			sub_s->s_max = sup_s->s_max;
			SLABLIST_SLAB_SET_MAX(sub_s);
		}

		sup_s = (slab_t *)sub_s->s_arr[0];
		if (sub_s->s_min != sup_s->s_min) {
			sub_s->s_min = sup_s->s_min;
			SLABLIST_SLAB_SET_MIN(sub_s);
		}

		if (sub_s->s_next != NULL) {
			sub_s = sub_s->s_next;
			sup_s = (slab_t *)sub_s->s_arr[(sub_s->s_elems - 1)];
			if (sub_s->s_max != sup_s->s_max) {
				sub_s->s_max = sup_s->s_max;
				SLABLIST_SLAB_SET_MAX(sub_s);
			}

			sup_s = (slab_t *)sub_s->s_arr[0];
			if (sub_s->s_min != sup_s->s_min) {
				sub_s->s_min = sup_s->s_min;
				SLABLIST_SLAB_SET_MIN(sub_s);
			}

			sub_s = sub_s->s_prev;
		}

		if (sub_s->s_prev != NULL) {
			sub_s = sub_s->s_prev;
			sup_s = (slab_t *)sub_s->s_arr[(sub_s->s_elems - 1)];
			if (sub_s->s_max != sup_s->s_max) {
				sub_s->s_max = sup_s->s_max;
				SLABLIST_SLAB_SET_MAX(sub_s);
			}

			sup_s = (slab_t *)sub_s->s_arr[0];
			if (sub_s->s_min != sup_s->s_min) {
				sub_s->s_min = sup_s->s_min;
				SLABLIST_SLAB_SET_MIN(sub_s);
			}

			sub_s = sub_s->s_next;
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
ripple_add_to_sublayers(slablist_t *sl, slab_t *new, slab_t *l[])
{
	int nu = sl->sl_sublayers;
	slablist_t *csl = sl;
	int cu = 0;
	int bc = 0;
	int edup;

	bc += (nu);
	ripple_update_extrema(l, bc);


	/*
	 * This loop ripples the new slab to the sublayers.
	 */
	bc = 0;
	bc += (nu - 1);
	while (new != NULL && cu < nu) {
		csl = csl->sl_sublayer;
		SLABLIST_RIPPLE_ADD_SLAB(csl, new, l[bc]);
		int fs = is_elem_in_range(new->s_min, l[bc]);
		new = slab_gen_insert(fs, (uintptr_t)new, l[bc], 0,
			NULL, &edup);
		csl->sl_elems++;
		SLABLIST_SL_INC_ELEMS(csl);
		cu++;
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
	test_slab_consistency(sl);

	if (SLIST_SORTED(sl->sl_flags)) {
		/*
		 * If the slablist is sorted, we find the slab with the
		 * appropriate range and place the element there.
		 */

		SLABLIST_ADD_BEGIN(sl, elem, rep);
		test_slab_sorting(sl);

		int fs;
		slab_t **bc;
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
			bc = mk_bc();
			fs = find_bubble_up(sl, elem, bc);
			s = bc[(sl->sl_sublayers)];

			if (SLABLIST_TEST_BREAD_CRUMBS_ENABLED()) {
				uint64_t bcn = sl->sl_sublayers;
				int l;
				int f = test_breadcrumbs(bc, &l, bcn);
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
			ripple_add_to_sublayers(sl, new, bc);
			test_sublayers(sl, elem);
			rm_bc(bc);
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
		uint64_t op;
		s = slab_get(sl, sl->sl_slabs - 1, &op, SLAB_IN_POS);

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
		SLABLIST_ADD_END(SL_EDUP);
		return (SL_EDUP);
	} else {
		sl->sl_elems++;
		SLABLIST_SL_INC_ELEMS(sl);
		SLABLIST_ADD_END(SL_SUCCESS);
		return (SL_SUCCESS);
	}
}
