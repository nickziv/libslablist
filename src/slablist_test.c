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
#include "slablist_impl.h"
#include "slablist_find.h"
#include "slablist_provider.h"
#include "slablist_cons.h"

#define	E_TEST_SLAB_NULL		1
#define	E_TEST_SLAB_LIST_NULL		2
#define	E_TEST_SLAB_SUBLAYER		3
#define	E_TEST_SLAB_EXTREMA		4
#define	E_TEST_SLAB_UNSORTED		5
#define	E_TEST_INS_ELEM_INDEX		6
#define	E_TEST_INS_ELEM_OUT_ORD		7
#define	E_TEST_SLAB_BSRCH		8
#define	E_TEST_REM_ELEM_EMPTY		9
#define	E_TEST_REM_ELEM_BEYOND		10
#define	E_TEST_SLAB_PREV		11
#define	E_TEST_SLAB_NEXT		12

#define	E_TEST_SUBSLAB_NULL		13
#define	E_TEST_SUBSLAB_LIST_NULL	14
#define	E_TEST_SUBSLAB_SUBARR_NULL	15
#define	E_TEST_SUBSLAB_TOPLAYER		16
#define	E_TEST_SUBSLAB_UNSORTED		17
#define	E_TEST_SUBSLAB_ELEM_NULL	18
#define	E_TEST_SUBSLAB_MIN		19
#define	E_TEST_SUBSLAB_MAX		20
#define	E_TEST_INS_SLAB_INDEX		21
#define	E_TEST_INS_SLAB_OUT_ORD		22
#define	E_TEST_SUBSLAB_BSRCH		23
#define	E_TEST_SUBSLAB_BSRCH_TOP	24
#define	E_TEST_REM_SLAB_EMPTY		25
#define	E_TEST_REM_SLAB_BEYOND		26
#define	E_TEST_RPLA_SUBSLAB_HAS_NOT	27
#define	E_TEST_RPLA_SLAB_HAS_NOT	28
#define	E_TEST_SUBSLAB_REFERENCES	29
#define	E_TEST_SUBSLAB_PREV		30
#define	E_TEST_SUBSLAB_NEXT		31
#define	E_TEST_INS_SLAB_LAYER		32
#define	E_TEST_INS_SUBSLAB_LAYER	33
#define	E_TEST_SUBSLAB_MOVE_NEXT_SCP	34
#define	E_TEST_SUBSLAB_MOVE_NEXT_SNCP	35
#define	E_TEST_SUBSLAB_MOVE_PREV_SCP	36
#define	E_TEST_SUBSLAB_MOVE_PREV_SPCP	37
#define	E_TEST_SLAB_MOVE_NEXT_SCP	38
#define	E_TEST_SLAB_MOVE_NEXT_SNCP	39
#define	E_TEST_SLAB_MOVE_PREV_SCP	40
#define	E_TEST_SLAB_MOVE_PREV_SPCP	41
#define	E_TEST_SUBSLAB_ARR_MIN		42
#define	E_TEST_SUBSLAB_ARR_MAX		43
#define	E_TEST_SUBSLAB_USR_ELEMS_OVER	44
#define	E_TEST_SUBSLAB_USR_ELEMS_UNDER	45
#define	E_TEST_ELEM_POS			46
#define	E_TEST_SLAB_BELOW		47
#define	E_TEST_FBU_NOT_LAYERED		48

int
test_slab_get_elem_pos(slablist_t *sl, slab_t *s, slab_t **f, uint64_t pos,
    uint64_t op, uint64_t *opbptr)
{
	uint64_t off;
	slab_t *s2 = slab_get_elem_pos_old(sl, pos, &off);
	if (s2 != s || (pos - off - 1) != op) {
		*f = s2;
		*opbptr = off;
		return (E_TEST_ELEM_POS);
	}
	return (0);
}

int
test_slab_to_sml(slablist_t *sl, slab_t *s)
{
	uint64_t elems = sl->sl_elems;
	uint64_t i = 0;
	small_list_t *sml = sl->sl_head;
	while (i < elems) {
		if (sl->sl_cmp_elem(s->s_arr[i], sml->sml_data) != 0) {
			return (1);
		}
		sml = sml->sml_next;
		i++;
	}

	return (0);
}

