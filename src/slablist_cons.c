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
#include <stdlib.h>
#include <thread.h>
#include <synch.h>
#include <strings.h>
#include <stdio.h>
#include "slablist_impl.h"
#include "slablist_find.h"
#include "slablist_test.h"
#include "slablist_provider.h"

static int init = 0;
static int nlists = 0;
static slablist_t *lst_sl = NULL;
/* static pthread_mutex_t lst_sl_lk; */
extern int slablist_umem_init();

slablist_t *
slablist_create(
	char *name,
	size_t obj_size,
	slablist_cmp_t cmpfun,
	uint16_t sublayer_req,
	uint8_t mcap,
	uint8_t brk,
	uint8_t fl)
{
	if (init == 0) {
		slablist_umem_init();
		init = 1;
	}
	slablist_t *list = mk_slablist();
	/* TODO: add this list to the master slablist */
	// link_slablist(master_list, list);
	size_t namesize = strlen(name);
	list->sl_name = mk_buf(namesize);
	strcpy(list->sl_name, name);
	list->sl_cmp_elem = cmpfun;
	list->sl_cmp_super = cmpfun;
	list->sl_obj_sz = obj_size;
	list->sl_flags = fl;
	if (mcap > 99) {
		list->sl_mcap = 99;
	}
	list->sl_mcap = mcap;
	list->sl_brk = brk;
	list->sl_req_sublayer = sublayer_req;
	SLABLIST_CREATE(list);
	if (lst_sl == NULL) {
		lst_sl = list;
	} else {
		list->sl_next = lst_sl;
		lst_sl->sl_prev = list;
		lst_sl = list;
	}
	nlists++;
	return (list);
}

void
slablist_setmcap(slablist_t *sl, uint8_t new)
{
	if (new <= 99) {
		sl->sl_mcap = new;
	} else {
		sl->sl_mcap = 99;
	}
}

char *
slablist_getname(slablist_t *sl)
{
	return (sl->sl_name);
}

uint8_t
slablist_getmcap(slablist_t *sl)
{
	return (sl->sl_mcap);
}

uint64_t
slablist_getelems(slablist_t *sl)
{
	return (sl->sl_elems);
}

uint64_t
slablist_gettype(slablist_t *sl)
{
	if (SLIST_SORTED(sl->sl_flags)) {
		return (SL_SORTED);
	}
	return (SL_ORDERED);
}

void
link_sml_node(slablist_t *sl, small_list_t *prev, small_list_t *to_link)
{
	SLABLIST_LINK_SML_NODE(sl);
	small_list_t *next = NULL;

	if (prev != NULL) {
		next = prev->sml_next;
		prev->sml_next = to_link;
		to_link->sml_next = next;
	} else {
		next = sl->sl_head;
		sl->sl_head = to_link;
		SLABLIST_ADD_HEAD(sl);
		to_link->sml_next = next;
	}

	sl->sl_elems++;
	SLABLIST_SL_INC_ELEMS(sl);
}

/*
 * `to_rem` is _always_ prev->sml_next...
 */
void
unlink_sml_node(slablist_t *sl, small_list_t *prev)
{
	SLABLIST_UNLINK_SML_NODE(sl);
	small_list_t *to_rem = NULL;

	if (prev == NULL) {
		to_rem = sl->sl_head;
		sl->sl_head = to_rem->sml_next;
		SLABLIST_REM_HEAD(sl);
	} else {
		to_rem = prev->sml_next;
		prev->sml_next = to_rem->sml_next;
	}

	sl->sl_elems--;
	SLABLIST_SL_DEC_ELEMS(sl);
}

void
link_slab(slab_t *s1, slab_t *s2, int flag)
{
	slablist_t *sl = s2->s_list;
	if (flag == SLAB_LINK_BEFORE) {
		SLABLIST_LINK_SLAB_BEFORE(sl, s1, s2);
		s1->s_next = s2;
		s1->s_prev = s2->s_prev;
		s2->s_prev = s1;
		if (s1->s_prev != NULL) {
			s1->s_prev->s_next = s1;
		}
		if (s2 == sl->sl_head) {
			sl->sl_head = s1;
			SLABLIST_ADD_HEAD(sl);
		}
	}

	if (flag == SLAB_LINK_AFTER) {
		SLABLIST_LINK_SLAB_AFTER(sl, s1, s2);
		s1->s_prev = s2;
		s1->s_next = s2->s_next;
		s2->s_next = s1;
		if (s1->s_next != NULL) {
			s1->s_next->s_prev = s1;
		}
	}

	s2->s_list->sl_slabs++;
	SLABLIST_SL_INC_SLABS(s2->s_list);
	s1->s_list = s2->s_list;

}

