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
 * Removal Implementation
 *
 * This file implements the procedures that remove elements from Slab Lists.
 * The code in this file uses functions defined in slablist_cons.c,
 * slablist_find.c, and slablist_test.c.
 *
 * The internal architecture of libslablist is described in slablist_impl.h
 */

#include <unistd.h>
#include <stdlib.h>
#include <thread.h>
#include <synch.h>
#include <strings.h>
#include <stdio.h>
#include "slablist_impl.h"
#include "slablist_provider.h"
#include "slablist_cons.h"
#include "slablist_find.h"
#include "slablist_test.h"

/*
 * This function removes an element from a slablist's linked list, either by
 * key or position, if the slablist is sorted or ordered, respectively.
 */
static int
small_list_rem(slablist_t *sl, uintptr_t elem, uint64_t pos, uintptr_t *rdl)
{
	if (sl->sl_head == NULL || sl->sl_elems == 0) {
		return (SL_EMPTY);
	}

	if (SLABLIST_TEST_ENABLED()) {

		if (SLABLIST_TEST_IS_SML_LIST_ENABLED()) {
			SLABLIST_TEST_IS_SML_LIST(!(sl->sl_is_small_list));
		}

		if (SLABLIST_TEST_SMLIST_NELEMS_ENABLED()) {
			int f = test_smlist_nelems(sl);
			SLABLIST_TEST_SMLIST_NELEMS(f);
		}

	}

	small_list_t *prev = NULL;

	if (SLIST_SORTED(sl->sl_flags)) {

		if (SLABLIST_TEST_ENABLED() &&
			SLABLIST_TEST_SMLIST_ELEMS_SORTED_ENABLED()) {
			/*
			 * If test probe is enabled, we verify that the elems
			 * are sorted.
			 */
			int f = test_smlist_elems_sorted(sl);
			SLABLIST_TEST_SMLIST_ELEMS_SORTED(f);
		}

		/*
		 * We initialize sml to the head of the list, and prev to NULL.
		 * So if unlink_sml_node gets called on the first element, it
		 * will know to unlink the head. `unlink_sml_node` can't unlink
		 * a node, if it does not know what the previous element is (as
		 * these are nodes in a single linked list)..
		 */
		small_list_t *sml = sl->sl_head;
		int i = 0;
		while (i < sl->sl_elems) {
			if (sl->sl_cmp_elem(elem, sml->sml_data) == 0) {
				/*
				 * If we find the sml node that contains `elem`
				 * we unlink it, free it, and set rdl to that
				 * elem.
				 */
				*rdl = sml->sml_data;
				unlink_sml_node(sl, prev);
				rm_sml_node(sml);
				return (SL_SUCCESS);
			} else {
				/*
				 * Otherwise, we go to the next element.
				 */
				prev = sml;
				sml = sml->sml_next;
			}
			i++;
		}

		/*
		 * We didn't find a node that matched, so we idicate that
		 * nothing was removed, and we return an indication that the
		 * node was not found.
		 */
		*rdl = NULL;
		return (SL_ENFOUND);

	} else {

		uint64_t mod = pos % sl->sl_elems;
		small_list_t *sml = sl->sl_head;
		sml = sl->sl_head;


		if (sl->sl_elems < pos && !SLIST_IS_CIRCULAR(sl->sl_flags)) {
			return (SL_ENCIRC);
		}

		int i = 0;
		while (i < mod) {
			prev = sml;
			sml = sml->sml_next;
		}
		*rdl = sml->sml_data;
		unlink_sml_node(sl, prev);
		rm_sml_node(sml);
		return (SL_SUCCESS);
	}
}

/*
 * This function takes last elems in slab `s` and moves to the beginning of
 * slab `sn`. The number of elements taken depends on the number of available
 * spaces in `sn`.
 */
