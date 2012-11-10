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

	if (sl->sl_elems <= 1) {
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

int
test_slab_bkptr(slablist_t *sl)
{
	int i = 0;
	slab_t *s = NULL;
	while (i < sl->sl_slabs) {
		if (s == NULL) {
			s = sl->sl_head;
		} else {
			s = s->s_next;
		}
		if (s->s_list != sl) {
			return (1);
		}
		i++;
	}

	return (0);
}

int
test_slab_elems_sorted(slablist_t *sl)
{
	int i = 0;
	int j = 0;
	slab_t *s = NULL;
	while (i < sl->sl_slabs) {

		j = 0;
		if (s == NULL) {
			s = sl->sl_head;
		} else {
			s = s->s_next;
		}

		if (s->s_elems == 1) {
			return (0);
		}

		while (j < (s->s_elems - 1)) {
			uintptr_t e1 = s->s_arr[j];
			uintptr_t e2 = s->s_arr[(j+1)];
			int c = sl->sl_cmp_elem(e1, e2);
			if (c > 0) {
				return (1);
			}
			j++;
		}
		i++;
	}

	return (0);
}

int
test_slabs_sorted(slablist_t *sl, slab_t **sp1, slab_t **sp2)
{
	if (sl->sl_slabs < 2) {
		return (0);
	}

	slab_t *s1 = NULL;
	slab_t *s2 = NULL;
	int i = 0;
	while (i < (sl->sl_slabs - 1)) {
		if (s1 == NULL) {
			s1 = sl->sl_head;
		} else {
			s1 = s1->s_next;
		}
		s2 = s1->s_next;
		if (sl->sl_cmp_elem(s1->s_max, s2->s_max) > 0 ||
		    sl->sl_cmp_elem(s1->s_min, s2->s_min) > 0 ||
		    sl->sl_cmp_elem(s1->s_max, s2->s_min) > 0 ||
		    sl->sl_cmp_elem(s1->s_min, s2->s_max) > 0) {
			*sp1 = s1;
			*sp2 = s2;
			return (1);
		}
		i++;
	}

	return (0);
}

int
test_slab_extrema(slablist_t *sl, int *l, int *k, slab_t **sptr)
{
	int i = 0;
	slab_t *s = NULL;
	uint64_t e;
	int r = 0;
	int p = 0;
	if (SLIST_ORDERED(sl->sl_flags)) {
		return (0);
	}
	while (i < sl->sl_slabs) {
		if (s  == NULL) {
			s  = sl->sl_head;
		} else {
			s  = s->s_next;
		}
		e = s->s_elems;
		if (s->s_min != s->s_arr[0]) {
			r |= 1;
		}
		if (s->s_max != s->s_arr[(e - 1)]) {
			r |= 2;
		}

		if (sl->sl_cmp_elem(s->s_min, s->s_max) > 0) {
			p = 1;
		}
		if (r || p) {
			*l = r;
			*k = p;
			*sptr = s;
			return (1);
		}
		i++;
	}
	*l = 0;
	return (0);
}

int
test_nelems(slablist_t *sl, uint64_t *l, uint64_t *ll)
{
	if (sl->sl_is_small_list) {
		return (0);
	}
	int i = 0;
	slab_t *s = sl->sl_head;

	uint64_t sum = 0;
	while (i < sl->sl_slabs) {
		sum += s->s_elems;
		s = s->s_next;
		i++;
	}

	*l = sl->sl_elems;
	*ll = sum;

	if (sl->sl_elems == sum) {
		return (0);
	} else {
		return (1);
	}
}

int
test_nslabs(slablist_t *sl, uint64_t *l, uint64_t *ll)
{
	if (sl->sl_is_small_list) {
		return (0);
	}
	uint64_t i = 0;
	slab_t *s = sl->sl_head;

	while (i < sl->sl_slabs) {
		s = s->s_next;
		i++;
	}

	*l = sl->sl_slabs;
	*ll = i;

	if (sl->sl_slabs == i) {
		return (0);
	} else {
		return (1);
	}

}

int
test_slab_elems_max(slablist_t *sl, uint64_t *l)
{
	if (sl->sl_is_small_list) {
		return (0);
	}

	int i = 0;
	slab_t *s = sl->sl_head;

	while (i < sl->sl_slabs) {
		if (s->s_elems > SELEM_MAX) {
			*l = s->s_elems;
			return (1);
		}
		s = s->s_next;
		i++;
	}

	*l = 0;
	return (0);
}

void
test_slab_consistency(slablist_t *sl)
{
	if (!SLABLIST_TEST_ENABLED()) {
		return;
	}

	if (SLABLIST_TEST_SLAB_BKPTR_ENABLED()) {
		int f = test_slab_bkptr(sl);
		SLABLIST_TEST_SLAB_BKPTR(f);
	}

	if (SLABLIST_TEST_SLAB_EXTREMA_ENABLED()) {
		int l;
		int k;
		slab_t *s;
		int f = test_slab_extrema(sl, &l, &k, &s);
		SLABLIST_TEST_SLAB_EXTREMA(f, l, k, s);
	}


	if (SLABLIST_TEST_NELEMS_ENABLED()) {
		uint64_t l;
		uint64_t ll;
		int f = test_nelems(sl, &l, &ll);
		SLABLIST_TEST_NELEMS(f, l, ll);
	}

	if (SLABLIST_TEST_NSLABS_ENABLED()) {
		uint64_t l;
		uint64_t ll;
		int f = test_nslabs(sl, &l, &ll);
		SLABLIST_TEST_NSLABS(f, l, ll);
	}

	if (SLABLIST_TEST_SLAB_ELEMS_MAX_ENABLED()) {
		uint64_t l;
		int f = test_slab_elems_max(sl, &l);
		SLABLIST_TEST_SLAB_ELEMS_MAX(f, l);
	}
}

void
test_slab_sorting(slablist_t *sl)
{
	if (!SLABLIST_TEST_ENABLED()) {
		return;
	}

	if (SLABLIST_TEST_SLAB_ELEMS_SORTED_ENABLED()) {
		int f = test_slab_elems_sorted(sl);
		SLABLIST_TEST_SLAB_ELEMS_SORTED(f);
	}

	if (SLABLIST_TEST_SLABS_SORTED_ENABLED()) {
		slab_t *s1;
		slab_t *s2;
		int f = test_slabs_sorted(sl, &s1, &s2);
		SLABLIST_TEST_SLABS_SORTED(f, s1, s2);
	}
}


int
test_bubble_up(slablist_t *sl, uintptr_t elem, int *l)
{
	slab_t *sls = NULL;
	slab_t **bc = mk_buf(8 * 256);
	int fs = find_linear_scan(sl, elem, &sls);
	int bs = find_bubble_up(sl, elem, bc);
	if (fs == bs && bc[(sl->sl_sublayers)] == sls) {
		rm_buf(bc, 8 * 256);
		return (0);
	}
	int ret = 0;
	if (fs != bs) {
		ret = 1;
	}

	if (sls != bc[(sl->sl_sublayers)]) {
		if (ret == 1) {
			ret = 3;
		} else {
			ret = 2;
		}
	}

	*l = ret;
	rm_buf(bc, 8 * 256);
	return (1);
}


int
test_sublayer_nelems(slablist_t *sl, uint64_t *l, uint64_t *ll, int *k)
{
	if (!(sl->sl_sublayers)) {
		return (0);
	}
	int i = 0;
	slablist_t *u = sl;
	int r = 0;
	while (i < sl->sl_sublayers) {
		u = u->sl_sublayer;
		r = test_nelems(u, l, ll);
		*k = u->sl_layer;
		if (r) {
			return (r);
		}
		i++;
	}
	return (0);
}

int
test_sublayers_sorted(slablist_t *sl, int *l)
{
	if (sl->sl_sublayers == 0) {
		return (0);
	}

	int i = 0;
	int j = 0;
	slab_t *s1 = NULL;
	slab_t *s2 = NULL;
	slablist_t *u = sl;
	while (i < sl->sl_sublayers) {
		u = u->sl_sublayer;
		j = 0;
		while (j < (u->sl_slabs - 1)) {
			if (s1 == NULL) {
				s1 = sl->sl_head;
			} else {
				s1 = s1->s_next;
			}
			s2 = s1->s_next;
			if (sl->sl_cmp_elem(s1->s_max, s2->s_max) > 0) {
				*l = u->sl_layer;
				return (1);
			}
			j++;
		}
		i++;
	}
	*l = sl->sl_sublayers;
	return (0);
}

uintptr_t
get_slab_max(slab_t *s)
{
	int clayer = s->s_list->sl_layer;
	slab_t *sm = s;
	while (clayer > 0) {
		sm = (slab_t *)sm->s_arr[(s->s_elems - 1)];
		clayer--;
	}
	return (sm->s_max);
}

uintptr_t
get_slab_min(slab_t *s)
{
	int clayer = s->s_list->sl_layer;
	slab_t *sm = s;
	while (clayer > 0) {
		sm = (slab_t *)sm->s_arr[0];
		clayer--;
	}
	return (sm->s_min);
}

int
test_sublayer_extrema(slablist_t *sl, int *l, int *k, int *m, slab_t **sptr)
{
	if (sl->sl_sublayers == 0 || SLIST_ORDERED(sl->sl_flags)) {
		return (0);
	}

	int layer = 0;
	uint64_t slab;
	slablist_t *csl = sl;
	slab_t *s;
	uintptr_t max;
	uintptr_t min;
	int r = 0;
	int p = 0;
	while (layer < sl->sl_sublayers) {
		csl = sl->sl_sublayer;
		s = csl->sl_head;
		slab = 0;
		while (slab < csl->sl_slabs) {
			max = get_slab_max(s);
			min = get_slab_min(s);
			if (sl->sl_cmp_super(min, s->s_min) != 0) {
				r ^= 1;
			}
			if (sl->sl_cmp_super(max, s->s_max) != 0) {
				r ^= 2;
			}
			if (sl->sl_cmp_super(s->s_min, s->s_max) > 0) {
				p = 1;
			}
			if (r || p) {
				/* layer is off by 1 */
				*l = (layer + 1);
				*k = r;
				*m = p;
				*sptr = s;
				return (1);
			}
			s = s->s_next;
			slab++;
		}
		layer++;
	}

	return (0);
}

int
test_sublayer_elems_sorted(slablist_t *sl, int *l)
{
	if (sl->sl_sublayers == 0) {
		return (0);
	}

	int i = 0;
	int j = 0;
	int k = 0;
	slab_t *s = NULL;
	slablist_t *u = sl;
	while (i < sl->sl_sublayers) {
		u = u->sl_sublayer;
		s = NULL;
		j = 0;
		while (j < (u->sl_slabs)) {
			k = 0;
			if (s == NULL) {
				s = u->sl_head;
			} else {
				s = s->s_next;
			}

			while (k < (s->s_elems - 1)) {
				slab_t *s1 = (slab_t *)(s->s_arr[k]);
				slab_t *s2 = (slab_t *)(s->s_arr[(k + 1)]);
				uintptr_t e1 = s1->s_min;
				uintptr_t e2 = s2->s_min;
				if (sl->sl_cmp_super(e1, e2) == 1) {
					*l = u->sl_layer;
					return (1);
				}
				k++;
			}
			j++;
		}
		i++;
	}
	*l = sl->sl_sublayers;
	return (0);
}


int
test_sublayers_have_all_slabs(slablist_t *sl, int *l)
{
	if (sl->sl_sublayers == 0) {
		return (0);
	}

	int i = 0;
	int j = 0;
	int k = 0;
	slablist_t *c = sl;
	slablist_t *u;
	slab_t *s = NULL;
	slab_t *s2 = NULL;
	while (i < sl->sl_sublayers) {
		u = c->sl_sublayer;
		s = NULL;
		s2 = NULL;
		while (j < u->sl_slabs) {
			if (s == NULL) {
				s = u->sl_head;
			} else {
				s = s->s_next;
			}
			while (k < s->s_elems) {
				slab_t *s1 = (slab_t *)s->s_arr[k];

				if (s2 == NULL) {
					s2 = c->sl_head;
				} else {
					s2 = s2->s_next;
				}

				if (s1 != s2) {
					*l = u->sl_layer;
					return (1);
				}
				k++;

			}
			k = 0;
			j++;
		}
		j = 0;
		c = u;
		i++;
	}
	*l = sl->sl_sublayers;
	return (0);
}


void
test_sublayers(slablist_t *sl, uintptr_t elem)
{
	if (!SLABLIST_TEST_ENABLED()) {
		return;
	}

	if (SLABLIST_TEST_SUBLAYER_EXTREMA_ENABLED()) {
		int l;
		int k;
		int m;
		slab_t *s;
		int f = test_sublayer_extrema(sl, &l, &k, &m, &s);
		SLABLIST_TEST_SUBLAYER_EXTREMA(f, l, k, m, s);
	}
	if (SLABLIST_TEST_SUBLAYERS_SORTED_ENABLED()) {
		int l;
		int f = test_sublayers_sorted(sl, &l);
		SLABLIST_TEST_SUBLAYERS_SORTED(f, l);
	}

	if (SLABLIST_TEST_SUBLAYER_ELEMS_SORTED_ENABLED()) {
		int l;
		int f = test_sublayer_elems_sorted(sl, &l);
		SLABLIST_TEST_SUBLAYER_ELEMS_SORTED(f, l);
	}

	if (SLABLIST_TEST_SUBLAYER_NELEMS_ENABLED()) {
		uint64_t l;
		uint64_t ll;
		int k;
		int f = test_sublayer_nelems(sl, &l, &ll, &k);
		SLABLIST_TEST_SUBLAYER_NELEMS(f, l, ll, k);
	}

	if (SLABLIST_TEST_BUBBLE_UP_ENABLED()) {
		int l;
		int f = test_bubble_up(sl, elem, &l);
		SLABLIST_TEST_BUBBLE_UP(f, l);
	}

	if (SLABLIST_TEST_SUBLAYERS_HAVE_ALL_SLABS_ENABLED()) {
		int l;
		int f = test_sublayers_have_all_slabs(sl, &l);
		SLABLIST_TEST_SUBLAYERS_HAVE_ALL_SLABS(f, l);
	}
}

int
test_breadcrumbs(slab_t **bc, int *l, uint64_t bcn)
{
	int i = 0;
	int cl;
	int nl;
	while (i < (bcn - 1)) {
		slab_t *s = bc[i];
		slab_t *s1 = bc[(i + 1)];
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
			return (1);
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
			return (1);
		}
		j--;
		k--;
	}

	return (0);
}