int
test_smlist_nelems(slablist_t *sl)
{

	uint64_t i = 0;
	small_list_t *sml = sl->sl_head;



	if (sl->sl_elems) {
		while (i < (sl->sl_elems - 1)) {
			sml = sml->sml_next;
			if (sml == NULL) {
				return (1);
			}
			i++;
		}
	} else {
		if (sml != NULL) {
			return (1);
		}
	}

	return (0);
}

int
test_smlist_elems_sorted(slablist_t *sl)
{
	uint64_t i = 0;
	small_list_t *sml = sl->sl_head;
	small_list_t *prev = NULL;

	if (sl->sl_elems <= 1 || !(SLIST_SORTED(sl->sl_flags))) {
		return (0);
	}

	slablist_elem_t e1;
	slablist_elem_t e2;

	while (i < (sl->sl_elems - 1)) {
		prev = sml;
		sml = sml->sml_next;
		e1 = prev->sml_data;
		e2 = sml->sml_data;
		if (sl->sl_cmp_elem(e1, e2) > 0) {
			return (1);
		}
		i++;
	}

	return (0);
}

slab_t *
get_first_slab(subslab_t *baseslab)
{
	int layer = 0;
	int layers = baseslab->ss_list->sl_layer;
	subslab_t *s = baseslab;
	while (layer < layers - 1) {
		s = GET_SUBSLAB_ELEM(s, 0);
		layer++;
	}
	slab_t *sf = GET_SUBSLAB_ELEM(s, 0);
	return (sf);
}

slab_t *
get_last_slab(subslab_t *baseslab)
{
	int layer = 0;
	int layers = baseslab->ss_list->sl_layer;
	subslab_t *s = baseslab;
	int last;
	while (layer < layers - 1) {
		last = s->ss_elems - 1;
		s = GET_SUBSLAB_ELEM(s, last);
		layer++;
	}
	last = s->ss_elems - 1;
	slab_t *sl = GET_SUBSLAB_ELEM(s, last);
	return (sl);
}

int
test_slab_extrema(slab_t *s)
{

	uint32_t elems = s->s_elems;

	/* test that extrema match the actual elems in the array */
	if ((s->s_min.sle_u != s->s_arr[0].sle_u ||
	    s->s_max.sle_u != s->s_arr[(elems - 1)].sle_u)) {
		return (E_TEST_SLAB_EXTREMA);
	}

	return (0);
}

int
test_subslab_usr_elems(subslab_t *ss)
{
	uint64_t i = 0;
	uint64_t sum = 0;
	subslab_t *sref = NULL;
	slab_t *ref = NULL;
	if (ss->ss_list->sl_layer > 1) {
		while (i < ss->ss_elems) {
			sref = GET_SUBSLAB_ELEM(ss, i);
			sum += sref->ss_usr_elems;
			i++;
		}
	} else {
		while (i < ss->ss_elems) {
			ref = GET_SUBSLAB_ELEM(ss, i);
			sum += ref->s_elems;
			i++;
		}
	}
	SLABLIST_SUM_USR_ELEMS(sum);
	if (sum < ss->ss_usr_elems) {
		return (E_TEST_SUBSLAB_USR_ELEMS_OVER);
	}
	if (sum > ss->ss_usr_elems) {
		return (E_TEST_SUBSLAB_USR_ELEMS_UNDER);
	}
	return (0);
}

int
test_subslab_extrema(subslab_t *ss)
{
	slab_t *f = get_first_slab(ss);
	SLABLIST_EXTREME_SLAB(f);
	slablist_t *top_lyr = f->s_list;

	if (top_lyr->sl_cmp_elem(ss->ss_min, f->s_min) != 0) {
		return (E_TEST_SUBSLAB_MIN);
	}

	if (top_lyr->sl_cmp_elem(ss->ss_min, f->s_arr[0]) != 0) {
		return (E_TEST_SUBSLAB_ARR_MIN);
	}


	slab_t *l = get_last_slab(ss);
	SLABLIST_EXTREME_SLAB(l);
	uint32_t lelems = l->s_elems;
	if (top_lyr->sl_cmp_elem(ss->ss_max, l->s_max) != 0) {
		return (E_TEST_SUBSLAB_MAX);
	}

	if (top_lyr->sl_cmp_elem(ss->ss_max, l->s_arr[(lelems - 1)]) != 0) {
		return (E_TEST_SUBSLAB_ARR_MAX);
	}

	return (0);
}

