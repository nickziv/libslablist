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
 * Copyright 2016 Nicholas Zivkovic. All rights reserved.
 * Use is subject to license terms.
 */

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <strings.h>
#include <stdio.h>
#include "slablist_impl.h"
#include "slablist_find.h"
#include "slablist_test.h"
#include "slablist_provider.h"

static int init = 0;
/*
 * Originally, it was intended that there would be a 'master' slablist that
 * held references to all of the slablists for accounting and statistics
 * purposes. This was never implemented, and the lines of code below are
 * vestiges of that vision. Though the dream lives on, and may be implemented
 * if a very compelling case can be made for it.
 */
/* static slablist_t *lst_sl = NULL; */
/* static pthread_mutex_t lst_sl_lk; */
extern int slablist_umem_init();

slablist_t *
slablist_create(
	char *name,		/* descriptive name */
	slablist_cmp_t cmpfun,	/* comparison function callback */
	slablist_bnd_t bndfun,	/* bounds function callback */
	uint8_t fl)		/* flags */
{
	/*
	 * If this is the first use of libslablist, we initialize the umem
	 * caches.
	 */
	if (init == 0) {
		slablist_umem_init();
		init = 1;
	}
	slablist_t *list = mk_slablist();
	/* TODO: add this list to the master slablist */
	/* link_slablist(master_list, list); */
	list->sl_name = name;
	list->sl_cmp_elem = cmpfun;
	list->sl_bnd_elem = bndfun;
	list->sl_flags = fl;

	/* reap defaults */
	list->sl_mpslabs = 30;
	list->sl_mslabs = 30;

	list->sl_req_sublayer = 10;

	SLABLIST_CREATE(list);
	return (list);
}

slablist_bm_t *
slablist_bm_create()
{
	slablist_bm_t *b = mk_bm();
	return (b);
}

void
slablist_bm_destroy(slablist_bm_t *b)
{
	rm_bm(b);
}




/*
 * This function allows the user to set the minimum percentage of reapable
 * slabs that need to be present before a reap begins.
 */
void
slablist_set_reap_pslabs(slablist_t *sl, uint8_t new)
{
	if (new <= 99) {
		sl->sl_mpslabs = new;
	} else {
		sl->sl_mpslabs = 99;
	}
}

/*
 * This function allows the user to set the minimum number of reapable slabs
 * that need to be present before a reap begins.
 */
void
slablist_set_reap_slabs(slablist_t *sl, uint64_t new)
{
	sl->sl_mslabs = new;
}

void
slablist_set_attach_req(slablist_t *sl, uint64_t req)
{
	/*
	 * Set the sublayer_req value, which can never be larger than the
	 * number of elems that can fit in a subslab NOR the maximum value of
	 * the sl_req_sublayer member (2^w where w is the number of bits that
	 * member can hold). So if we make that member 16-bit, we will expect
	 * SL_REQ_MAX to be equivalent to SUBELEM_MAX.
	 */
	if (req <= SL_REQ_MAX) {
		sl->sl_req_sublayer = req;
	} else {
		sl->sl_req_sublayer = SL_REQ_MAX;
	}
}

uint64_t
slablist_get_attach_req(slablist_t *sl)
{
	return (sl->sl_req_sublayer);
}

char *
slablist_get_name(slablist_t *sl)
{
	return (sl->sl_name);
}

uint8_t
slablist_get_reap_pslabs(slablist_t *sl)
{
	return (sl->sl_mpslabs);
}

uint64_t
slablist_get_reap_slabs(slablist_t *sl)
{
	return (sl->sl_mslabs);
}

uint64_t
slablist_get_elems(slablist_t *sl)
{
	return (sl->sl_elems);
}

uint64_t
slablist_get_type(slablist_t *sl)
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
		SLABLIST_SET_HEAD(sl, to_link->sml_data);
		to_link->sml_next = next;
	}

	sl->sl_elems++;
	SLABLIST_SL_INC_ELEMS(sl);
}

/*
 * We unlink the node that comes after `prev` from the linked list.
 */