static void
move_to_next(slab_t *s, slab_t *sn)
{
	uint64_t nelems = sn->s_elems;
	uint64_t nfelems = SELEM_MAX - nelems;
	uint64_t melems = s->s_elems;
	uint64_t cpelems;
	uint64_t from = 0;
	size_t sz = sizeof (uintptr_t);
	slablist_t *sl = s->s_list;

	if (!melems) {
		return;
	}

	if (melems >= nfelems) {
		cpelems = nfelems;
		from = s->s_elems - cpelems;
	}

	if (nfelems > melems) {
		cpelems = melems;
	}



	slab_t *scp = NULL;
	slab_t *sncp = NULL;
	if (SLABLIST_TEST_ENABLED() && SLABLIST_TEST_MOVE_NEXT_ENABLED()) {
		/*
		 * To test that the next slab has of the elements that are to
		 * be copied, we make copies of the slabs before they get
		 * modified.
		 */
		scp = mk_slab();
		sncp = mk_slab();
		bcopy(s, scp, sizeof (slab_t));
		bcopy(sn, sncp, sizeof (slab_t));
	}

	/*
	 * We make space in the next slab for all of the preceding elements,
	 * which necessarily come before the elements in the slab. We also want
	 * to give preference to the slab with the most free space.
	 */
	bcopy(&(sn->s_arr[0]), &(sn->s_arr[cpelems]), nelems*sz);


	/*
	 * We actually move the elems from s to sn.
	 */
	bcopy(&(s->s_arr[from]), &(sn->s_arr[0]), cpelems*sz);
	sn->s_elems = sn->s_elems + cpelems;
	s->s_elems = s->s_elems - cpelems;

	if (SLABLIST_TEST_ENABLED() && SLABLIST_TEST_MOVE_NEXT_ENABLED()) {
		/*
		 * Here we ccompare the modified slabs with their pre-mod
		 * copies. And we remove the copies when done.
		 */
		int i;
		int f = test_move_next(s, scp, sn, sncp, &i);
		SLABLIST_TEST_MOVE_NEXT(f, scp, sncp, sn, from, i);
		rm_slab(scp);
		rm_slab(sncp);
	}

	if (sl->sl_layer) {
		slab_t *ss = (slab_t *)sn->s_arr[0];
		sn->s_min = ss->s_min;
		if (s->s_elems) {
			/*
			 * We only update the max of the middle slab if it is
			 * not empty.
			 */
			ss = (slab_t *)s->s_arr[(s->s_elems - 1)];
			s->s_max = ss->s_max;
		}
	} else {
		sn->s_min = sn->s_arr[0];
		s->s_max = s->s_arr[(s->s_elems - 1)];
	}
	SLABLIST_SLAB_INC_ELEMS(sn);
	SLABLIST_SLAB_DEC_ELEMS(s);
	SLABLIST_SLAB_SET_MAX(s);
	SLABLIST_SLAB_SET_MIN(sn);
}

/*
 * This function takes first elems in slab `s` and moves to the end of slab
 * `sp`. The number of elements taken depends on the number of available spaces
 * in `sp`.
 */