int
test_slab(slab_t *s)
{
	/* test that the slab is not NULL */
	if (s == NULL) {
		return (E_TEST_SLAB_NULL);
	}

	/* test that the list back-ptr is not NULL */
	if (s->s_list == NULL) {
		return (E_TEST_SLAB_LIST_NULL);
	}

	slablist_t *sl = s->s_list;

	uint32_t elems = s->s_elems;

	if (elems == 0) {
		return (0);
	}

	/* test that the slab is at the top layer */
	if (sl->sl_layer != 0) {
		return (E_TEST_SLAB_SUBLAYER);
	}

	/* test that this slab has a pointer to its subslab */
	if (sl->sl_sublayers && s->s_below == NULL) {
		return (E_TEST_SLAB_BELOW);
	}

	int f = test_slab_extrema(s);
	if (f) {
		return (f);
	}

	/* test that elems are sorted within a slab */
	if (elems > 1) {
		uint64_t j = 0;
		elems = s->s_elems;
		while (j < (elems - 1)) {
			slablist_elem_t e1 = s->s_arr[j];
			slablist_elem_t e2 = s->s_arr[(j+1)];
			int c = sl->sl_cmp_elem(e1, e2);
			if (c > 0) {
				return (E_TEST_SLAB_UNSORTED);
			}
			j++;
		}
	}

	/*
	 * test that the slab is greater than the prev slab and less than the
	 * next slab
	 */
	if (s->s_prev != NULL) {
		if (sl->sl_cmp_elem(s->s_prev->s_max, s->s_min) > 0 ||
		    sl->sl_cmp_elem(s->s_prev->s_max, s->s_max) > 0 ||
		    sl->sl_cmp_elem(s->s_prev->s_min, s->s_min) > 0 ||
		    sl->sl_cmp_elem(s->s_prev->s_min, s->s_max) > 0) {
			return (E_TEST_SLAB_PREV);
		}
	}

	if (s->s_next != NULL) {
		if (sl->sl_cmp_elem(s->s_next->s_max, s->s_min) < 0 ||
		    sl->sl_cmp_elem(s->s_next->s_max, s->s_max) < 0 ||
		    sl->sl_cmp_elem(s->s_next->s_min, s->s_min) < 0 ||
		    sl->sl_cmp_elem(s->s_next->s_min, s->s_max) < 0) {
			return (E_TEST_SLAB_NEXT);
		}
	}

	return (0);
}

int
test_subslab(subslab_t *s)
{
	/* test that the subslab is not NULL */
	if (s == NULL) {
		return (E_TEST_SUBSLAB_NULL);
	}

	/* test that the list back-ptr is not NULL */
	if (s->ss_list == NULL) {
		return (E_TEST_SUBSLAB_LIST_NULL);
	}

	if (s->ss_arr == NULL) {
		return (E_TEST_SUBSLAB_SUBARR_NULL);
	}

	slablist_t *sl = s->ss_list;

	uint32_t elems = s->ss_elems;

	if (elems == 0) {
		return (0);
	}

	/* test that the subslab is not at the top layer */
	if (sl->sl_layer == 0) {
		return (E_TEST_SUBSLAB_TOPLAYER);
	}

	/* test that the elems are non-null */
	uint64_t j = 0;
	elems = s->ss_elems;
	while (j < elems) {
		slab_t *e = GET_SUBSLAB_ELEM(s, j);
		if (e == NULL) {
			return (E_TEST_SUBSLAB_ELEM_NULL);
		}
		j++;
	}

	/* test that elems are sorted within a subslab */
	if (elems > 1) {
		uint64_t j;
		uint64_t k;
		elems = s->ss_elems;
		if (sl->sl_layer == 1) {
			j = 0;
			while (j < (elems - 1)) {
				k = j + 1;
				slab_t *e1 = GET_SUBSLAB_ELEM(s, j);
				slab_t *e2 = GET_SUBSLAB_ELEM(s, k);
				int c = sl->sl_cmp_elem(e1->s_max, e2->s_max);
				if (c > 0) {
					return (E_TEST_SUBSLAB_UNSORTED);
				}
				j++;
			}

		} else {

			j = 0;
			while (j < (elems - 1)) {
				k = j + 1;
				subslab_t *se1 = GET_SUBSLAB_ELEM(s, j);
				subslab_t *se2 = GET_SUBSLAB_ELEM(s, k);
				int c = sl->sl_cmp_elem(se1->ss_max,
					    se2->ss_max);

				if (c > 0) {
					return (E_TEST_SUBSLAB_UNSORTED);
				}
				j++;
			}
		}
	}

	if (s->ss_prev != NULL) {
		if (sl->sl_cmp_elem(s->ss_prev->ss_max, s->ss_min) > 0 ||
		    sl->sl_cmp_elem(s->ss_prev->ss_max, s->ss_max) > 0 ||
		    sl->sl_cmp_elem(s->ss_prev->ss_min, s->ss_min) > 0 ||
		    sl->sl_cmp_elem(s->ss_prev->ss_min, s->ss_max) > 0) {
			return (E_TEST_SUBSLAB_PREV);
		}
	}

	if (s->ss_next != NULL) {
		if (sl->sl_cmp_elem(s->ss_next->ss_max, s->ss_min) < 0 ||
		    sl->sl_cmp_elem(s->ss_next->ss_max, s->ss_max) < 0 ||
		    sl->sl_cmp_elem(s->ss_next->ss_min, s->ss_min) < 0 ||
		    sl->sl_cmp_elem(s->ss_next->ss_min, s->ss_max) < 0) {
			return (E_TEST_SUBSLAB_NEXT);
		}
	}

	return (0);
}