void
unlink_sml_node(slablist_t *sl, small_list_t *prev)
{
	SLABLIST_UNLINK_SML_NODE(sl);
	small_list_t *to_rem = NULL;

	/*
	 * If prev is NULL, we want to remove the head, otherwise we just
	 * remove the next node.
	 */
	if (prev == NULL) {
		to_rem = sl->sl_head;
		sl->sl_head = to_rem->sml_next;
		if (to_rem->sml_next != NULL) {
			SLABLIST_SET_HEAD(sl, to_rem->sml_next->sml_data);
		}
	} else {
		to_rem = prev->sml_next;
		prev->sml_next = to_rem->sml_next;
		if (to_rem->sml_next == NULL) {
			sl->sl_end = prev;
			SLABLIST_SET_END(sl, prev->sml_data);
		}
	}

	sl->sl_elems--;
	SLABLIST_SL_DEC_ELEMS(sl);
}

/*
 * We link slab `s1` to slab `s2`. `flag` indicates if we link to the left or
 * to the right of `s2`.
 */
void
link_slab(slab_t *s1, slab_t *s2, int flag)
{
	slablist_t *sl = s2->s_list;
	s1->s_below = s2->s_below;
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
			SLABLIST_SET_HEAD(sl, s1->s_min);
		}
	}

	if (flag == SLAB_LINK_AFTER) {
		SLABLIST_LINK_SLAB_AFTER(sl, s1, s2);
		s1->s_prev = s2;
		s1->s_next = s2->s_next;
		s2->s_next = s1;
		if (s1->s_next != NULL) {
			s1->s_next->s_prev = s1;
		} else {
			sl->sl_end = s1;
			SLABLIST_SET_END(sl, s1->s_max);
		}
	}

	s2->s_list->sl_slabs++;
	SLABLIST_SL_INC_SLABS(s2->s_list);
	s1->s_list = s2->s_list;

}

/*
 * We link subslab `s1` to subslab `s2`. `flag` indicates if we link to the
 * left or to the right of `s2`.
 */
void
link_subslab(subslab_t *s1, subslab_t *s2, int flag)
{
	slablist_t *sl = s2->ss_list;
	s1->ss_below = s2->ss_below;
	if (flag == SLAB_LINK_BEFORE) {
		SLABLIST_LINK_SUBSLAB_BEFORE(sl, s1, s2);
		s1->ss_next = s2;
		s1->ss_prev = s2->ss_prev;
		s2->ss_prev = s1;
		if (s1->ss_prev != NULL) {
			s1->ss_prev->ss_next = s1;
		}
		if (s2 == sl->sl_head) {
			sl->sl_head = s1;
			SLABLIST_SET_HEAD(sl, s1->ss_min);
		}
	}

	if (flag == SLAB_LINK_AFTER) {
		SLABLIST_LINK_SUBSLAB_AFTER(sl, s1, s2);
		s1->ss_prev = s2;
		s1->ss_next = s2->ss_next;
		s2->ss_next = s1;
		if (s1->ss_next != NULL) {
			s1->ss_next->ss_prev = s1;
		}
		if (s2 == sl->sl_end) {
			sl->sl_end = s1;
			SLABLIST_SET_END(sl, s1->ss_max);
		}
	}

	s2->ss_list->sl_slabs++;
	SLABLIST_SL_INC_SUBSLABS(s2->ss_list);
	s1->ss_list = s2->ss_list;

}

/*
 * Removes slab from slab list.
 */
void
unlink_slab(slab_t *s)
{
	slablist_t *sl = s->s_list;
	SLABLIST_UNLINK_SLAB(sl, s);
	if (s->s_prev != NULL) {
		s->s_prev->s_next = s->s_next;
		if (sl->sl_end == s) {
			sl->sl_end = s->s_prev;
			SLABLIST_SET_END(sl, s->s_prev->s_max);
		}
	}

	if (s->s_next != NULL) {
		s->s_next->s_prev = s->s_prev;
		if (sl->sl_head == s) {
			sl->sl_head = s->s_next;
			SLABLIST_SET_HEAD(sl, s->s_next->s_min);
		}
	}

	sl->sl_slabs--;
	SLABLIST_SL_DEC_SLABS(sl);
}