void
move_to_prev(slab_t *s, slab_t *sp)
{
	uint64_t pelems = sp->s_elems;
	uint64_t pfelems = SELEM_MAX - pelems;
	uint64_t melems = s->s_elems;
	uint64_t cpelems = 0;
	uint64_t from = s->s_elems - 1;
	size_t sz = sizeof (uintptr_t);
	slablist_t *sl = s->s_list;

	if (!melems) {
		return;
	}

	if (melems >= pfelems) {
		// off = melems - pfelems;
		cpelems = pfelems;
		from = pfelems - 1;
	}

	if (melems < pfelems) {
		cpelems = melems;
	}

	slab_t *scp = NULL;
	slab_t *spcp = NULL;
	if (SLABLIST_TEST_ENABLED() && SLABLIST_TEST_MOVE_PREV_ENABLED()) {
		/*
		 * To test that the next slab has of the elements that are to
		 * be copied, we make copies of the slabs before they get
		 * modified.
		 */
		scp = mk_slab();
		spcp = mk_slab();
		bcopy(s, scp, sizeof (slab_t));
		bcopy(sp, spcp, sizeof (slab_t));
	}

	/*
	 * We move the elems from s to sp.
	 */
	bcopy((s->s_arr), (sp->s_arr + pelems), cpelems*sz);
	sp->s_elems = sp->s_elems + cpelems;
	s->s_elems = s->s_elems - cpelems;
	/* bwd shift */
	bcopy((s->s_arr + cpelems), (s->s_arr), (melems-cpelems)*sz);

	if (SLABLIST_TEST_ENABLED() && SLABLIST_TEST_MOVE_PREV_ENABLED()) {
		/*
		 * Here we ccompare the modified slabs with their pre-mod
		 * copies. And we remove the copies when done.
		 */
		int i;
		int f = test_move_prev(s, scp, sp, spcp, &i);
		SLABLIST_TEST_MOVE_PREV(f, scp, spcp, sp, from, i);
		rm_slab(scp);
		rm_slab(spcp);
	}

	if (sl->sl_layer) {
		slab_t *ss;
		if (s->s_elems) {
			ss = (slab_t *)s->s_arr[0];
			s->s_min = ss->s_min;
		}
		ss = (slab_t *)sp->s_arr[(sp->s_elems - 1)];
		sp->s_max = ss->s_max;
	} else {
		s->s_min = s->s_arr[0];
		sp->s_min = sp->s_arr[0];
		sp->s_max = sp->s_arr[(sp->s_elems - 1)];
		s->s_max = s->s_arr[(s->s_elems - 1)];
	}
	SLABLIST_SLAB_INC_ELEMS(sp);
	SLABLIST_SLAB_DEC_ELEMS(s);
	SLABLIST_SLAB_SET_MAX(sp);
	SLABLIST_SLAB_SET_MIN(s);
}

/*
 * This function tries to remove a slab if it is empty. If it not empty, it
 * tries to shuffle its elements between adjacent slabs, to make it empty (and
 * thus removable). If it can't remove `s` in such a way, it will try to remove
 * the slabs adjacent to `s`. Note that, unlike slab_generic_insert, this
 * function does not remove elements within the slab. It merely places them
 * elsewhere. the slablist_rem function (below) does remove discrete elements
 * from the slab.
 */
