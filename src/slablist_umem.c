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

#ifdef UMEM
#include <umem.h>
#else
#include <stdlib.h>
#endif
#include <strings.h>
#include "slablist_impl.h"

#define	UNUSED(x) (void)(x)
#define	CTOR_HEAD	UNUSED(ignored); UNUSED(flags)


umem_cache_t *cache_slablist;
umem_cache_t *cache_bm;
umem_cache_t *cache_lk_slablist;
umem_cache_t *cache_mt_slablist;
umem_cache_t *cache_slab;
umem_cache_t *cache_subslab;
umem_cache_t *cache_subarr;
umem_cache_t *cache_small_list;
umem_cache_t *cache_add_ctx;

#ifdef UMEM
int
slablist_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	slablist_t *s = buf;
	bzero(s, sizeof (slablist_t));
	return (0);
}

int
bm_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	slablist_bm_t *b = buf;
	bzero(b, sizeof (slablist_bm_t));
	return (0);
}

int
lk_slablist_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	lk_slablist_t *s = buf;
	bzero(s, sizeof (lk_slablist_t));
	return (0);
}

int
mt_slablist_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	mt_slablist_t *s = buf;
	bzero(s, sizeof (mt_slablist_t));
	return (0);
}

int
slab_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	slab_t *s = buf;
	bzero(s, (sizeof (slab_t)));
	return (0);
}

int
subslab_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	subslab_t *ss = buf;
	bzero(ss, (sizeof (subslab_t)));
	return (0);
}

int
subarr_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	subarr_t *sa = buf;
	bzero(sa, (sizeof (subarr_t)));
	return (0);
}


int
small_list_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	small_list_t *s = buf;
	bzero(s, sizeof (small_list_t));
	return (0);
}

int
add_ctx_ctor(void *buf, void *ignored, int flags)
{
	CTOR_HEAD;
	add_ctx_t *ctx = buf;
	bzero(ctx, (sizeof (add_ctx_t)));
	return (0);
}
#endif

