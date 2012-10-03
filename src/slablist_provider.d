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

typedef struct { int dummy; } slabinfo_t;
typedef struct { int dummy; } slinfo_t;
typedef struct { int dummy; } slab_t;
typedef struct { int dummy; } slablist_t;

provider slablist {
	probe create(slablist_t *sl) : (slinfo_t *sl);
	probe reap_begin(slablist_t *sl) : (slinfo_t *sl);
	probe reap_end(slablist_t *sl) : (slinfo_t *sl);
	probe destroy(slablist_t *sl) : (slinfo_t *sl);
	probe add_begin(slablist_t *sl, uintptr_t e, uint64_t r) :
		(slinfo_t *sl, uintptr_t e, uint64_t r);
	probe add_end(int);
	probe add_head(slablist_t *sl) : (slinfo_t *sl);
	probe add_into(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_into_spill_next(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_into_spill_prev(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_into_spill_next_mk(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_into_spill_prev_mk(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_before(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_after(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_before_mk(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_after_mk(slablist_t *sl, slab_t *s, uintptr_t e) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e);
	probe add_replace(slablist_t *sl, slab_t *s, uintptr_t e, int b) :
		(slinfo_t *sl, slabinfo_t *s, uintptr_t e, int b);
	probe rem_begin(slablist_t *sl, uintptr_t e, uint64_t p) :
		(slinfo_t *sl, uintptr_t e, uint64_t p);
	probe rem_head(slablist_t *sl) : (slinfo_t *sl);
	probe rem_end(int);
	probe find_begin(slablist_t *sl, uintptr_t k) :
		(slinfo_t *sl, uintptr_t k);
	probe find_end(int, uintptr_t);
	probe get_rand(slablist_t *sl, uintptr_t f) :
		(slinfo_t *sl, uintptr_t f);
	probe ripple_rem_slab(slablist_t *sl, slab_t *s, slab_t *b) :
		(slinfo_t *sl, slabinfo_t *s, slabinfo_t *b);
	probe ripple_add_slab(slablist_t *sl, slab_t *s, slab_t *b) :
		(slinfo_t *sl, slabinfo_t *s, slabinfo_t *b);
	probe slab_mk(slablist_t *sl) : (slinfo_t *sl);
	probe slab_rm(slablist_t *sl) : (slinfo_t *sl);
	probe to_small_list(slablist_t *sl) : (slinfo_t *sl);
	probe to_slab(slablist_t *sl) : (slinfo_t *sl);
	probe attach_sublayer(slablist_t *sl1, slablist_t *sl2) :
		(slinfo_t *sl1, slinfo_t *sl2);
	probe detach_sublayer(slablist_t *sl1, slablist_t *sl2) :
		(slinfo_t *sl1, slinfo_t *sl2);
	probe link_sml_node(slablist_t *sl) : (slinfo_t *sl);
	probe unlink_sml_node(slablist_t *sl) : (slinfo_t *sl);
	probe link_slab_after(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe link_slab_before(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe unlink_slab(slablist_t *sl, slab_t *s) :
		(slinfo_t *sl, slabinfo_t *s);
	probe find_slab_pos_begin(int);
	probe find_slab_pos_end(int);
	probe fwdshift_begin(slablist_t *sl, slab_t *s, int i) :
		(slinfo_t *sl, slabinfo_t *s, int i);
	probe fwdshift_end();
	probe bwdshift_begin(slablist_t *sl, slab_t *s, int i) :
		(slinfo_t *sl, slabinfo_t *s, int i);
	probe bwdshift_end();
	probe got_here(uint64_t);
	probe set_crumb(slablist_t *s, slab_t *c, int bc) :
		(slinfo_t *s, slabinfo_t *c, int bc);
	/*
	 * Fire whenever a slab's max or min changes.
	 */
	probe slab_set_min(slab_t *s) : (slabinfo_t *s);
	probe slab_set_max(slab_t *s) : (slabinfo_t *s);
	/*
	 * Inc probes increase elem count in slabs or lists. Dec probes
	 * decrease elem count in slabs or lists.
	 */
	probe slab_inc_elems(slab_t *s) : (slabinfo_t *s);
	probe slab_dec_elems(slab_t *s) : (slabinfo_t *s);
	probe sl_inc_elems(slablist_t *sl) : (slinfo_t *sl);
	probe sl_dec_elems(slablist_t *sl) : (slinfo_t *sl);
	probe sl_inc_slabs(slablist_t *sl) : (slinfo_t *sl);
	probe sl_dec_slabs(slablist_t *sl) : (slinfo_t *sl);
	probe sl_inc_layer(slablist_t *sl) : (slinfo_t *sl);
	probe sl_inc_sublayers(slablist_t *sl) : (slinfo_t *sl);
	/*
	 * When bin searching a subslab. Arg0 is the subslab being searched.
	 * Arg1 is the slab we are about to compare.
	 */
	probe subslab_bin_srch(slab_t *s, slab_t *e) :
		(slabinfo_t *s, slabinfo_t *e);
	/*
	 * When bin searching a slab. Arg0 is the subslab being searched.  Arg1
	 * is the elem we are about to compare.
	 */
	probe slab_bin_srch(slab_t *s, uintptr_t e) :
		(slabinfo_t *s, uintptr_t e);
	probe bin_search(int, int, int);
	probe bin_search_loop(int, int, int, int, int, void *);
	/*
	 * The following two probes fire when a linear scan begins and ends.
	 */
	probe linear_scan_begin(slablist_t *sl) :
		(slinfo_t *sl);
	probe linear_scan_end(int);
	/*
	 * Fires every time we move to a slab (in the baselayer) until we find
	 * a slab that is of appropriate range, or until we run out of slabs.
	 * `s` is the slab that we are currently inspecting.
	 */
	probe linear_scan(slablist_t *sl, slab_t *s) :
		(slinfo_t *sl, slabinfo_t *s);
	/*
	 * The following two probes fire when a bubble up begins and ends.
	 */
	probe bubble_up_begin(slablist_t *sl) :
		(slinfo_t *sl);
	probe bubble_up_end(int);
	/*
	 * Fires every time we bubble up to a superslab. `s` is the superslab
	 * that we just bubbled up to. `sl` is the list.
	 */
	probe bubble_up(slablist_t *sl, slab_t *s) :
		(slinfo_t *sl, slabinfo_t *s);
	probe move_mid_to_next(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe move_mid_to_prev(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe move_next_to_mid(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe move_next_to_prev(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe move_prev_to_mid(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe move_prev_to_next(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	/*
	 * The first argument of every test probe is a boolean that indicates
	 * whether a test was failed (1) or passed (0).
	 */
	/*
	 * This probe has to be enabled, in order to run any of the below
	 * tests. Just enabling the below probes won't test anything.
	 */
	probe test();
	/*
	 * Verifies that the slab is properly converted to an sml.
	 */
	probe test_slab_to_sml(int);
	/*
	 * Verifies that the inserted elements are sorted if the slablist is
	 * the sorted type, and if it is currently backed by the small_list.
	 */
	probe test_smlist_elems_sorted(int);
	/*
	 * Same as previous but for slab-backed slablists.
	 */
	probe test_slab_elems_sorted(int);
	/*
	 * Verifies that the slabs themselves are sorted.
	 */
	probe test_slabs_sorted(int r, slab_t *s1, slab_t *s2) : 
				(int r, slabinfo_t *s1, slabinfo_t *s2);
	/*
	 * Verifies that the max and min of the slab correspont to the first
	 * and last elements. arg1 indicates if min (1) max (2) or both (3) are
	 * inconsistent. arg2 is 1 if the max is smaller than the min.
	 */
	probe test_slab_extrema(int a, int b, int c, slab_t *s) :
		(int a, int b, int c, slabinfo_t *s);
	/*
	 * Same as above, but for subslabs. arg1 is layer, arg2 is the same as
	 * arg1 above.  arg3 is the same as arg2 above.
	 */
	probe test_sublayer_extrema(int a, int b, int c, int d, slab_t *s) :
		(int a, int b, int c, int d, slabinfo_t *s);
	/*
	 * Verifies that the slab points back to its slablist_t, instead of
	 * something else.
	 */
	probe test_slab_bkptr(int);
	/*
	 * Verifies that the results given by bubble_up are same as those given
	 * by linear_scan. arg1 indicates if the return values don't match (1),
	 * the slabs (2), or both (3).
	 */
	probe test_bubble_up(int, int);
	/*
	 * Verifies that the number of elems that are nominally in a slablist,
	 * matches the sum of the number of elems in the individual slabs of
	 * the slablist. arg1 shows how many elems slablist_t counts, and arg2
	 * shows the sum of elems in each individual slab.
	 */
	probe test_nelems(int, uint64_t, uint64_t);
	/*
	 * Verifies that the sml list does not end (reach NULL next ptr) before
	 * we count the number of elems indicated by the slablist_t anchor.
	 */
	probe test_smlist_nelems(int);
	/*
	 * Verified that the number of slabs that are nominally in a slablist
	 * matches number of reachable slabs.
	 */
	probe test_nslabs(int, uint64_t, uint64_t);
	/*
	 * Same as above, but for sublayers. Arg2 is the layer...
	 */
	probe test_sublayer_nelems(int, uint64_t, uint64_t, int);
	/*
	 * Makes sure that no slab has >max elems in it.
	 */
	probe test_slab_elems_max(int, uint64_t);
	/*
	 * Verifies that the slab is part of an sublayer, if passed to a
	 * function that works on sublayers' slabs.
	 */
	probe test_slab_sublayer(int);
	/*
	 * Verifies that all the data stored in an sublayer is properly
	 * sorted. arg0 is a bool, arg1 is the layer (from 1 to N).
	 */
	probe test_sublayer_elems_sorted(int, int);
	/*
	 * Verifies that the underlying slabs are sorted.
	 */
	probe test_sublayers_sorted(int, int);
	/*
	 * Verifies that the sublayers have references to all of the
	 * slablist's slabs.
	 */
	probe test_sublayers_have_all_slabs(int, int);
	/*
	 * Verifies that the slablist implementation maintains some minimum
	 * usage ratio for slabs.
	 */
	probe test_slist_usage_ratio(int);
	/*
	 * Verifies that each element of the breadcrumb trail corresponds to a
	 * distinct layer within a slablist, and is in the right order. arg1 is
	 * the layer. Arg4 is from where in s0 we are copying the elem, and
	 * arg5 is where in s2 there is a problem.
	 */
	probe test_bread_crumbs(int, int);
	probe test_move_next(int i, slab_t *s0, slab_t *s1, slab_t *s2, int f, int j) :
		(int i, slabinfo_t *s0, slabinfo_t *s1, slabinfo_t *s2, int f, int j);
	probe test_move_prev(int i, slab_t *s0, slab_t *s1, slab_t *s2, int f, int j) :
		(int i, slabinfo_t *s0, slabinfo_t *s1, slabinfo_t *s2, int f, int j);
	/*
	 * Verifies that functions don't recieve a NULL argument, if it is
	 * invalid. arg0 is a bool, arg1 is the argument number (0 to N) that
	 * is NULL.
	 */
	probe test_nullarg(int, int);
	/*
	 * Verifies that the functions that add and remove to an sml_list, get
	 * an sml list as an argument, and not something else.
	 */
	probe test_is_sml_list(int);
	/*
	 * Verifies that the functions that add and remove to a slab list, get
	 * a slab list as an argument, and not something else.
	 */
	probe test_is_slab_list(int);
};

#pragma D attributes Evolving/Evolving/ISA      provider slablist provider
#pragma D attributes Private/Private/Unknown    provider slablist module
#pragma D attributes Private/Private/Unknown    provider slablist function
#pragma D attributes Private/Private/ISA        provider slablist name
#pragma D attributes Evolving/Evolving/ISA      provider slablist args