slab_t *
slab_generic_rem(slab_t *s)
{
	slab_t *sn = s->s_next;
	slab_t *sp = s->s_prev;
	slab_t *uls = NULL; /* this is ptr to slab we have to unlink + free */
	slablist_t *sl = s->s_list;
	uint64_t nslabs = sl->sl_slabs;
	uint64_t melems = s->s_elems;
	uint64_t nelems = 0;
	uint64_t pelems = 0;
	uint64_t free_nelems = 0;
	uint64_t free_pelems = 0;
	uint64_t free_melems = 0;

	if (s->s_elems == 0) {
		/*
		 * If the slab becomes empty we can free it right away.
		 */
		uls = s;
		goto end;
	}

	if (nslabs == 1) {
		/*
		 * If we have only one slab, there is nothing for us to do and
		 * we return from the function.
		 */
		return (NULL);
	}

	if (s->s_next != NULL) {
		nelems = s->s_next->s_elems;
		free_nelems = SELEM_MAX - nelems;
		sn = s->s_next;
	}

	if (s->s_prev != NULL) {
		pelems = s->s_prev->s_elems;
		free_pelems = SELEM_MAX - pelems;
		sp = s->s_prev;
	}

	free_melems = SELEM_MAX - melems;

	if (free_nelems + free_pelems >= melems) {
		/*
		 * We can consolidate the slabs by moving elements from the
		 * middle slab into the previous and next slabs.
		 */
		if (sn != NULL && s != NULL && free_nelems >= melems) {
			/*
			 * We see if we can move all elems into a single slab.
			 */
			SLABLIST_MOVE_MID_TO_NEXT(sl, s, sn);
			move_to_next(s, sn);
			uls = s;
			goto end;
		}

		if (sp != NULL && s != NULL && free_pelems >= melems) {
			/*
			 * Same as above.
			 */
			SLABLIST_MOVE_MID_TO_PREV(sl, s, sp);
			move_to_prev(s, sp);
			uls = s;
			goto end;
		}

		/*
		 * The if-stmts below, we first copy from s into the adjacent
		 * slab that has the most free space, and then into the other
		 * adjacent slab that has less. If the free space is equal, the
		 * order doesn't matter. And this is why this particular
		 * ordering of if-stmts is important.
		 */
		if (free_nelems > 0 && sn != NULL && s != NULL &&
		    free_nelems >= free_pelems) {
			SLABLIST_MOVE_MID_TO_NEXT(sl, s, sn);
			move_to_next(s, sn);
			uls = s;
		}

		if (free_pelems > 0 && s != NULL && sp != NULL &&
		    free_pelems >= free_nelems) {
			SLABLIST_MOVE_MID_TO_PREV(sl, s, sp);
			move_to_prev(s, sp);
			uls = s;
		}

		if (free_nelems > 0 && s != NULL && sn != NULL &&
		    free_nelems < free_pelems) {
			SLABLIST_MOVE_MID_TO_NEXT(sl, s, sn);
			move_to_next(s, sn);
			uls = s;
		}

		if (free_pelems > 0 && s != NULL && sp != NULL &&
		    free_pelems < free_nelems) {
			SLABLIST_MOVE_MID_TO_PREV(sl, s, sp);
			move_to_prev(s, sp);
			uls = s;
		}

		goto end;

	} else if (free_nelems + free_melems >= pelems) {
		/*
		 * We can consolidate the slabs by moving elements from the
		 * previous slab into the next and middle slabs.
		 */
		if (sn != NULL && sp != NULL && free_nelems >= pelems) {
			/*
			 * We see if we can move all elems into a single slab.
			 */
			SLABLIST_MOVE_PREV_TO_NEXT(sl, sp, sn);
			move_to_next(sp, sn);
			uls = sp;
			goto end;
		}

		if (s != NULL && sp != NULL && free_melems >= pelems) {
			/*
			 * Same as above.
			 */
			SLABLIST_MOVE_PREV_TO_MID(sl, sp, s);
			move_to_next(sp, s);
			uls = sp;
			goto end;
		}

		if (free_nelems > 0 && sp != NULL && sn != NULL &&
		    free_nelems >= free_melems) {
			SLABLIST_MOVE_PREV_TO_NEXT(sl, sp, sn);
			move_to_next(sp, sn);
			uls = sp;
			goto end;
		}

		if (free_melems > 0 && sp != NULL && s != NULL &&
		    free_melems >= free_nelems) {
			SLABLIST_MOVE_PREV_TO_MID(sl, sp, s);
			move_to_next(sp, s);
			uls = sp;
			goto end;
		}

		if (free_nelems > 0 && sp != NULL && sn != NULL &&
		    free_nelems < free_melems) {
			SLABLIST_MOVE_PREV_TO_NEXT(sl, sp, sn);
			move_to_next(sp, sn);
			uls = sp;
			goto end;
		}

		if (free_melems > 0 && sp != NULL && s != NULL &&
		    free_melems < free_nelems) {
			SLABLIST_MOVE_PREV_TO_MID(sl, sp, s);
			move_to_next(sp, s);
			uls = sp;
			goto end;
		}



	} else if (free_pelems + free_melems >= nelems) {
		/*
		 * We can consolidate the slabs by moving elements from the
		 * next slab into the previous and middle slabs.
		 */
		if (sp != NULL && sn != NULL && free_pelems >= nelems) {
			/*
			 * We see if we can move all elems into a single slab.
			 */
			SLABLIST_MOVE_NEXT_TO_PREV(sl, sn, sp);
			move_to_prev(sn, sp);
			uls = sn;
			goto end;
		}

		if (s != NULL && sn != NULL && free_melems >= nelems) {
			/*
			 * Same as above.
			 */
			SLABLIST_MOVE_NEXT_TO_MID(sl, sn, s);
			move_to_prev(sn, s);
			uls = sn;
			goto end;
		}

		if (free_pelems > 0 && sp != NULL && sn != NULL &&
		    free_pelems >= free_melems) {
			SLABLIST_MOVE_NEXT_TO_PREV(sl, sn, sp);
			move_to_prev(sn, sp);
			uls = sn;
			goto end;
		}

		if (free_melems > 0 && s != NULL && sn != NULL &&
		    free_melems >= free_pelems) {
			SLABLIST_MOVE_NEXT_TO_MID(sl, sn, s);
			move_to_prev(sn, s);
			uls = sn;
			goto end;
		}

		if (free_pelems > 0 && sp != NULL && sn != NULL &&
		    free_pelems < free_melems) {
			SLABLIST_MOVE_NEXT_TO_PREV(sl, sn, sp);
			move_to_prev(sn, sp);
			uls = sn;
			goto end;
		}

		if (free_melems > 0 && s != NULL && sn != NULL &&
		    free_melems < free_pelems) {
			SLABLIST_MOVE_NEXT_TO_MID(sl, sn, s);
			move_to_prev(sn, s);
			uls = sn;
			goto end;
		}

	}

end:;

	if (sl->sl_head == uls) {
		sl->sl_head = uls->s_next;
	}

	if (uls != NULL) {
		unlink_slab(uls);
		rm_slab(uls);
		SLABLIST_SLAB_RM(sl);
	}


	return (uls);
}