int
test_subslab_ref(subslab_t *s)
{
	/* test that this subslab references all of the superslabs */
	uint64_t j = 0;
	uint64_t elems = s->ss_elems;
	if (s->ss_list->sl_layer == 1) {
		slab_t *curslab = GET_SUBSLAB_ELEM(s, 0);
		while (j < elems) {
			if (GET_SUBSLAB_ELEM(s, j) != curslab) {
				return (E_TEST_SUBSLAB_REFERENCES);
			}
			curslab = curslab->s_next;
			j++;
		}
	} else {
		j = 0;
		elems = s->ss_elems;
		subslab_t *cursubslab = GET_SUBSLAB_ELEM(s, 0);
		while (j < elems) {
			if (GET_SUBSLAB_ELEM(s, j) != cursubslab) {
				return (E_TEST_SUBSLAB_REFERENCES);
			}
			cursubslab = cursubslab->ss_next;
			j++;
		}
	}

	return (0);
}

int
test_slab_bin_srch(slablist_elem_t elem, slab_t *s)
{
	int f = test_slab(s);
	if (f != 0) {
		return (f);
	}

	/* test that binary search and linear search return the same index */
	int i1 = slab_bin_srch(elem, s);
	int i2 = slab_lin_srch(elem, s);
	if (i1 != i2) {
		return (E_TEST_SLAB_BSRCH);
	}

	return (0);
}

int
test_subslab_bin_srch(slablist_elem_t elem, subslab_t *s)
{
	int f = test_subslab(s);
	if (f != 0) {
		return (f);
	}

	/* test that binary search and linear search return the same index */
	int i1 = subslab_bin_srch(elem, s);
	int i2 = subslab_lin_srch(elem, s);
	if (i1 != i2) {
		return (E_TEST_SUBSLAB_BSRCH);
	}

	return (0);
}

int
test_subslab_bin_srch_top(slablist_elem_t elem, subslab_t *s)
{
	int f = test_subslab(s);
	if (f != 0) {
		return (f);
	}

	/* test that binary search and linear search return the same index */
	int i1 = subslab_bin_srch_top(elem, s);
	int i2 = subslab_lin_srch_top(elem, s);
	if (i1 != i2) {
		return (E_TEST_SUBSLAB_BSRCH_TOP);
	}

	return (0);
}

/*
 * This function tests that a slab is consistent within the context of the
 * add_elem() function.
 */