int
slablist_umem_init()
{
#ifdef UMEM
	cache_slablist = umem_cache_create("slablist",
		sizeof (slablist_t),
		0,
		slablist_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

	cache_bm = umem_cache_create("bm",
		sizeof (slablist_bm_t),
		0,
		bm_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

	cache_lk_slablist = umem_cache_create("lk_slablist",
		sizeof (lk_slablist_t),
		0,
		lk_slablist_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

	cache_mt_slablist = umem_cache_create("mt_slablist",
		sizeof (mt_slablist_t),
		0,
		mt_slablist_ctor,
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

	cache_subslab = umem_cache_create("subslab",
		sizeof (subslab_t),
		0,
		subslab_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

	cache_subarr = umem_cache_create("subarr",
		sizeof (subarr_t),
		0,
		subarr_ctor,
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

	cache_add_ctx = umem_cache_create("add_ctx",
		sizeof (add_ctx_t),
		0,
		add_ctx_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);

#endif
	return (0);
}

slablist_t *
mk_slablist()
{
#ifdef UMEM
	return (umem_cache_alloc(cache_slablist, UMEM_NOFAIL));
#else
	return (calloc(1, sizeof (slablist_t)));
#endif

}

void
rm_slablist(slablist_t *sl)
{
#ifdef UMEM
	bzero(sl, sizeof (slablist_t));
	umem_cache_free(cache_slablist, sl);
#else
	bzero(sl, sizeof (slablist_t));
	free(sl);
#endif
}

slablist_bm_t *
mk_bm()
{
#ifdef UMEM
	return (umem_cache_alloc(cache_bm, UMEM_NOFAIL));
#else
	return (calloc(1, sizeof (slablist_bm_t)));
#endif
}

void
rm_bm(slablist_bm_t *b)
{
#ifdef UMEM
	bzero(b, sizeof (slablist_bm_t));
	umem_cache_free(cache_bm, b);
#else
	bzero(b, sizeof (slablist_bm_t));
	free(b);
#endif
}

lk_slablist_t *
mk_lk_slablist()
{
#ifdef UMEM
	return (umem_cache_alloc(cache_lk_slablist, UMEM_NOFAIL));
#else
	return (calloc(1, sizeof (lk_slablist_t)));
#endif

}

void
rm_lk_slablist(lk_slablist_t *sl)
{
#ifdef UMEM
	bzero(sl, sizeof (lk_slablist_t));
	umem_cache_free(cache_lk_slablist, sl);
#else
	bzero(sl, sizeof (lk_slablist_t));
	free(sl);
#endif
}

mt_slablist_t *
mk_mt_slablist()
{
#ifdef UMEM
	return (umem_cache_alloc(cache_mt_slablist, UMEM_NOFAIL));
#else
	return (calloc(1, sizeof (mt_slablist_t)));
#endif

}

void
rm_mt_slablist(mt_slablist_t *sl)
{
#ifdef UMEM
	bzero(sl, sizeof (mt_slablist_t));
	umem_cache_free(cache_mt_slablist, sl);
#else
	bzero(sl, sizeof (mt_slablist_t));
	free(sl);
#endif
}

slab_t *
mk_slab()
{
#ifdef UMEM
	slab_t *s = umem_cache_alloc(cache_slab, UMEM_NOFAIL);
#else
	slab_t *s = calloc(1, sizeof (slab_t));
#endif
	return (s);
}

void
rm_slab(slab_t *s)
{
	bzero(s, sizeof (slab_t));
#ifdef UMEM
	umem_cache_free(cache_slab, s);
#else
	free(s);
#endif
}


subslab_t *
mk_subslab()
{
#ifdef UMEM
	subslab_t *ss = umem_cache_alloc(cache_subslab, UMEM_NOFAIL);
#else
	subslab_t *ss = calloc(1, sizeof (subslab_t));
#endif
	return (ss);
}

void
rm_subslab(subslab_t *s)
{
	bzero(s, sizeof (subslab_t));
#ifdef UMEM
	umem_cache_free(cache_subslab, s);
#else
	free(s);
#endif
}

add_ctx_t *
mk_add_ctx()
{
#ifdef UMEM
	add_ctx_t *ctx = umem_cache_alloc(cache_add_ctx, UMEM_NOFAIL);
#else
	add_ctx_t *ctx = calloc(1, sizeof (add_ctx_t));
#endif
	return (ctx);
}

void
rm_add_ctx(add_ctx_t *ctx)
{
	bzero(ctx, sizeof (add_ctx_t));
#ifdef UMEM
	umem_cache_free(cache_add_ctx, ctx);
#else
	free(ctx);
#endif
}

subarr_t *
mk_subarr()
{
#ifdef UMEM
	subarr_t *sa = umem_cache_alloc(cache_subarr, UMEM_NOFAIL);
#else
	subarr_t *sa = calloc(1, sizeof (subarr_t));
#endif
	return (sa);
}

void
rm_subarr(subarr_t *s)
{
	bzero(s, sizeof (subarr_t));
#ifdef UMEM
	umem_cache_free(cache_subarr, s);
#else
	free(s);
#endif
}

small_list_t *
mk_sml_node()
{
#ifdef UMEM
	small_list_t *s = umem_cache_alloc(cache_small_list, UMEM_NOFAIL);
#else
	small_list_t *s = calloc(1, sizeof (small_list_t));
#endif
	return (s);
}

void
rm_sml_node(small_list_t *s)
{
	bzero(s, sizeof (small_list_t));
#ifdef UMEM
	umem_cache_free(cache_small_list, s);
#else
	free(s);
#endif
}

void *
mk_buf(size_t sz)
{
#ifdef UMEM
	return (umem_alloc(sz, UMEM_NOFAIL));
#else
	return (malloc(sz));
#endif
}

void *
mk_zbuf(size_t sz)
{
#ifdef UMEM
	return (umem_zalloc(sz, UMEM_NOFAIL));
#else
	return (calloc(1, sz));
#endif
}

void
rm_buf(void *s, size_t sz)
{
#ifdef UMEM
	umem_free(s, sz);
#else
	free(s);
#endif
}
