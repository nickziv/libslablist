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

#include <umem.h>
#include <strings.h>
#include "slablist_impl.h"

umem_cache_t *cache_slablist;
umem_cache_t *cache_slab;
umem_cache_t *cache_small_list;

int
slablist_ctor(void *buf, void *ignored, int flags)
{
	slablist_t *s = buf;
	bzero(s, sizeof (slablist_t));
	s->sl_is_small_list = 1;
	return (0);
}

int
slab_ctor(void *buf, void *ignored, int flags)
{
	slab_t *s = buf;
	bzero(s, (sizeof (slab_t)));
	return (0);
}


int
small_list_ctor(void *buf, void *ignored, int flags)
{
	small_list_t *s = buf;
	bzero(s, sizeof (small_list_t));
	return (0);
}


int
bc_ctor(void *buf, void *ignored, int flags)
{
	bzero(buf, sizeof (bc_t));
	return (0);
}

int
bc_path_ctor(void *buf, void *ignored, int flags)
{
	bzero(buf, MAX_LYRS * sizeof (bc_t));
	return (0);
}

int
slablist_umem_init()
{
	cache_slablist = umem_cache_create("slablist",
		sizeof (slablist_t),
		0,
		slablist_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

	cache_slab = umem_cache_create("slab",
		sizeof (slab_t),
		0,
		slab_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

	cache_small_list = umem_cache_create("small_list",
		sizeof (small_list_t),
		0,
		small_list_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

	return (0);
}

slablist_t *
mk_slablist()
{
	return (umem_cache_alloc(cache_slablist, UMEM_NOFAIL));
}

void
rm_slablist(slablist_t *sl)
{
	bzero(sl, sizeof (slablist_t));
	sl->sl_is_small_list = 1;
	umem_cache_free(cache_slablist, sl);
}


slab_t *
mk_slab()
{
	slab_t *s = umem_cache_alloc(cache_slab, UMEM_NOFAIL);
	return (s);
}

void
rm_slab(slab_t *s)
{
	bzero(s, sizeof (slab_t));
	umem_cache_free(cache_slab, s);
}

small_list_t *
mk_sml_node()
{
	small_list_t *s = umem_cache_alloc(cache_small_list, UMEM_NOFAIL);
	return (s);
}

void
rm_sml_node(small_list_t *s)
{
	bzero(s, sizeof (small_list_t));
	umem_cache_free(cache_small_list, s);
}

void *
mk_buf(size_t sz)
{
	return (umem_alloc(sz, UMEM_NOFAIL));
}

void *
mk_zbuf(size_t sz)
{
	return (umem_zalloc(sz, UMEM_NOFAIL));
}

void
rm_buf(void *s, size_t sz)
{
	umem_free(s, sz);
}