int
test_add_elem(slab_t *s, slablist_elem_t elem, uint64_t i)
{
	int f = test_slab(s);
	if (f != 0) {
		return (f);
	}

	slablist_t *sl = s->s_list;

	if (s->s_elems == 0 && i > 0) {
		return (E_TEST_INS_ELEM_INDEX);
	}

	/* test that the elem is being added into the right place */
	uint64_t elems = s->s_elems;

	if (elems == 0 || sl->sl_layer != 0) {
		return (0);
	}

	if (i > 0 && i <= (elems - 1)) {
		if (sl->sl_cmp_elem(elem, s->s_arr[i]) > 0 ||
		    sl->sl_cmp_elem(elem, s->s_arr[(i - 1)]) < 0) {
			return (E_TEST_INS_ELEM_OUT_ORD);
		}
	}

	if (i == elems) {
		if (sl->sl_cmp_elem(elem, s->s_arr[(i - 1)]) < 0) {
			return (E_TEST_INS_ELEM_OUT_ORD);
		}
	}

	if (i == 0) {
		if (sl->sl_cmp_elem(elem, s->s_arr[i]) > 0) {
			return (E_TEST_INS_ELEM_OUT_ORD);
		}
	}

	return (0);
}
/*
 * This function tests that a slab is consistent within the context of the
 * add_elem() function. Bear in mind that one and only one of `s1` or `s2`
 * _must_ be NULL.
 */
int
test_add_slab(subslab_t *s, slab_t *s1, subslab_t *s2, uint64_t i)
{
	int f = test_subslab(s);
	if (f != 0) {
		return (f);
	}

	slablist_t *sl = s->ss_list;

	if (s->ss_elems == 0 && i > 0) {
		return (E_TEST_INS_SLAB_INDEX);
	}

	/* test that the elem is being added into the right place */
	uint64_t elems = s->ss_elems;

	if (elems == 0) {
		return (0);
	}

	/*
	 * We can't add slab_t's into layers beneath the second layer.
	 */
	if (s1 != NULL && sl->sl_layer > 1) {
		return (E_TEST_INS_SLAB_LAYER);
	}

	/*
	 * We can't add subslab_t's into the first or second layer.
	 */
	if (s2 != NULL && sl->sl_layer < 2) {
		return (E_TEST_INS_SUBSLAB_LAYER);
	}

	int h = i - 1;
	if (s1 == NULL) {
		goto ss_test;
	}

	slab_t *next = GET_SUBSLAB_ELEM(s, i);
	slab_t *prev = GET_SUBSLAB_ELEM(s, h);
	if (i > 0 && i <= (elems - 1)) {
		if (sl->sl_cmp_elem(s1->s_max, next->s_max) > 0 ||
		    sl->sl_cmp_elem(s1->s_max, prev->s_max) < 0) {
			return (E_TEST_INS_SLAB_OUT_ORD);
		}
	}

	if (i == elems && elems > 0) {
		if (sl->sl_cmp_elem(s1->s_max, prev->s_max) < 0) {
			return (E_TEST_INS_SLAB_OUT_ORD);
		}
	}

	if (i == 0 && elems > 0) {
		if (sl->sl_cmp_elem(s1->s_max, next->s_max) > 0) {
			return (E_TEST_INS_SLAB_OUT_ORD);
		}
	}

	return (0);

ss_test:;
	/*
	 * We put this superfluous check here, so that clang's static analyzer
	 * keeps quiet.
	 */
	if (s2 == NULL) {
		return (0);
	}
	subslab_t *snext = GET_SUBSLAB_ELEM(s, i);
	subslab_t *sprev = GET_SUBSLAB_ELEM(s, h);
	if (i > 0 && i <= (elems - 1)) {
		if (sl->sl_cmp_elem(s2->ss_max, snext->ss_max) > 0 ||
		    sl->sl_cmp_elem(s2->ss_max, sprev->ss_max) < 0) {
			return (E_TEST_INS_SLAB_OUT_ORD);
		}
	}

	if (i == elems && elems > 0) {
		if (sl->sl_cmp_elem(s2->ss_max, sprev->ss_max) < 0) {
			return (E_TEST_INS_SLAB_OUT_ORD);
		}
	}

	if (i == 0 && elems > 0) {
		if (sl->sl_cmp_elem(s2->ss_max, snext->ss_max) > 0) {
			return (E_TEST_INS_SLAB_OUT_ORD);
		}
	}

	return (0);
}

/*
 * This function tests that a slab is consistent within the context of the
 * remove_elem() function.
 */
int
test_remove_elem(uint64_t i, slab_t *s)
{
	int f = test_slab(s);
	if (f != 0) {
		return (f);
	}
	if (i > SELEM_MAX - 1) {
		return (E_TEST_REM_ELEM_BEYOND);
	}
	return (0);
}

int
test_remove_slab(uint64_t i, subslab_t *s)
{
	int f = test_subslab(s);
	if (f != 0) {
		return (f);
	}
	if (i > SUBELEM_MAX - 1) {
		return (E_TEST_REM_SLAB_BEYOND);
	}
	return (0);
}

