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

extern slablist_elem_t slablist_get(slablist_t *, uint64_t);
extern slab_t *slab_get_elem_pos(slablist_t *, uint64_t, uint64_t *);
extern slab_t *slab_get_elem_pos_old(slablist_t *, uint64_t, uint64_t *);
extern slab_t *slab_get_pos(slablist_t *, uint64_t);
extern int sublayer_slab_ptr_srch(void *, subslab_t *);
extern int slab_bin_srch(slablist_elem_t, slab_t *);
extern int subslab_bin_srch(slablist_elem_t, subslab_t *);
extern int subslab_bin_srch_top(slablist_elem_t, subslab_t *);
extern int slab_lin_srch(slablist_elem_t, slab_t *);
extern int subslab_lin_srch(slablist_elem_t, subslab_t *);
extern int subslab_lin_srch_top(slablist_elem_t, subslab_t *);
extern int find_bubble_up(slablist_t *, slablist_elem_t, slab_t **);
extern int find_linear_scan(slablist_t *, slablist_elem_t, slab_t **);
