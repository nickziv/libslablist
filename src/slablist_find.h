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

extern uintptr_t slablist_get(slablist_t *, uint64_t);
extern slab_t *slab_get(slablist_t *, uint64_t, uint64_t *, int);
extern int sublayer_slab_bin_srch(uintptr_t, slab_t *);
extern int sublayer_slab_ptr_srch(uintptr_t, slab_t *, int);
extern int slab_bin_srch(uintptr_t, slab_t *);
extern int gen_bin_srch(uintptr_t, slab_t *, int);
extern int slab_srch(uintptr_t, slab_t *, int);
extern int find_bubble_up(slablist_t *, uintptr_t, slab_t **);
extern int find_linear_scan(slablist_t *, uintptr_t, slab_t **);
extern int is_elem_in_range(uintptr_t, slab_t *);
