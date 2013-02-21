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

int test_slab_to_sml(slablist_t *sl, slab_t *s);

int test_smlist_nelems(slablist_t *sl);

int test_smlist_elems_sorted(slablist_t *sl);

int test_slab_elems_sorted(slablist_t *sl);

int test_slab_bkptr(slablist_t *sl);

int test_nelems(slablist_t *sl, uint64_t *l, uint64_t *ll);

int test_nslabs(slablist_t *sl, uint64_t *l, uint64_t *ll);

int test_bubble_up(slablist_t *sl, uintptr_t elem, int *l);

int test_slab_elems_max(slablist_t *sl, uint64_t *l);

int test_sublayer_nelems(slablist_t *sl, uint64_t *l, uint64_t *ll, int *k);

int test_slab_extrema(slablist_t *sl, int *l);

int test_sublayer_extrema(slablist_t *, int *, int *);

int test_slabs_sorted(slablist_t *sl, slab_t **, slab_t **);

int test_sublayers_sorted(slablist_t *sl, int *l);

int test_sublayer_elems_sorted(slablist_t *sl, int *l);

int test_sublayers_have_all_slabs(slablist_t *sl, int *l);

int test_breadcrumbs(bc_t *bc, int *l, uint64_t bcn);

int test_move_next(slab_t *s, slab_t *sn, slab_t *scp, slab_t *sncp, int *i);

int test_move_prev(slab_t *s, slab_t *sn, slab_t *scp, slab_t *sncp, int *i);

extern void test_slab_consistency(slablist_t *sl);
extern void test_slab_sorting(slablist_t *sl);
extern void test_sublayers(slablist_t *sl, uintptr_t elem);