/*
 * Removes subslab from subslab list.
 */
void
unlink_subslab(subslab_t *s)
{
	slablist_t *sl = s->ss_list;
	SLABLIST_UNLINK_SUBSLAB(sl, s);
	if (s->ss_prev != NULL) {
		s->ss_prev->ss_next = s->ss_next;
		if (sl->sl_end == s) {
			sl->sl_end = s->ss_prev;
			SLABLIST_SET_END(sl, s->ss_prev->ss_max);
		}
	}

	if (s->ss_next != NULL) {
		s->ss_next->ss_prev = s->ss_prev;
		if (sl->sl_head == s) {
			sl->sl_head = s->ss_next;
			SLABLIST_SET_HEAD(sl, s->ss_next->ss_min);
		}
	}

	sl->sl_slabs--;
	SLABLIST_SL_DEC_SUBSLABS(sl);
}

/*
 * Removes all slabs from `sl`. Used as a catch-all.
 */
static void
remove_slabs(slablist_t *sl, slablist_rem_cb_t cb)
{
	slab_t *s;
	slab_t *sn;
	s = sl->sl_head;
	uint64_t i = 0;
	uint64_t nslabs = sl->sl_slabs;
	while (i < nslabs) {
		sn = s->s_next;
		if (cb == NULL) {
			goto skip_cb;
		}

		int j = 0;
		while (j <= s->s_elems) {
			cb(s->s_arr[j]);
			j++;
		}
skip_cb:;
		unlink_slab(s);
		rm_slab(s);
		SLABLIST_SLAB_RM(sl);
		s = sn;
		i++;
	}
}

/*
 * Removes all subslabs from `sl`. Used as a catch-all.
 */
static void
remove_subslabs(slablist_t *sl)
{
	subslab_t *s;
	subslab_t *sn;
	s = sl->sl_head;
	uint64_t i = 0;
	uint64_t nslabs = sl->sl_slabs;
	while (i < nslabs) {
		sn = s->ss_next;
		unlink_subslab(s);
		rm_subarr(s->ss_arr);
		rm_subslab(s);
		SLABLIST_SUBSLAB_RM(sl);
		s = sn;
		i++;
	}
}

/*
 * Destroys a slab list, and frees all slabs as well as removing `sl` from
 * memory. If a callback is present, it is called on each element in the
 * slablist as we iterate from left to right. This is useful if you want to
 * free the elements before their pointers are destroyed (along with the slabs
 * they're stored in).
 */
