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



struct slablist;
typedef struct slablist slablist_t;

typedef int slablist_cmp_t(uintptr_t, uintptr_t);


slablist_t *slablist_create(char *, size_t, slablist_cmp_t, uint16_t, uint8_t,
		uint8_t, uint8_t);
extern void slablist_destroy(slablist_t *);
extern void slablist_setmcap(slablist_t *, uint8_t);
extern uint8_t slablist_getmcap(slablist_t *);
extern uint64_t slablist_getelems(slablist_t *);
extern uint64_t slablist_gettype(slablist_t *);
extern char *slablist_getname(slablist_t *);
extern int slablist_add(slablist_t *, uintptr_t, int, uintptr_t *);
extern int slablist_rem(slablist_t *, uintptr_t, uint64_t, uintptr_t *);
extern void slablist_reap(slablist_t *);
extern uintptr_t slablist_get(slablist_t *, uint64_t);
extern uintptr_t slablist_get_rand(slablist_t *);
extern uint64_t slablist_get_rand_pos(slablist_t *);
extern int slablist_find(slablist_t *, uintptr_t, uintptr_t *);
extern int slablist_rem_eq(slablist_t *, uintptr_t);