int
test_ripple_add_subslab(subslab_t *new, int bc)
{
	int f = test_subslab(new);
	if (f != 0) {
		return (f);
	}

	if (bc != 0) {
		subslab_t *sub = new->ss_below;
		subslab_t *s = new;
		int i = 0;
		int has = 0;
		while (i < sub->ss_elems) {
			if ((subslab_t *)GET_SUBSLAB_ELEM(sub, i) == s) {
				has = 1;
			}
			i++;
		}

		if (!has) {
			return (E_TEST_RPLA_SUBSLAB_HAS_NOT);
		}
	}
	return (0);
}

int
test_ripple_add_slab(slab_t *new, slab_t *modified, int bc)
{
	int f = test_slab(new);
	if (f != 0) {
		return (f);
	}

	subslab_t *sub = modified->s_below;
	if (bc != 0) {
		slab_t *s = modified;
		int i = 0;
		int has = 0;
		while (i < sub->ss_elems) {
			if ((slab_t *)GET_SUBSLAB_ELEM(sub, i) == s) {
				has = 1;
			}
			i++;
		}

		if (!has) {
			return (E_TEST_RPLA_SLAB_HAS_NOT);
		}
	}
	return (0);
}

int
test_rem_range(slab_t *s)
{
	int f;
	f = test_slab(s);
	return (f);
}

int
test_rem_range_sub_slim(subslab_t *s)
{
	int f;
	f = test_subslab_usr_elems(s);
	if (f != 0) {
		return (f);
	}
	return (0);
}

int
test_rem_range_sub(subslab_t *s)
{
	int f;
	f = test_subslab(s);
	if (f != 0) {
		return (f);
	}
	f = test_subslab_extrema(s);
	if (f != 0) {
		return (f);
	}
	f = test_subslab_usr_elems(s);
	if (f != 0) {
		return (f);
	}
	return (0);
}


int
test_find_bubble_up(subslab_t *found, slab_t *sbptr, slablist_elem_t elem)
{
	int f;
	if (sbptr == NULL) {
		subslab_t *ss = found;
		slablist_t *sl = ss->ss_list;
		f = test_subslab(ss);
		if (f != 0) {
			return (f);
		}
		f = test_subslab_ref(ss);
		if (f != 0) {
			return (f);
		}
		f = test_subslab_extrema(ss);
		if (f != 0) {
			return (f);
		}
		f = test_subslab_usr_elems(ss);
		if (f != 0) {
			return (f);
		}

		if (sl->sl_layer > 1) {
			test_subslab_bin_srch(elem, ss);
		} else {
			test_subslab_bin_srch_top(elem, ss);
		}

	} else {

		slab_t *s = sbptr;
		f = test_slab(s);
		if (f != 0) {
			return (f);
		}
		f = test_slab_bin_srch(elem, s);
		if (f != 0) {
			return (f);
		}
	}

	return (0);
}


int
test_slab_move_next(slab_t *scp, slab_t *sn, slab_t *sncp, int *i)
{
	uint64_t cpable = SELEM_MAX - sncp->s_elems;
	uint64_t tocp = 0;
	int from = 0;

	if (scp->s_elems >= cpable) {
		tocp = cpable;
		from = scp->s_elems - tocp;
	}


	/*
	 * loops that check that elem scp[from] -> sncp[end] ==
	 * the elem sn[0] to sn[NELEMS]. One loop per slab.
	 */
	int j = 0;
	int k = from;
	while (k < scp->s_elems) {
		/*
		 * We start at `scp` and check all the elements against the
		 * _copied_ elements that are stored in `sn`.
		 */
		if (scp->s_arr[k].sle_u != sn->s_arr[j].sle_u) {
			*i = j;
			return (E_TEST_SLAB_MOVE_NEXT_SCP);
		}
		j++;
		k++;
	}
	k = 0;
	while (k < sncp->s_elems) {
		/*
		 * We now continue to `sncp` and check all the elements against
		 * the _original_ elements that are stored in `sn`.
		 */
		if (sncp->s_arr[k].sle_u != sn->s_arr[j].sle_u) {
			*i = j;
			return (E_TEST_SLAB_MOVE_NEXT_SNCP);
		}
		j++;
		k++;
	}

	return (0);
}