/*
 * This function, given an index (0..SELEM_MAX), removes the elment at that
 * index and shifts all the elements _after_ that element to the left.
 */
static void
remove_elem(int i, slab_t *s)
{
	slablist_t *sl = s->s_list;


	SLABLIST_BWDSHIFT_BEGIN(sl, s, i);
	size_t sz = 8 * (s->s_elems - (i + 1));
	if (i != s->s_elems - 1) {
		/*
		 * If i is not the last element, we do a bwdshift. But if it
		 * is, we only have to decrement s_elems.
		 */
		bcopy(&(s->s_arr[(i + 1)]), &(s->s_arr[i]), sz);
	}
	SLABLIST_BWDSHIFT_END();
	s->s_elems--;
	SLABLIST_SLAB_DEC_ELEMS(s);

	slab_t *sm = NULL;
	if (i == 0) {
		if (SLIST_SUBLAYER(sl->sl_flags)) {
			sm = (slab_t *)s->s_arr[0];
			s->s_min = sm->s_min;
		} else {
			s->s_min = s->s_arr[0];
		}
		SLABLIST_SLAB_SET_MIN(s);
	}

	if (i == (s->s_elems)) {
		if (SLIST_SUBLAYER(sl->sl_flags)) {
			sm = (slab_t *)s->s_arr[(i - 1)];
			s->s_max = sm->s_max;
		} else {
			s->s_max = s->s_arr[(i - 1)];
		}
		SLABLIST_SLAB_SET_MAX(s);
	}
}

/*
 * Removing an element from a slablist, may result in the freeing of a slab. If
 * the slablist has sublayers, we need to "ripple" through those sublayers,
 * removing any references to the slab that has been freed.
 */