void
unlink_slab(slab_t *s)
{
	slablist_t *sl = s->s_list;
	SLABLIST_UNLINK_SLAB(sl, s);
	if (s->s_prev != NULL) {
		s->s_prev->s_next = s->s_next;
	}

	if (s->s_next != NULL) {
		s->s_next->s_prev = s->s_prev;
		if (sl->sl_head == s) {
			sl->sl_head = s->s_next;
			SLABLIST_REM_HEAD(sl);
		}
	}

	sl->sl_slabs--;
	SLABLIST_SL_DEC_SLABS(sl);
}

static void
remove_slabs(slablist_t *sl)
{
	slab_t *s;
	s = sl->sl_head;
	uint64_t i = 0;
	uint64_t nslabs = sl->sl_slabs;
	while (i < nslabs) {
		/*
		 * We are not responsible for user-allocated objects.
		 */
		unlink_slab(s);
		rm_slab(s);
		SLABLIST_SLAB_RM(sl);
		s = s->s_next;
		i++;
	}
}

void
slablist_destroy(slablist_t *sl)
{
	SLABLIST_DESTROY(sl);
	size_t namesz = strlen(sl->sl_name);
	rm_buf(sl->sl_name, namesz);
	small_list_t *sml;
	small_list_t *smln;

	slablist_t *p;
	slablist_t *q;
	p = sl->sl_sublayer;
	if (!(sl->sl_is_small_list) && sl->sl_head != NULL) {
		remove_slabs(sl);
	}

	if (!(sl->sl_is_small_list) && sl->sl_sublayer != NULL) {
		while (p != NULL) {
			q = p;
			remove_slabs(p);
			p = q->sl_sublayer;
			rm_slablist(q);
		}
	}

	if (sl->sl_is_small_list && sl->sl_head != NULL) {
		sml = sl->sl_head;
		uint64_t i = 0;
		while (i < sl->sl_elems) {
			smln = sml->sml_next;
			rm_sml_node(sml);
			sml = smln;
			i++;
		}
	}

	nlists--;
	rm_slablist(sl);
}


/*
 * This function detaches the sublayer that is immediately attached `sl`, and
 * frees all data associated with it. Be careful not to detach a sublayer which
 * has sublayers! This is the caller's responsibility.
 */
void
detach_sublayer(slablist_t *sl)
{
	slablist_t *sub = sl->sl_sublayer;
	SLABLIST_DETACH_SUBLAYER(sl, sub);

	remove_slabs(sub);
	sl->sl_sublayer = NULL;

	SLABLIST_SL_DEC_ELEMS(sub);

	rm_slablist(sub);
	slablist_t *sup = sl;
	while (sup != NULL) {
		sup->sl_baselayer = sl;
		sup->sl_sublayers--;
		sup = sup->sl_superlayer;
	}
}

static int
sublayer_cmp(uintptr_t e1, uintptr_t e2)
{
	int ret = is_elem_in_range(e1, (slab_t *)e2);
	return (ret);
}

/*
 * This function attaches a new sublayer to `sl`. Be careful not to attach a
 * new sublayer to a list which already has a sublayer. This is the caller's
 * responsibility.
 */
void
attach_sublayer(slablist_t *sl)
{
	slablist_t *sub = mk_slablist();
	SLABLIST_ATTACH_SUBLAYER(sl, sub);
	sub->sl_req_sublayer = sl->sl_req_sublayer;
	bcopy(sl, sub, sizeof (slablist_t));
	sl->sl_sublayer = sub;
	sl->sl_baselayer = sub;

	sub->sl_head = mk_slab();
	sub->sl_cmp_elem = sublayer_cmp;

	/*
	 * We set the sublayer's mcap to 99, so as to minimize its memory
	 * overhead.
	 */
	sub->sl_mcap = 99;
	SLABLIST_SLAB_MK(sub);
	SLIST_SET_SUBLAYER(sub->sl_flags);

	slab_t *sh = sub->sl_head;
	sh->s_elems = sl->sl_slabs;
	sh->s_list = sub;

	slab_t *h = sl->sl_head;

	sub->sl_slabs = 1;
	sub->sl_elems = sl->sl_slabs;
	sub->sl_superlayer = sl;
	sub->sl_layer++;
	SLABLIST_SL_INC_LAYER(sub);

	slablist_t *sup = sub->sl_superlayer;

	while (sup != NULL) {
		sup->sl_sublayers++;
		sup->sl_baselayer = sub;
		SLABLIST_SL_INC_SUBLAYERS(sup);
		sup = sup->sl_superlayer;
	}

	slab_t *c = h;

	int i = 0;

	while (i < sl->sl_slabs) {
		SLABLIST_ADD_INTO(sub, sh, (uintptr_t)c);
		sh->s_arr[i] = (uintptr_t)c;
		c = c->s_next;
		i++;
	}

	slab_t *f = (slab_t *)(sh->s_arr[0]);
	slab_t *l = (slab_t *)(sh->s_arr[(i - 1)]);
	sh->s_min = (uintptr_t)(f->s_min);
	SLABLIST_SLAB_SET_MIN(sh);
	sh->s_max = (uintptr_t)(l->s_max);
	SLABLIST_SLAB_SET_MAX(sh);
}

