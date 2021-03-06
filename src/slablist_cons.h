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

extern void unlink_sml_node(slablist_t *, small_list_t *);
extern void link_sml_node(slablist_t *, small_list_t *, small_list_t *);
extern void link_slab(slab_t *, slab_t *, int);
extern void link_subslab(subslab_t *, subslab_t *, int);
extern void unlink_slab(slab_t *);
extern void unlink_subslab(subslab_t *);
extern void small_list_to_slab(slablist_t *);
extern void slab_to_small_list(slablist_t *);
extern void attach_sublayer(slablist_t *);
extern void detach_sublayer(slablist_t *);
extern void try_reap(slablist_t *);
extern void try_reap_all(slablist_t *);
extern selem_t slablist_foldl_range_impl(slablist_t *, slablist_fold_t,
selem_t, selem_t, selem_t);
extern selem_t slablist_foldr_range_impl(slablist_t *, slablist_fold_t,
selem_t, selem_t, selem_t);