void
slablist_destroy(slablist_t *sl, slablist_rem_cb_t cb)
{
	SLABLIST_DESTROY(sl);
	small_list_t *sml;
	small_list_t *smln;

	/*
	 * If we are dealing with a non-empty small list, we remove
	 * the individual linked list nodes.
	 */
	if (IS_SMALL_LIST(sl) && sl->sl_head != NULL) {
		sml = sl->sl_head;
		uint64_t i = 0;
		while (i < sl->sl_elems) {
			smln = sml->sml_next;
			if (cb != NULL) {
				cb(sml->sml_data);
			}
			rm_sml_node(sml);
			sml = smln;
			i++;
		}
	}

	slablist_t *p;
	slablist_t *q;
	p = sl->sl_sublayer;
	/*
	 * We remove all of the slabs in the top layer/
	 */
	if (!(IS_SMALL_LIST(sl)) && sl->sl_head != NULL) {
		remove_slabs(sl, NULL);
	}

	/*
	 * We remove all of the sublabs in the sublayers, one by one.
	 */
	if (!(IS_SMALL_LIST(sl)) && sl->sl_sublayer != NULL) {
		while (p != NULL) {
			q = p;
			remove_subslabs(p);
			p = q->sl_sublayer;
			rm_slablist(q);
		}
	}

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


	uint64_t i = 0;
	slab_t *s = sl->sl_head;
	subslab_t *ss = sl->sl_head;
	if (sl->sl_layer == 0) {
		while (i < sl->sl_slabs) {
			s->s_below = NULL;
			s = s->s_next;
			i++;
		}
	} else {
		while (i < sl->sl_slabs) {
			ss->ss_below = NULL;
			ss = ss->ss_next;
			i++;
		}
	}

	remove_subslabs(sub);

	sl->sl_sublayer = NULL;

	SLABLIST_SL_DEC_ELEMS(sub);

	rm_slablist(sub);
	slablist_t *sup = sl;
	/* Update the sublayer counter in all the superlayers. */
	while (sup != NULL) {
		sup->sl_baselayer = sl;
		sup->sl_sublayers--;
		sup = sup->sl_superlayer;
	}
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

	sub->sl_head = mk_subslab();

	SLABLIST_SLAB_MK(sub);

	subslab_t *sh = sub->sl_head;
	sh->ss_elems = sl->sl_slabs;
	sh->ss_list = sub;
	sh->ss_arr = mk_subarr();

	slab_t *h = sl->sl_head;
	subslab_t *hh = sl->sl_head;

	sub->sl_slabs = 1;
	sub->sl_elems = sl->sl_slabs;
	sub->sl_superlayer = sl;
	sub->sl_layer++;
	SLABLIST_SL_INC_LAYER(sub);

	slablist_t *sup = sub->sl_superlayer;

	/* Update the sublayer counter in all the superlayers */
	while (sup != NULL) {
		sup->sl_sublayers++;
		sup->sl_baselayer = sub;
		SLABLIST_SL_INC_SUBLAYERS(sup);
		sup = sup->sl_superlayer;
	}

	subslab_t *sc = hh;
	slab_t *c = h;
	subslab_t *sfirst = NULL;
	subslab_t *slast = NULL;
	slab_t *f = NULL;
	slab_t *l = NULL;
	uint64_t i = 0;
	if (sl->sl_layer) {
		/* Copy pointers of all superslabs into the head subslab */
		while (i < sl->sl_slabs) {
			SLABLIST_SUBSLAB_AI(sub, sh, NULL, sc);
			SET_SUBSLAB_ELEM(sh, (void *)sc, i);
			sc->ss_below = sh;
			sh->ss_usr_elems += sc->ss_usr_elems;
			sc = sc->ss_next;
			i++;
		}
		/* Set the max and the min of head subslab */
		sfirst = (subslab_t *)(GET_SUBSLAB_ELEM(sh, 0));
		int last = i - 1;
		slast = (subslab_t *)(GET_SUBSLAB_ELEM(sh, last));
		sh->ss_min = (sfirst->ss_min);
		SLABLIST_SUBSLAB_SET_MIN(sh);
		sh->ss_max = (slast->ss_max);
		SLABLIST_SUBSLAB_SET_MAX(sh);
	} else {
		/* Copy pointers of all superslabs into the head subslab */
		while (i < sl->sl_slabs) {
			SLABLIST_SUBSLAB_AI(sub, sh, c, NULL);
			SET_SUBSLAB_ELEM(sh, (void *)c, i);
			c->s_below = sh;
			sh->ss_usr_elems += c->s_elems;
			c = c->s_next;
			i++;
		}
		/* Set the max and the min of head subslab */
		f = (slab_t *)(GET_SUBSLAB_ELEM(sh, 0));
		int last = i - 1;
		l = (slab_t *)(GET_SUBSLAB_ELEM(sh, last));
		sh->ss_min = (f->s_min);
		SLABLIST_SUBSLAB_SET_MIN(sh);
		sh->ss_max = (l->s_max);
		SLABLIST_SUBSLAB_SET_MAX(sh);
	}
}

/*
 * We use this function later, so it needs to be declared.
 */
int small_list_add(slablist_t *, slablist_elem_t, int, slablist_elem_t *);


/*
 * When we decrease to less than 50% capacity in a slab list with one remaining
 * slab, we convert the slab into a singly linked list. This way, if we keep
 * removing elements from this slab list, we will never dip below 50% memory
 * efficiency.
 */
