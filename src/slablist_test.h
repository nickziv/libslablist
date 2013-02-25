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

int test_slab_to_sml(slablist_t *, slab_t *);
int test_smlist_nelems(slablist_t *);
int test_smlist_elems_sorted(slablist_t *);
int test_breadcrumbs(bc_t *, int *, uint64_t);
int test_insert_elem(uintptr_t , slab_t *, int );
int test_remove_elem(int i, slab_t *);
int test_slab_extrema(slab_t *);
int test_ripple_add(slab_t *, bc_t *, int);
int test_find_bubble_up(int, bc_t *, int, uintptr_t);
int test_slab_srch(uintptr_t, slab_t *, int);
int test_move_next(slab_t *, slab_t *, slab_t *, slab_t *, int *);
int test_move_prev(slab_t *, slab_t *, slab_t *, slab_t *, int *);
