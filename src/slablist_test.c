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

#define TEST_SLAB_ERR_MAX	6

int
test_slab_to_sml(slablist_t *sl, slab_t *s)
{
	uint64_t elems = sl->sl_elems;
	int i = 0;
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

	uintptr_t e1;
	uintptr_t e2;

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
get_first_slab(slab_t *baseslab)
{
        slab_t *s = baseslab;
        int layers = baseslab->s_list->sl_layer;
        int layer = 0;
	bc_t bc;
	bc.bc_slab = s;
	bc.bc_on_edge = ON_LEFT_EDGE;
	SLABLIST_GET_EXTREME_PATH(&bc, layer);
        while (layer < layers) {
		s = (slab_t *)s->s_arr[0];
		bc.bc_slab = s;
		layer++;
		SLABLIST_GET_EXTREME_PATH(&bc, layer);
        }
        return (s);
}

slab_t *
get_last_slab(slab_t *baseslab)
{
        slab_t *s = baseslab;
        int layers = baseslab->s_list->sl_layer;
        int layer = 0;
	bc_t bc;
	bc.bc_slab = s;
	bc.bc_on_edge = ON_RIGHT_EDGE;
	SLABLIST_GET_EXTREME_PATH(&bc, layer);
        while (layer < layers) {
                uint32_t lelems = s->s_elems;
                s = (slab_t *)s->s_arr[(lelems - 1)];
		bc.bc_slab = s;
                layer++;
		SLABLIST_GET_EXTREME_PATH(&bc, layer);
        }
        return (s);
}

int
test_slab_extrema(slab_t *s)
{
	slablist_t *sl = s->s_list;

	uint32_t elems = s->s_elems;

	/* test that extrema match the actual elems in the array */
	if (sl->sl_layer == 0) {
		if ((s->s_min != s->s_arr[0] ||
		    s->s_max != s->s_arr[(elems - 1)])) {
			return (3);
		}

	} else {

		slab_t *f = get_first_slab(s);
		slablist_t *top_lyr = f->s_list;

		if (top_lyr->sl_cmp_elem(s->s_min, f->s_arr[0]) != 0 ){ 
			return (4);
		}

		slab_t *l = get_last_slab(s);
		uint32_t lelems = l->s_elems;
		if (top_lyr->sl_cmp_elem(s->s_max, l->s_arr[(lelems - 1)]) != 0) {
			return (5);
		}
	}

	return (0);
}

int
test_slab(slab_t *s)
{
	/* test that the slab is not NULL */
	if (s == NULL) {
		return (1);
	}

	/* test that the list back-ptr is not NULL */
	if (s->s_list == NULL) {
		return (2);
	}

	slablist_t *sl = s->s_list;

	uint32_t elems = s->s_elems;

	if (elems == 0) {
		return (0);
	}


	/* test that elems are sorted within a slab */
	if (sl->sl_layer == 0 && elems > 1) {
		int j = 0;
		elems = s->s_elems;
		while (j < (elems - 1)) {
			uintptr_t e1 = s->s_arr[j];
			uintptr_t e2 = s->s_arr[(j+1)];
			int c = sl->sl_cmp_elem(e1, e2);
			if (c > 0) {
				return (6);
			}
			j++;
		}
	}


	return (0);
}

extern int gen_bin_srch(uintptr_t, slab_t *, int);
extern int gen_lin_srch(uintptr_t, slab_t *, int);

int
test_slab_srch(uintptr_t elem, slab_t *s, int is_slab)
{
	int f = test_slab(s);
	if (f != 0) {
		return (f);
	}

	/* test that binary search and linear search return the same index */
	int i1 = gen_bin_srch(elem, s, is_slab);
	int i2 = gen_lin_srch(elem, s, is_slab);
	if (i1 != i2) {
		return (TEST_SLAB_ERR_MAX + 1);
	}

	return (0);
}

/*
 * This function tests that a slab is consistent within the context of the
 * insert_elem() function.
 */
int
test_insert_elem(uintptr_t elem, slab_t *s, int i)
{
	int f = test_slab(s);
	if (f != 0) {
		return (f);
	}

	slablist_t *sl = s->s_list;

	if (s->s_elems == 0 && i > 0) {
		return (TEST_SLAB_ERR_MAX + 1);
	}

	/* test that the elem is being inserted into the right place */
	uint64_t elems = s->s_elems;

	if (elems == 0 || sl->sl_layer != 0) {
		return (0);
	}

	if (i > 0 && i <= (elems - 1)) {
		if (sl->sl_cmp_elem(elem, s->s_arr[i]) > 0 ||
		    sl->sl_cmp_elem(elem, s->s_arr[(i - 1)]) < 0) {
			return (TEST_SLAB_ERR_MAX + 2);
		}
	}

	if (i == elems) {
		if (sl->sl_cmp_elem(elem, s->s_arr[(i - 1)]) < 0) {
			return (TEST_SLAB_ERR_MAX + 2);
		}
	}

	if (i == 0) {
		if (sl->sl_cmp_elem(elem, s->s_arr[i]) > 0) {
			return (TEST_SLAB_ERR_MAX + 2);
		}
	}

	return (0);
}

/*
 * This function tests that a slab is consistent within the context of the
 * remove_elem() function.
 */
int
test_remove_elem(int i, slab_t *s)
{
	int f = test_slab(s);
	if (f != 0) {
		return (f);
	}
	if (s->s_elems == 0) {
		return (TEST_SLAB_ERR_MAX + 1);
	}
	return (0);
}

int
test_ripple_add(slab_t *new, bc_t *crumbs, int bc)
{
	int f = test_slab(new);
	if (f != 0) {
		return (f);
	}

	if (bc != 0) {
		slab_t *sub = crumbs[(bc - 1)].bc_slab;
		slab_t *s = crumbs[bc].bc_slab;
		int i = 0;
		int has = 0;
		while (i < sub->s_elems) {
			if ((slab_t *)sub->s_arr[i] == s) {
				has = 1;
			}
			i++;
		}

		if (!has) {
			return (TEST_SLAB_ERR_MAX + 1);
		}
	}
	return (0);
}

int
test_find_bubble_up(int layers, bc_t *crumbs, int bc, uintptr_t elem)
{
	int f = test_slab(crumbs[(bc - 1)].bc_slab);
	if (f != 0) {
		return (f);
	}
	f = test_slab_extrema(crumbs[(bc - 1)].bc_slab);
	if (f != 0) {
		return (f);
	}

	int b = gen_bin_srch(elem, crumbs[(bc - 1)].bc_slab, 0);
	int l = gen_lin_srch(elem, crumbs[(bc - 1)].bc_slab, 0);
	if (b != l) {
		return (TEST_SLAB_ERR_MAX + 1);
	}
	return (0);
}


int
test_breadcrumbs(bc_t *bc, int *l, uint64_t bcn)
{
	int i = 0;
	int cl;
	int nl;
	while (i < (bcn - 1)) {
		slab_t *s = bc[i].bc_slab;
		slab_t *s1 = bc[(i + 1)].bc_slab;
		cl = s->s_list->sl_layer;
		nl = s1->s_list->sl_layer;
		if (cl <= nl) {
			*l = nl;
			return (1);
		}
		i++;
	}

	*l = nl;
	return (0);
}

int
test_move_next(slab_t *s, slab_t *scp, slab_t *sn, slab_t *sncp, int *i)
{
	uint64_t cpable = SELEM_MAX - sncp->s_elems;
	uint64_t tocp = 0;
	int from = 0;

	if (scp->s_elems >= cpable) {
		tocp = cpable;
		from = scp->s_elems - tocp;
	}

	if (scp->s_elems < cpable) {
		tocp = scp->s_elems;
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
		if (scp->s_arr[k] != sn->s_arr[j]) {
			*i = j;
			return (1);
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
		if (sncp->s_arr[k] != sn->s_arr[j]) {
			*i = j;
			return (2);
		}
		j++;
		k++;
	}

	return (0);
}

int
test_move_prev(slab_t *s, slab_t *scp, slab_t *sp, slab_t *spcp, int *i)
{
	uint64_t cpable = SELEM_MAX - spcp->s_elems;
	uint64_t tocp = 0;
	int from = scp->s_elems - 1;

	if (scp->s_elems >= cpable) {
		tocp = cpable;
		from = tocp - 1;
	}

	if (scp->s_elems < cpable) {
		tocp = scp->s_elems;
	}


	/*
	 * loops that check that elem spcp[from] -> scp[0] == the elem sp[0]
	 * to s[NELEMS]. One loop per slab.
	 */
	int j = sp->s_elems - 1;
	int k = from;
	while (k >= 0) {
		/*
		 * We start from `scp` and work backwards from `from` to 0. We
		 * check against all of the elements that have been _copied_
		 * into `sp`.
		 */
		if (scp->s_arr[k] != sp->s_arr[j]) {
			*i = j;
			return (1);
		}
		j--;
		k--;
	}
	k = spcp->s_elems - 1;
	while (k >= 0) {
		/*
		 * We then continue to `spcp` and check that against the
		 * _original_ elems.
		 */
		if (spcp->s_arr[k] != sp->s_arr[j]) {
			*i = j;
			return (2);
		}
		j--;
		k--;
	}

	return (0);
}