void
slab_to_small_list(slablist_t *sl)
{
	slab_t *h = sl->sl_head;
	sl->sl_head = NULL;
	sl->sl_elems = 0;
	uint64_t i = 0;
	/*
	 * We copy the data from the head slab into sml_nodes, which we link up
	 * into a singly linked list.
	 */
	while (i < h->s_elems) {
		small_list_add(sl, h->s_arr[i], 0, NULL);
		i++;
	}

	sl->sl_slabs = 0;
	if (SLABLIST_TEST_SLAB_TO_SML_ENABLED()) {
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
	sl->sl_head = s;
	sl->sl_end = s;
	sl->sl_slabs = 1;
	SLABLIST_SL_INC_SLABS(sl);

	s->s_min = s->s_arr[0];
	s->s_max = s->s_arr[(s->s_elems - 1)];
	SLABLIST_SET_HEAD(sl, s->s_min);
	SLABLIST_SET_END(sl, s->s_max);

}

/*
 * This function tries to reap all the slabs in a slab list (not counting any
 * subslabs). It will only reap if the slab list can have a minumum number of
 * slabs AND a minimum percentage of slabs reaped. This way the user won't
 * waste cycles on trivial memory savings.
 */
void
try_reap(slablist_t *sl)
{
	uint64_t slabs_saveable = sl->sl_slabs - (sl->sl_elems / SELEM_MAX);
	float percntg_slabs_saveable = ((float)slabs_saveable) /
	    ((float)sl->sl_slabs);
	float req_percntg = ((float)(sl->sl_mpslabs))/100.0;
	if (slabs_saveable >= sl->sl_mslabs &&
	    percntg_slabs_saveable >= req_percntg) {
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
	/* TODO make reaps work for sublayers too */
	try_reap(csl);
}

/*
 * Maps a singly linked list.
 */
void
slablist_map_sml(slablist_t *sl, slablist_map_t f)
{
	uint64_t nodes = sl->sl_elems;
	uint64_t node = 0;
	small_list_t *s = (small_list_t *)sl->sl_head;
	/*
	 * We place the elems in an array, to avoid calling `f` more than we
	 * have to.
	 */
	slablist_elem_t elems[SMELEM_MAX];
	while (node < nodes) {
		elems[node] = s->sml_data;
		s = s->sml_next;
		node++;
	}
	f(elems, nodes);
}

void
slablist_map_range_sml(slablist_t *sl, slablist_map_t f, slablist_elem_t min,
    slablist_elem_t max)
{
	uint64_t nodes = sl->sl_elems;
	uint64_t node = 0;
	small_list_t *s = (small_list_t *)sl->sl_head;
	/*
	 * We place the elems in an array, to avoid calling `f` more than we
	 * have to.
	 */
	slablist_elem_t elems[SMELEM_MAX];
	while (node < nodes) {
		elems[node] = s->sml_data;
		s = s->sml_next;
		node++;
	}
	int i = 0;
	int j = nodes - 1;
	while (sl->sl_cmp_elem(elems[i], min) < 0) {
		i++;
	}
	while (sl->sl_cmp_elem(elems[j], max) > 0) {
		j--;
	}
	if (j < i) {
		return;
	}
	nodes = j - i + 1;
	f(elems+i, nodes);
}

/*
 * Map a function to every element in the slab list. If this function is called
 * on a sorted slab list, and it invalidates the sorting of the elements in any
 * way, it _will_ have undefined results.
 */
void
slablist_map(slablist_t *sl, slablist_map_t f)
{
	if (IS_SMALL_LIST(sl)) {
		slablist_map_sml(sl, f);
		return;
	}
	uint64_t slabs = sl->sl_slabs;
	uint64_t slab = 0;
	slab_t *s = (slab_t *)sl->sl_head;
	while (slab < slabs) {
		f(s->s_arr, s->s_elems);
		s = s->s_next;
		slab++;
	}
}

void
slablist_map_range(slablist_t *sl, slablist_map_t f, slablist_elem_t min,
    slablist_elem_t max)
{
	if (IS_SMALL_LIST(sl)) {
		slablist_map_range_sml(sl, f, min, max);
		return;
	}
	slab_t *smin = NULL;
	slab_t *smax = NULL;
	find_bubble_up(sl, min, &smin);
	find_bubble_up(sl, max, &smax);
	int i;
	int j;
	if (smin == smax) {
		i = slab_bin_srch(min, smin);
		j = slab_bin_srch(max, smin);
		f((smin->s_arr)+i, j-i);
	}
	slab_t *slab = smin;
	i = slab_bin_srch(min, smin);
	f((slab->s_arr)+i, (slab->s_elems)-i);
	slab = slab->s_next;
	while (slab != smax) {
		f(slab->s_arr, slab->s_elems);
		slab = slab->s_next;
	}
	i = slab_bin_srch(max, slab);
	f(slab->s_arr, i+1);
}


/*
 * Folds left or right on a singly linked list, depending on `f`.
 */
slablist_elem_t
slablist_fold_sml(slablist_t *sl, slablist_fold_t f, slablist_elem_t zero)
{
	uint64_t nodes = sl->sl_elems;
	uint64_t node = 0;
	small_list_t *s = (small_list_t *)sl->sl_head;
	slablist_elem_t accumulator = zero;
	/*
	 * We place the elems in an array, to avoid calling `f` more than we
	 * have to. This has the additional benefit of allowing us to use the
	 * same code for both left and right folds.
	 */
	slablist_elem_t elems[SMELEM_MAX];
	while (node < nodes) {
		elems[node] = s->sml_data;
		s = s->sml_next;
		node++;
	}
	accumulator = f(accumulator, elems, nodes);
	return (accumulator);
}

/*
 * Folds left or right on a singly linked list, depending on `f`, on a range.
 */
slablist_elem_t
slablist_fold_range_sml(slablist_t *sl, slablist_fold_t f, slablist_elem_t min,
    slablist_elem_t max, slablist_elem_t zero)
{
	uint64_t nodes = sl->sl_elems;
	uint64_t node = 0;
	small_list_t *s = (small_list_t *)sl->sl_head;
	slablist_elem_t accumulator = zero;
	/*
	 * We place the elems in an array, to avoid calling `f` more than we
	 * have to. This has the additional benefit of allowing us to use the
	 * same code for both left and right folds.
	 */
	slablist_elem_t elems[SMELEM_MAX];
	while (node < nodes) {
		elems[node] = s->sml_data;
		s = s->sml_next;
		node++;
	}

	int i = 0;
	int j = nodes - 1;
	int b = 0;
	while (i < nodes && (b = sl->sl_bnd_elem(elems[i], min, max)) != 0) {
		i++;
	}
	if (b != 0) {
		return (accumulator);
	}
	while (j >= 0 && (b = sl->sl_bnd_elem(elems[j], min, max)) != 0) {
		j--;
	}
	if (b != 0) {
		return (accumulator);
	}
	nodes = j - i + 1;

	accumulator = f(accumulator, elems+i, nodes);
	return (accumulator);
}

/*
 * Folds a function from start to end of a slab list. The function itself folds
 * from the start to the end of _slab_. This function expects an accumulator,
 * an array of slablist_elem_ts and the size of the array as arguments.  If the
 * slablist holds values (not pointers) the accumulator is updated by f's
 * return value.  If the accumulator is a pointer, then the return value does
 * nothing, as the accumulation is stored in the accumulator structure that is
 * pointed to. In this context, the value of `zero` changes. So before it can
 * be reused in another call to foldr or foldl, it has to be "re-zeroed".
 */
slablist_elem_t
slablist_foldr(slablist_t *sl, slablist_fold_t f, slablist_elem_t zero)
{
	if (sl->sl_elems == 0) {
		return (zero);
	}
	if (IS_SMALL_LIST(sl)) {
		slablist_elem_t ret = slablist_fold_sml(sl, f, zero);
		return (ret);
	}
	uint64_t slabs = sl->sl_slabs;
	uint64_t slab = 0;
	slab_t *s = (slab_t *)sl->sl_head;
	slablist_elem_t accumulator = zero;
	while (slab < slabs) {
		accumulator = f(accumulator, s->s_arr, s->s_elems);
		s = s->s_next;
		slab++;
	}
	return (accumulator);
}

/*
 * Folds a function from end to start of a slab list. All the caveates of
 * pointer vs value that apply to foldr apply here as well. The callback
 * function expects the same arguments, but has to fold _backward_ from the end
 * of the array to start of it. The array pointer that is passed, though,
 * _still_ points to the start of the array (it is the function's
 * responsibility to jump to the end of the array, before folding backward).
 */
slablist_elem_t
slablist_foldl(slablist_t *sl, slablist_fold_t f, slablist_elem_t zero)
{
	if (sl->sl_elems == 0) {
		return (zero);
	}
	if (IS_SMALL_LIST(sl)) {
		slablist_elem_t ret = slablist_fold_sml(sl, f, zero);
		return (ret);
	}
	uint64_t slabs = sl->sl_slabs;
	uint64_t slab = 0;
	slab_t *s = (slab_t *)sl->sl_end;
	slablist_elem_t accumulator = zero;
	while (slab < slabs) {
		accumulator = f(accumulator, s->s_arr, s->s_elems);
		s = s->s_prev;
		slab++;
	}
	return (accumulator);
}

slablist_elem_t
slablist_foldr_range_impl(slablist_t *sl, slablist_fold_t f,
    slablist_elem_t min, slablist_elem_t max, slablist_elem_t zero)
{
	if (sl->sl_elems == 0) {
		return (zero);
	}
	slablist_elem_t accumulator = zero;
	if (IS_SMALL_LIST(sl)) {
		slablist_elem_t ret = slablist_fold_range_sml(sl, f, min, max,
		    zero);
		return (ret);
	}
	slab_t *smin = NULL;
	slab_t *smax = NULL;
	if (sl->sl_sublayers > 0) {
		find_bubble_up(sl, min, &smin);
		find_bubble_up(sl, max, &smax);
	} else {
		find_linear_scan(sl, min, &smin);
		find_linear_scan(sl, max, &smax);
	}
	int i;
	int j;
	if (smin == smax) {
		i = slab_bin_srch(min, smin);
		j = slab_bin_srch(max, smin);
		if (i == smin->s_elems) {
			i--;
		}
		if (j == smin->s_elems) {
			j--;
		}
		/*
		 * The binary search returns an insertion point, the value of
		 * which might not be in range. If the insertion for the min
		 * elem is not in range, we know that there is no element in
		 * that range. If the insertion point for the max elem is not
		 * in range, we are off by one.
		 */
		if (sl->sl_bnd_elem(smin->s_arr[i], min, max) != 0) {
			return (zero);
		}
		if (sl->sl_bnd_elem(smin->s_arr[j], min, max) != 0) {
			if (j > 0) {
				j--;
			}
		}
		accumulator = f(accumulator, (smin->s_arr)+i, j-i+1);
		return (accumulator);
	}
	slab_t *slab = smin;
	i = slab_bin_srch(min, smin);
	if (sl->sl_bnd_elem(slab->s_arr[i], min, max) == 0) {
		accumulator = f(accumulator, (slab->s_arr)+i, (slab->s_elems)-i);
	} else {
		if (i >= (smin->s_elems - 1)) {
			smin = smin->s_next;
			if (smin != NULL) {
				i = 0;
			} else {
				return (zero);
			}
		} else {
			i++;
		}
		if (sl->sl_bnd_elem(smin->s_arr[i], min, max) != 0) {
			return (zero);
		}
	}
	slab = slab->s_next;
	while (slab != smax) {
		accumulator = f(accumulator, slab->s_arr, slab->s_elems);
		slab = slab->s_next;
	}
	i = slab_bin_srch(max, slab);
	if (i == slab->s_elems) {
		i--;
	}
	if (sl->sl_bnd_elem(slab->s_arr[i], min, max) == 0) {
		accumulator = f(accumulator, slab->s_arr, (slab->s_elems)-i);
	} else {
		if (i > 0) {
			i--;
			accumulator = f(accumulator, slab->s_arr, i+1);
		}
	}
	return (accumulator);
}

slablist_elem_t
slablist_foldr_range(slablist_t *sl, slablist_fold_t f,
    slablist_elem_t min, slablist_elem_t max, slablist_elem_t zero)
{
	if (SLABLIST_TEST_FOLDR_RANGE_ENABLED()) {
		int res = test_slablist_foldr_range(sl, min, max);
		SLABLIST_TEST_FOLDR_RANGE(res);
	}
	return (slablist_foldr_range_impl(sl, f, min, max, zero));
}

slablist_elem_t
slablist_foldl_range_impl(slablist_t *sl, slablist_fold_t f, slablist_elem_t min,
    slablist_elem_t max, slablist_elem_t zero)
{
	slablist_elem_t accumulator = zero;
	if (IS_SMALL_LIST(sl)) {
		slablist_elem_t ret = slablist_fold_range_sml(sl, f, min, max,
		    zero);
		return (ret);
	}
	slab_t *smin = NULL;
	slab_t *smax = NULL;
	if (sl->sl_sublayers > 0) {
		find_bubble_up(sl, min, &smin);
		find_bubble_up(sl, max, &smax);
	} else {
		find_linear_scan(sl, min, &smin);
		find_linear_scan(sl, max, &smax);
	}
	int i;
	int j;
	if (smin == smax) {
		i = slab_bin_srch(min, smin);
		j = slab_bin_srch(max, smin);
		accumulator = f(accumulator, (smin->s_arr)+i, j-i);
		return (accumulator);
	}
	slab_t *slab = smax;
	i = slab_bin_srch(max, smax);
	accumulator = f(accumulator, slab->s_arr, i+1);
	slab = slab->s_prev;
	while (slab != smin) {
		accumulator = f(accumulator, slab->s_arr, slab->s_elems);
		slab = slab->s_prev;
	}
	i = slab_bin_srch(min, slab);
	accumulator = f(accumulator, slab->s_arr+i, slab->s_elems - i);
	return (accumulator);
}

slablist_elem_t
slablist_foldl_range(slablist_t *sl, slablist_fold_t f,
    slablist_elem_t min, slablist_elem_t max, slablist_elem_t zero)
{
	if (SLABLIST_TEST_FOLDL_RANGE_ENABLED()) {
		int res = test_slablist_foldl_range(sl, min, max);
		SLABLIST_TEST_FOLDL_RANGE(res);
	}
	return (slablist_foldl_range_impl(sl, f, min, max, zero));
}

/*
 * Returns 0 if 2 lists are equivalent, 1 if not.
 */
int
slablist_cmp(slablist_t *sl1, slablist_t *sl2)
{
	if (sl1->sl_elems != sl2->sl_elems) {
		return (1);
	}

	uint64_t i = 0;
	if (sl1->sl_elems <= SMELEM_MAX) {
		small_list_t *n1 = NULL;
		small_list_t *n2 = NULL;
		n1 = sl1->sl_head;
		n2 = sl2->sl_head;
		selem_t e1;
		selem_t e2;
		while (i < sl1->sl_elems) {
			e1 = n1->sml_data;
			e2 = n2->sml_data;
			if (sl1->sl_cmp_elem(e1, e2)) {
				return (1);
			}
			n1 = n1->sml_next;
			n2 = n2->sml_next;
			i++;
		}
		return (0);
	}


	slab_t *s1 = NULL;
	slab_t *s2 = NULL;
	s1 = sl1->sl_head;
	s2 = sl2->sl_head;
	selem_t e1;
	selem_t e2;
	uint64_t i1 = 0;
	uint64_t i2 = 0;
	while (i < sl1->sl_elems) {
		e1.sle_u = s1->s_arr[i1].sle_u;
		e2.sle_u = s1->s_arr[i2].sle_u;
		i1++;
		i2++;
		if (sl1->sl_cmp_elem(e1, e2)) {
			return (1);
		}
		if (i1 == s1->s_elems) {
			i1 = 0;
			s1 = s1->s_next;
		}
		if (i2 == s2->s_elems) {
			i2 = 0;
			s2 = s2->s_next;
		}
		i++;
	}
	return (0);

}