void
ripple_rem_to_sublayers(slablist_t *sl, slab_t *r, slab_t **l)
{
	int nu = sl->sl_sublayers;
	slablist_t *csl = sl;
	int cu = 0;
	int bc = 0;
	slab_t *sp;
	slab_t *sn;
	bc += (nu - 1);
	int i;
	/*
	 * In this loop we update the sublayers by removing references to the
	 * removed slab (r).
	 */
	while (r != NULL && cu < nu) {
		SLABLIST_RIPPLE_REM_SLAB(sl, r, l[bc]);
		csl = csl->sl_sublayer;
		sp = l[bc]->s_prev;
		sn = l[bc]->s_next;
		i = sublayer_slab_ptr_srch((uintptr_t)r, l[bc], 0);
		if (i != -1) {
			/* clearly r is ref'd in this slab... */
			remove_elem(i, l[bc]);
			r = slab_generic_rem(l[bc]);
			goto try_setbc;
		}
		i = sublayer_slab_ptr_srch((uintptr_t)r, sn, 0);
		if (i != -1) {
			/* clearly r is ref'd in next slab ... */
			remove_elem(i, sn);
			r = slab_generic_rem(sn);
			goto try_setbc;
		}
		i = sublayer_slab_ptr_srch((uintptr_t)r, sp, 0);
		if (i != -1) {
			/* clearly r is ref'd in prev slab ... */
			remove_elem(i, sp);
			r = slab_generic_rem(sp);
			goto try_setbc;
		}

try_setbc:;
		if (r == l[bc]) {
			/*
			 * If we removed l[bc] we need to update the l[bc] to
			 * point to an adjacent slab, if possible.
			 */
			if (sn != NULL) {
				l[bc] = sn;
			} else if (sp != NULL) {
				l[bc] = sp;
			}
			SLABLIST_SET_CRUMB(sl, l[bc], bc);
		}
		csl->sl_elems--;
		SLABLIST_SL_DEC_ELEMS(csl);
		cu++;
		bc--;
	}
	/*
	 * Now we update the extrema.
	 */
	bc = nu;
	ripple_update_extrema(l, bc);
}


/*
 * This function tries to reap slabs once the elems/slabs ratio in a slablist
 * reaches a user-defined minimum. A reap tries to cram all of the elements so
 * that as many slabs from the beginning of the slablist as possible are full.
 * All of the empty slabs should be at the back of the slablist. Optionally,
 * one partial slab should be between the first empty slab and the last full
 * slab.
 */
void
slablist_reap(slablist_t *sl)
{
	if (sl->sl_is_small_list) {
		return;
	}

	double es = (double)sl->sl_elems;
	double ss = (double)sl->sl_slabs;
	/* the minimum number of elems with 1 partial-slab */
	double mxe = (ss*(double)SELEM_MAX) - (double)(SELEM_MAX - 1);
	/* the maximum efficiency with 1 partial-slab */
	double mxss = mxe / (ss * (double)SELEM_MAX);
	double cratio = es / (ss * SELEM_MAX);
	if (ss == 1 || mxss <= cratio) {
		/*
		 * We check to see if reaping will actually save us space. If
		 * the maximum efficiency is greater than the current
		 * efficiency, we do the reap. If not, we return.
		 */
		return;
	}

	/*
	 * For now, we iterate over all slabs. but this is not neccessary.  We
	 * just have to iterate over all the slabs between the first and last
	 * partial slabs, inclusive.
	 */
	SLABLIST_REAP_BEGIN(sl);
	slab_t *s = sl->sl_head;
	slab_t **bc = mk_bc();
	slab_t *sn = NULL;
	slab_t *rmd;
	uintptr_t min;
	uint64_t i = 0;
	while (i < (sl->sl_slabs - 1)) {
		rmd = NULL;
		sn = s->s_next;
		min = sn->s_min;
		if (s->s_elems < SELEM_MAX) {
			if (sl->sl_sublayers) {
				/*
				 * We build a bread crumb path so that we can
				 * ripple changes down after we 1) deallocate
				 * the slab and 2) change its extrema.
				 */
				find_bubble_up(sl, min, bc);
			}
			move_to_prev(sn, s);
			if (sn->s_elems == 0) {
				/*
				 * We unlink and free the slab.
				 */
				unlink_slab(sn);
				rm_slab(sn);
				rmd = sn;
				SLABLIST_SLAB_RM(sl);

			}
			if (sl->sl_sublayers) {
				/*
				 * If we have sublayers, we ripple the changes
				 * down, using the bread crumb path created in
				 * the previous if-stmt.
				 */
				ripple_rem_to_sublayers(sl, rmd, bc);
			}
		}
		s = s->s_next;
		i++;
	}
	rm_bc(bc);
	SLABLIST_REAP_END(sl);
}