/*
 * We use this function later, so it needs to be declared.
 */
int small_list_add(slablist_t *, uintptr_t, int, uintptr_t *);


void
slab_to_small_list(slablist_t *sl)
{
	sl->sl_is_small_list = 1;
	slab_t *h = sl->sl_head;
	sl->sl_head = NULL;
	uint64_t nelems = sl->sl_elems;
	sl->sl_elems = 0;
	uint64_t i = 0;
	while (i < nelems) {
		/*
		 * We copy the data from the head slab into sml_nodes, which we
		 * link up into a singly linked list.
		 */
		small_list_add(sl, h->s_arr[i], 0, NULL);
		i++;
	}

	sl->sl_slabs = 0;
	if (SLABLIST_TEST_ENABLED() && SLABLIST_TEST_SLAB_TO_SML_ENABLED()) {
		int f = test_slab_to_sml(sl, h);
		SLABLIST_TEST_SLAB_TO_SML(f);
	}
	rm_slab(h);
	SLABLIST_SLAB_RM(sl);
	SLABLIST_TO_SMALL_LIST(sl);
}

void
small_list_to_slab(slablist_t *sl)
{
	SLABLIST_TO_SLAB(sl);
	slab_t *s = NULL;
	s = mk_slab();
	SLABLIST_SLAB_MK(sl);
	s->s_list = sl;
	small_list_t *sml = sl->sl_head;
	small_list_t *smlp = NULL;
	uint64_t i = 0;
	while (i < sl->sl_elems) {
		s->s_arr[i] = sml->sml_data;
		smlp = sml;
		sml = sml->sml_next;
		rm_sml_node(smlp);
		s->s_elems++;
		i++;
	}

	SLABLIST_SLAB_INC_ELEMS(s);
	sl->sl_is_small_list = 0;
	sl->sl_head = s;
	sl->sl_slabs = 1;
	SLABLIST_SL_INC_SLABS(sl);

	if (SLIST_SORTED(sl->sl_flags)) {
		s->s_min = s->s_arr[0];
		s->s_max = s->s_arr[(s->s_elems - 1)];
	}
}

/*
 * This function tries to reap all the slabs in a slab list (not counting any
 * subslabs). It will only reap if the slab list is below its minimum memory
 * capacity.
 */
void
try_reap(slablist_t *sl)
{
	double es = (double)sl->sl_elems;
	double ss = (double)sl->sl_slabs;
	/* the minimum number of elems with 1 partial-slab */
	double mxe = (ss*(double)SELEM_MAX) - (double)(SELEM_MAX - 1);
	/* the maximum efficiency with 1 partial-slab */
	double mxss = mxe / (ss*(double)SELEM_MAX);
	/* the current utilization ration */
	double cratio = es / (ss * SELEM_MAX);
	/* the mcap in double foating point form */
	double dmcap = ((double)sl->sl_mcap)/100;
	if (!(sl->sl_mcap > 100) && ss > 1 &&
	    mxss >= dmcap && (cratio <= dmcap)) {
		/*
		 * If we have a valid mcap (minimum capacity) value, the mcap
		 * is not greater than the maximum possible efficiency of the
		 * slablist, and the current utilization is not at or above the
		 * mcap value, we initiate a reap.
		 */
		slablist_reap(sl);
	}
}

/*
 * This function tries to reap all the slabs in a slab list, including all the
 * subslabs.
 */
void
try_reap_all(slablist_t *sl)
{
	slablist_t *csl = sl;
	int i = 0;
	//try_reap(csl);
	//return;
	do {
		try_reap(csl);
		csl = csl->sl_sublayer;
		i++;
	} while (i < sl->sl_sublayers);
}