int
test_slab_move_prev(slab_t *scp, slab_t *sp, slab_t *spcp, int *i)
{
	uint64_t cpable = SELEM_MAX - spcp->s_elems;
	uint64_t tocp = 0;
	int from = scp->s_elems - 1;

	if (scp->s_elems >= cpable) {
		tocp = cpable;
		from = tocp - 1;
	}

	/*
	 * loops that check that elem spcp[from] -> scp[0] == the elem sp[0]
	 * to s[NELEMS]. One loop per slab.
	 */
	int j = sp->s_elems - 1;
	int k = from;
	/*
	 * We start from `scp` and work backwards from `from` to 0. We check
	 * against all of the elements that have been _copied_ into `sp`.
	 */
	while (k >= 0) {
		if (scp->s_arr[k].sle_u != sp->s_arr[j].sle_u) {
			*i = j;
			return (E_TEST_SLAB_MOVE_PREV_SCP);
		}
		j--;
		k--;
	}
	k = spcp->s_elems - 1;
	/*
	 * We then continue to `spcp` and check that against the _original_
	 * elems.
	 */
	while (k >= 0) {
		if (spcp->s_arr[k].sle_u != sp->s_arr[j].sle_u) {
			*i = j;
			return (E_TEST_SLAB_MOVE_PREV_SPCP);
		}
		j--;
		k--;
	}

	return (0);
}

int
test_subslab_move_next(subslab_t *scp, subslab_t *sn,
    subslab_t *sncp, int *i)
{
	uint64_t cpable = SUBSLAB_FREE_SPACE(sncp);
	uint64_t tocp = 0;
	int from = 0;

	if (scp->ss_elems >= cpable) {
		tocp = cpable;
		from = scp->ss_elems - tocp;
	}


	/*
	 * loops that check that elem scp[from] -> sncp[end] ==
	 * the elem sn[0] to sn[NELEMS]. One loop per slab.
	 */
	int j = 0;
	int k = from;
	/*
	 * We start at `scp` and check all the elements against the _copied_
	 * elements that are stored in `sn`.
	 */
	while (k < scp->ss_elems) {
		if (GET_SUBSLAB_ELEM(scp, k) != GET_SUBSLAB_ELEM(sn, j)) {
			*i = j;
			return (E_TEST_SUBSLAB_MOVE_NEXT_SCP);
		}
		j++;
		k++;
	}
	k = 0;
	/*
	 * We now continue to `sncp` and check all the elements against the
	 * _original_ elements that are stored in `sn`.
	 */
	while (k < sncp->ss_elems) {
		if (GET_SUBSLAB_ELEM(sncp, k) != GET_SUBSLAB_ELEM(sn, j)) {
			*i = j;
			return (E_TEST_SUBSLAB_MOVE_NEXT_SNCP);
		}
		j++;
		k++;
	}

	return (0);
}

int
test_subslab_move_prev(subslab_t *scp, subslab_t *sp,
    subslab_t *spcp, int *i)
{
	uint64_t cpable = SUBSLAB_FREE_SPACE(spcp);
	uint64_t tocp = 0;
	int from = scp->ss_elems - 1;

	if (scp->ss_elems >= cpable) {
		tocp = cpable;
		from = tocp - 1;
	}


	/*
	 * loops that check that elem spcp[from] -> scp[0] == the elem sp[0]
	 * to s[NELEMS]. One loop per slab.
	 */
	int j = sp->ss_elems - 1;
	int k = from;
	/*
	 * We start from `scp` and work backwards from `from` to 0. We check
	 * against all of the elements that have been _copied_ into `sp`.
	 */
	while (k >= 0) {
		if (GET_SUBSLAB_ELEM(scp, k) != GET_SUBSLAB_ELEM(sp, j)) {
			*i = j;
			return (E_TEST_SUBSLAB_MOVE_PREV_SCP);
		}
		j--;
		k--;
	}
	k = spcp->ss_elems - 1;
	/*
	 * We then continue to `spcp` and check that against the _original_
	 * elems.
	 */
	while (k >= 0) {
		if (GET_SUBSLAB_ELEM(spcp, k) != GET_SUBSLAB_ELEM(sp, j)) {
			*i = j;
			return (E_TEST_SUBSLAB_MOVE_PREV_SPCP);
		}
		j--;
		k--;
	}

	return (0);
}