/*
 * Remove the pos'th element in this slablist. Or remove the element that is ==
 * to `elem`. Note that this function does _not_ deallocate the memory that
 * backs large objects.
 */
int
slablist_rem(slablist_t *sl, uintptr_t elem, uint64_t pos, uintptr_t *rdl)
{
	uint64_t off_pos;
	slab_t *s = NULL;
	slab_t **bc;
	int i;

	if (!(sl->sl_is_small_list) && sl->sl_elems == SMELEM_MAX) {
		/*
		 * If we have lowered the number of elems to 1/2 a slab, we
		 * turn the slab into a small linked list.
		 */
		slab_to_small_list(sl);
	}


	if (sl->sl_is_small_list) {
		/*
		 * We are dealing with a small list and have to remove the
		 * element.
		 */
		SLABLIST_REM_BEGIN(sl, elem, pos);
		int ret = small_list_rem(sl, elem, pos, rdl);
		SLABLIST_REM_END(ret);
		return (ret);
	}


	test_slab_consistency(sl);

	if (SLIST_SORTED(sl->sl_flags)) {

		SLABLIST_REM_BEGIN(sl, elem, pos);

		test_slab_sorting(sl);

		if (sl->sl_sublayers) {

			bc = mk_bc();
			find_bubble_up(sl, elem, bc);
			s = bc[sl->sl_sublayers];

			if (SLABLIST_TEST_BREAD_CRUMBS_ENABLED()) {
				uint64_t bcn = sl->sl_sublayers;
				int l;
				int f = test_breadcrumbs(bc, &l, bcn);
				SLABLIST_TEST_BREAD_CRUMBS(f, l);
			}

		} else {

			find_linear_scan(sl, elem, &s);

		}

		i = slab_srch(elem, s, 0);

		if (sl->sl_cmp_elem(s->s_arr[i], elem) != 0) {
			/*
			 * If the element was not found, we have nothing to
			 * remove, and return.
			 */
			if (rdl != NULL) {
				*rdl = NULL;
			}

			if (sl->sl_sublayers) {
				rm_bc(bc);
			}
			SLABLIST_REM_END(SL_ENFOUND);
			return (SL_ENFOUND);
		}

	} else {

		SLABLIST_REM_BEGIN(sl, elem, pos);

		s = slab_get(sl, pos, &off_pos, SLAB_VAL_POS);
		i = pos - off_pos;

	}

	if (rdl != NULL) {
		*rdl = s->s_arr[i];
	}

	remove_elem(i, s);

	slab_t *remd = NULL;
	remd = slab_generic_rem(s);
	if (sl->sl_sublayers) {
		ripple_rem_to_sublayers(sl, remd, bc);
		test_sublayers(sl, elem);
	}

	if (sl->sl_sublayers) {
		rm_bc(bc);
	}

	slablist_t *subl = get_lowest_sublayer(sl);
	slablist_t *supl = subl->sl_superlayer;
	if (subl != sl && supl->sl_slabs < sl->sl_req_sublayer) {
		/*
		 * If the lowest sublayer's superlayer has < sl_req_sublayer,
		 * the lowest sublayer is not needed. We remove it.
		 */
		detach_sublayer(supl);
	}

	try_reap_all(sl);

	sl->sl_elems--;
	SLABLIST_SL_DEC_ELEMS(sl);
	SLABLIST_REM_END(SL_SUCCESS);
	return (SL_SUCCESS);
}
