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

#include <unistd.h>
#define	SLAB_VAL_POS 1
#define	SLAB_IN_POS 2
#define	SL_SORTED 0x80
#define	SL_ORDERED 0x00
#define	SL_CIRCULAR 0x10

#define	SL_SUCCESS	0
#define	SL_ENFOUND	-1
#define	SL_ARGSORT	-2
#define	SL_ARGORD	-3
#define	SL_ARGNULL	-4
#define	SL_EMPTY	-5
#define	SL_ENCIRC	-6
#define	SL_EDUP		-7
#define	SL_ESML		-8



typedef struct slablist_bm slablist_bm_t;

typedef union slablist_elem {
	uint64_t	sle_u;
	double		sle_d;
	void		*sle_p;
	int64_t		sle_i;
	char		sle_c[8];
} slablist_elem_t;

struct slablist;
typedef struct slablist slablist_t;
typedef struct mt_slablist mt_slablist_t;

typedef int slablist_cmp_t(slablist_elem_t, slablist_elem_t);
typedef int slablist_bnd_t(slablist_elem_t, slablist_elem_t, slablist_elem_t);
typedef slablist_elem_t slablist_fold_t(slablist_elem_t, slablist_elem_t *, uint64_t);
typedef void slablist_map_t(slablist_elem_t *, uint64_t);
typedef void slablist_rem_cb_t(slablist_elem_t);


extern void slablist_map(slablist_t *, slablist_map_t);
extern int slablist_next(slablist_t *, slablist_bm_t *, slablist_elem_t *);

extern int slablist_prev(slablist_t *, slablist_bm_t *, slablist_elem_t *);

extern slablist_elem_t slablist_foldl(slablist_t *, slablist_fold_t,
slablist_elem_t zero);
extern slablist_elem_t slablist_foldr(slablist_t *, slablist_fold_t,
slablist_elem_t zero);

extern slablist_elem_t slablist_foldl_range(slablist_t *, slablist_fold_t,
slablist_elem_t, slablist_elem_t, slablist_elem_t zero);
extern slablist_elem_t slablist_foldr_range(slablist_t *, slablist_fold_t,
slablist_elem_t, slablist_elem_t, slablist_elem_t zero);

/*
 * TODO implement the mt functions.
 */

slablist_t *slablist_create(char *, slablist_cmp_t, slablist_bnd_t, uint8_t);
//mt_slablist_t *slablist_mt_create(char *, slablist_cmp_t, slablist_bnd_t, uint8_t);

extern void slablist_destroy(slablist_t *);
//extern void slablist_mt_destroy(mt_slablist_t *);

extern void slablist_set_reap_pslabs(slablist_t *, uint8_t);
//extern void slablist_mt_set_reap_pslabs(mt_slablist_t *, uint8_t);

extern void slablist_set_reap_slabs(slablist_t *, uint64_t);
//extern void slablist_mt_set_reap_slabs(mt_slablist_t *, uint64_t);

extern void slablist_set_attach_req(slablist_t *, uint64_t);
//extern void slablist_mt_set_attach_req(slablist_t *, uint64_t);

extern uint64_t slablist_get_attach_req(slablist_t *);
//extern uint64_t slablist_mt_get_attach_req(slablist_t *);

extern uint8_t slablist_get_reap_pslabs(slablist_t *);
//extern uint8_t slablist_mt_get_reap_pslabs(slablist_t *);

extern uint64_t slablist_get_reap_slabs(slablist_t *);
//extern uint64_t slablist_mt_get_reap_slabs(slablist_t *);

extern uint64_t slablist_get_elems(slablist_t *);
//extern uint64_t slablist_mt_get_elems(mt_slablist_t *);

extern uint64_t slablist_get_type(slablist_t *);
//extern uint64_t slablist_mt_get_type(mt_slablist_t *);

extern char *slablist_get_name(slablist_t *);
//extern char *slablist_mt_get_name(mt_slablist_t *);

extern int slablist_add(slablist_t *, slablist_elem_t, int);
//extern int slablist_mt_add(mt_slablist_t *, slablist_elem_t, int);

extern int slablist_sort(slablist_t *, slablist_cmp_t, slablist_bnd_t);
//extern int slablist_mt_sort(mt_slablist_t *, slablist_cmp_t, slablist_bnd_t);

extern int slablist_rem(slablist_t *, slablist_elem_t, uint64_t, slablist_rem_cb_t);
//extern int slablist_mt_rem(mt_slablist_t *, slablist_elem_t, uint64_t, slablist_rem_cb_t);

extern int slablist_rem_range(slablist_t *, slablist_elem_t, slablist_elem_t, slablist_rem_cb_t);
//extern int slablist_mt_rem_range(mt_slablist_t *, slablist_elem_t, slablist_elem_t, slablist_rem_cb_t);

extern void slablist_reap(slablist_t *);
//extern void slablist_mt_reap(mt_slablist_t *);

extern slablist_elem_t slablist_get(slablist_t *, uint64_t);
//extern slablist_elem_t slablist_mt_get(mt_slablist_t *, uint64_t);

//TODO
//extern slablist_elem_t slablist_get_head(slablist_t *);
//extern slablist_elem_t slablist_get_end(slablist_t *);
extern int slablist_find(slablist_t *, slablist_elem_t, slablist_elem_t *);
//extern int slablist_mt_find(mt_slablist_t *, slablist_elem_t, slablist_elem_t *);

extern int slablist_subseq(slablist_t *, slablist_t *, slablist_elem_t *, uint64_t);
//extern int slablist_mt_subseq(mt_slablist_t *, slablist_t *, slablist_elem_t *, uint64_t);

extern int slablist_rem_eq(slablist_t *, slablist_elem_t);
//extern int slablist_mt_rem_eq(mt_slablist_t *, slablist_elem_t);

extern void slablist_reverse(slablist_t *);
//extern void slablist_mt_reverse(mt_slablist_t *);
