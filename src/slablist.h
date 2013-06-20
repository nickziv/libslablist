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



typedef union slablist_elem {
	uint64_t	sle_u;
	double		sle_d;
	void		*sle_p;
	int64_t		sle_i;
	char		sle_c[8];
} slablist_elem_t;

struct slablist;
typedef struct slablist slablist_t;

typedef int slablist_cmp_t(slablist_elem_t, slablist_elem_t);
typedef int slablist_bnd_t(slablist_elem_t, slablist_elem_t, slablist_elem_t);
typedef slablist_elem_t slablist_fold_t(slablist_elem_t, slablist_elem_t *, uint64_t);
typedef void slablist_map_t(slablist_elem_t *, uint64_t);


extern void slablist_map(slablist_t *, slablist_map_t);
extern slablist_elem_t slablist_foldl(slablist_t *, slablist_fold_t, slablist_elem_t zero);
extern slablist_elem_t slablist_foldr(slablist_t *, slablist_fold_t, slablist_elem_t zero);

slablist_t *slablist_create(char *, size_t, slablist_cmp_t, slablist_bnd_t, uint8_t);
extern void slablist_destroy(slablist_t *);
extern void slablist_set_reap_pslabs(slablist_t *, uint8_t);
extern void slablist_set_reap_slabs(slablist_t *, uint64_t);
extern void slablist_set_attach_req(slablist_t *, uint64_t);
extern uint64_t slablist_get_attach_req(slablist_t *);
extern uint8_t slablist_get_reap_pslabs(slablist_t *);
extern uint64_t slablist_get_reap_slabs(slablist_t *);
extern uint64_t slablist_get_elems(slablist_t *);
extern uint64_t slablist_get_type(slablist_t *);
extern char *slablist_get_name(slablist_t *);
extern int slablist_add(slablist_t *, slablist_elem_t, int);
extern int slablist_rem(slablist_t *, slablist_elem_t, uint64_t, slablist_elem_t *);
extern void slablist_reap(slablist_t *);
extern slablist_elem_t slablist_get(slablist_t *, uint64_t);
extern slablist_elem_t slablist_get_rand(slablist_t *);
extern uint64_t slablist_get_rand_pos(slablist_t *);
extern int slablist_find(slablist_t *, slablist_elem_t, slablist_elem_t *);
extern int slablist_rem_eq(slablist_t *, slablist_elem_t);
