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
typedef struct { int dummy; } subslabinfo_t;
typedef struct { int dummy; } slinfo_t;
typedef struct { int dummy; } slab_t;
typedef struct { int dummy; } subslab_t;
typedef struct { int dummy; } slablist_t;
typedef union slablist_elem { int dummy; } slablist_elem_t;

provider slablist {
	probe create(slablist_t *sl) : (slinfo_t *sl);
	probe reap_begin(slablist_t *sl) : (slinfo_t *sl);
	probe reap_end(slablist_t *sl) : (slinfo_t *sl);
	probe destroy(slablist_t *sl) : (slinfo_t *sl);
	probe add_begin(slablist_t *sl, slablist_elem_t e, uint64_t r) :
		(slinfo_t *sl, slablist_elem_t e, uint64_t r);
	probe add_end(int);
	probe add_head(slablist_t *sl) : (slinfo_t *sl);
	probe slab_ai(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_aisn(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_aisp(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_aisnm(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_aispm(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_ab(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_aa(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_abm(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_aam(slablist_t *sl, slab_t *s, slablist_elem_t e) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e);
	probe slab_ar(slablist_t *sl, slab_t *s, slablist_elem_t e, int b) :
		(slinfo_t *sl, slabinfo_t *s, slablist_elem_t e, int b);
	probe subslab_ai(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_aisn(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_aisp(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_aisnm(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_aispm(
		slablist_t *sl,
		subslab_t *s, 
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_ab(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_aa(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_abm(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe subslab_aam(
		slablist_t *sl,
		subslab_t *s,
		slab_t *s1,
		subslab_t *s2) :
		(slinfo_t *sl,
		subslabinfo_t *s,
		slabinfo_t *s1,
		subslabinfo_t *s2);
	probe rem_begin(slablist_t *sl, slablist_elem_t e, uint64_t p) :
		(slinfo_t *sl, slablist_elem_t e, uint64_t p);
	probe rem_head(slablist_t *sl) : (slinfo_t *sl);
	probe rem_end(int);
	probe find_begin(slablist_t *sl, slablist_elem_t k) :
		(slinfo_t *sl, slablist_elem_t k);
	probe find_end(int, slablist_elem_t);
	probe get_rand(slablist_t *sl, slablist_elem_t f) :
		(slinfo_t *sl, slablist_elem_t f);
	probe ripple_rem_slab(slablist_t *sl, slab_t *s, subslab_t *b) :
		(slinfo_t *sl, slabinfo_t *s, subslabinfo_t *b);
	probe ripple_rem_subslab(slablist_t *sl, subslab_t *s, subslab_t *b) :
		(slinfo_t *sl, subslabinfo_t *s, subslabinfo_t *b);
	probe ripple_add_slab(slablist_t *sl, slab_t *s, subslab_t *b) :
		(slinfo_t *sl, slabinfo_t *s, subslabinfo_t *b);
	probe ripple_add_subslab(slablist_t *sl, subslab_t *s, subslab_t *b) :
		(slinfo_t *sl, subslabinfo_t *s, subslabinfo_t *b);
	probe slab_mk(slablist_t *sl) : (slinfo_t *sl);
	probe slab_rm(slablist_t *sl) : (slinfo_t *sl);
	probe subslab_mk(slablist_t *sl) : (slinfo_t *sl);
	probe subslab_rm(slablist_t *sl) : (slinfo_t *sl);
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
	probe link_subslab_after(slablist_t *sl, subslab_t *s1, subslab_t *s2) :
		(slinfo_t *sl, subslabinfo_t *s1, subslabinfo_t *s2);
	probe link_subslab_before(slablist_t *sl, subslab_t *s1, subslab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, subslabinfo_t *s2);
	probe unlink_subslab(slablist_t *sl, subslab_t *s) :
		(slinfo_t *sl, subslabinfo_t *s);
	probe find_slab_pos_begin(int);
	probe find_slab_pos_end(int);
	probe fwdshift_begin(slablist_t *sl, slab_t *s, int i) :
		(slinfo_t *sl, slabinfo_t *s, int i);
	probe fwdshift_end();
	probe subfwdshift_begin(slablist_t *sl, subslab_t *s, int i) :
		(slinfo_t *sl, subslabinfo_t *s, int i);
	probe subfwdshift_end();
	probe bwdshift_begin(slablist_t *sl, slab_t *s, int i) :
		(slinfo_t *sl, slabinfo_t *s, int i);
	probe bwdshift_end();
	probe subbwdshift_begin(slablist_t *sl, subslab_t *s, int i) :
		(slinfo_t *sl, subslabinfo_t *s, int i);
	probe subbwdshift_end();
	probe got_here(uint64_t);
	probe set_crumb(slablist_t *s, subslab_t *c, int bc) :
		(slinfo_t *s, subslabinfo_t *c, int bc);
	/*
	 * Fire whenever a slab's max or min changes.
	 */
	probe slab_set_min(slab_t *s) : (slabinfo_t *s);
	probe slab_set_max(slab_t *s) : (slabinfo_t *s);
	probe subslab_set_min(subslab_t *s) : (subslabinfo_t *s);
	probe subslab_set_max(subslab_t *s) : (subslabinfo_t *s);
	probe slab_set_below(slab_t *s) : (slabinfo_t *s);
	probe subslab_set_below(subslab_t *s) : (subslabinfo_t *s);
	/* Traces the sum of usr_elems in testing code */
	probe sum_usr_elems(uint64_t);
	/*
	 * Inc probes increase elem count in slabs or lists. Dec probes
	 * decrease elem count in slabs or lists.
	 */
	probe set_usr_elems(subslab_t *s) : (subslabinfo_t *s);
	probe slab_inc_elems(slab_t *s) : (slabinfo_t *s);
	probe slab_dec_elems(slab_t *s) : (slabinfo_t *s);
	probe subslab_inc_elems(subslab_t *s) : (subslabinfo_t *s);
	probe subslab_dec_elems(subslab_t *s) : (subslabinfo_t *s);
	probe sl_inc_elems(slablist_t *sl) : (slinfo_t *sl);
	probe sl_dec_elems(slablist_t *sl) : (slinfo_t *sl);
	probe sl_inc_slabs(slablist_t *sl) : (slinfo_t *sl);
	probe sl_dec_slabs(slablist_t *sl) : (slinfo_t *sl);
	probe sl_inc_subslabs(slablist_t *sl) : (slinfo_t *sl);
	probe sl_dec_subslabs(slablist_t *sl) : (slinfo_t *sl);
	probe sl_inc_layer(slablist_t *sl) : (slinfo_t *sl);
	probe sl_inc_sublayers(slablist_t *sl) : (slinfo_t *sl);
	/*
	 * When bin searching a subslab. Arg0 is the subslab being searched.
	 * Arg1 is the subslab we are about to compare.
	 */
	probe subslab_bin_srch(subslab_t *s, subslab_t *e) :
		(subslabinfo_t *s, subslabinfo_t *e);
	/*
	 * When bin searching a subslab. Arg0 is the subslab being searched.
	 * Arg1 is the slab we are about to compare.
	 */
	probe subslab_bin_srch_top(subslab_t *s, slab_t *e) :
		(subslabinfo_t *s, slabinfo_t *e);
	/*
	 * When bin searching a slab. Arg0 is the subslab being searched.  Arg1
	 * is the elem we are about to compare.
	 */
	probe slab_bin_srch(slab_t *s, slablist_elem_t e) :
		(slabinfo_t *s, slablist_elem_t e);
	probe bin_search(int, int, int);
	probe bin_search_loop(int, int, int, int, int, void *);
	/*
	 * The following two probes fire when a linear scan begins and ends.
	 */
	probe linear_scan_begin(slablist_t *sl) :
		(slinfo_t *sl);
	probe linear_scan_end(int);
	probe sub_linear_scan_begin(slablist_t *sl) :
		(slinfo_t *sl);
	probe sub_linear_scan_end(int);
	/*
	 * Fires every time we move to a slab (in the baselayer) until we find
	 * a slab that is of appropriate range, or until we run out of slabs.
	 * `s` is the slab that we are currently inspecting.
	 */
	probe linear_scan(slablist_t *sl, slab_t *s) :
		(slinfo_t *sl, slabinfo_t *s);
	probe sub_linear_scan(slablist_t *sl, subslab_t *s) :
		(slinfo_t *sl, subslabinfo_t *s);
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
	probe bubble_up(slablist_t *sl, subslab_t *s) :
		(slinfo_t *sl, subslabinfo_t *s);
	probe bubble_up_top(slablist_t *sl, slab_t *s) :
		(slinfo_t *sl, slabinfo_t *s);
	/*
	 * Fire right before we try to move elems from one slab to an adjacent
	 * slab in slab_generic_rem().
	 */
	probe slab_move_mid_to_next(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe slab_move_mid_to_prev(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe slab_move_next_to_mid(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	probe slab_move_prev_to_mid(slablist_t *sl, slab_t *s1, slab_t *s2) :
		(slinfo_t *sl, slabinfo_t *s1, slabinfo_t *s2);
	/* same as above but for subslabs */
	probe subslab_move_mid_to_next(slablist_t *sl, subslab_t *s1, subslab_t *s2) :
		(slinfo_t *sl, subslabinfo_t *s1, subslabinfo_t *s2);
	probe subslab_move_mid_to_prev(slablist_t *sl, subslab_t *s1, subslab_t *s2) :
		(slinfo_t *sl, subslabinfo_t *s1, subslabinfo_t *s2);
	probe subslab_move_next_to_mid(slablist_t *sl, subslab_t *s1, subslab_t *s2) :
		(slinfo_t *sl, subslabinfo_t *s1, subslabinfo_t *s2);
	probe subslab_move_prev_to_mid(slablist_t *sl, subslab_t *s1, subslab_t *s2) :
		(slinfo_t *sl, subslabinfo_t *s1, subslabinfo_t *s2);
	probe extreme_slab(slab_t *s) : (slabinfo_t *s);
	/*
	 * The first argument of every test probe is an error code that
	 * indicates if a test failed, and why.
	 */
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
	 * Verifies that the sml list does not end (reach NULL next ptr) before
	 * we count the number of elems indicated by the slablist_t anchor.
	 */
	probe test_smlist_nelems(int);
	/*
	 * This probe tests breadcrumb paths.
	 *	Error codes:
	 *		0 Success
	 *		1 The elems in bcpath are not ascending layer-wise.
	 */
	probe test_bread_crumbs(int, int);
	/*
	 * This probe tests the conistency of the slab right before and right
	 * after insert_elem() inserts an elem into a slab. Error codes 3, 4,
	 * and 5 can only happen _after_ an insertion.
	 *    Error codes:
	 *		0 Success
	 *		1 Null slab `s`
	 *		2 Null `s->s_list` bptr
	 *		3 after Actual extrema don't match with cached extrema
	 *		4 First topslab of `s` has different min than `s`.
	 *		5 Last topslab of `s` has different max than `s`.
	 *		6 Elems are not sorted in the slab
	 *		7 Trying to insert into non-zero index of empty slab
	 *		8 Elem is not getting inserted into the right index
	 *
	 *	Args:
	 *		arg0 is the error code
	 *		arg1 is the slab we are inserting into
	 *		arg2 is the elem we are trying to insert.
	 *		arg3 is the index we are trying to insert at in `s`
	 */
	probe test_add_elem(int e, slab_t *s, slablist_elem_t elem, int i) :
		(int e, slabinfo_t *s, slablist_elem_t elem, int i);
	probe test_add_slab(int e, subslab_t *s, slab_t *s1, subslab_t *s2, int i) :
		(int e, subslabinfo_t *s, slabinfo_t *s1, subslabinfo_t *s2, int i);
	/*
	 * This probe tests the conistency of the slab right before and right
	 * after remove_elem() removes an elem from a slab. Error codes 3, 4,
	 * and 5 can only happen _after_ a removal.
	 *    Error codes:
	 *		0 Success
	 *		1 Null slab `s`
	 *		2 Null `s->s_list` bptr
	 *		3 after Actual extrema don't match with cached extrema
	 *		4 First topslab of `s` has different min than `s`.
	 *		5 Last topslab of `s` has different max than `s`.
	 *		6 Elems are not sorted in the slab
	 *		7 Trying to remove from an empty slab.
	 *
	 *	Args:
	 *		arg0 is the error code
	 *		arg1 is the slab we are inserting into
	 *		arg2 is the index we are trying to insert at in `s`
	 */
	probe test_remove_elem(int e, slab_t *s, int i) :
		(int e, slabinfo_t *s, int i);
	probe test_remove_slab(int e, subslab_t *s, int i) :
		(int e, subslabinfo_t *s, int i);
	/*
	 * This probe tests that the search functions used on a slab will all
	 * return the same result. Fires whenever we search a slab.
	 *    Error codes:
	 *		0..2,6 Same as above
	 *		7 binsrch & linsrch of `s` don't give same index.
	 *	Args:
	 *		arg0 is the error code
	 *		arg1 is the slab we are about to search
	 *		arg2 is the element we want to insert or find
	 *		arg3 indicates if arg3 is a value (0) or slab-ptr (1)
	 *		
	 */
	probe test_slab_bin_srch(int e, slab_t *s, slablist_elem_t a) :
		(int e, slabinfo_t *s, slablist_elem_t a);
	probe test_subslab_bin_srch(int e, subslab_t *s, slablist_elem_t a) :
		(int e, subslabinfo_t *s, slablist_elem_t a);
	probe test_subslab_bin_srch_top(int e, slab_t *s, slablist_elem_t a) :
		(int e, slabinfo_t *s, slablist_elem_t a);
	/*
	 * This probe tests find_bubble_up() as it is jumping from layer to
	 * layer. It is similar to the previous test, but the crucial
	 * distinction is that this test doesn't test the correctness of search
	 * at the topslab.
	 *	Error codes:
	 *		0..6 Same as above
	 *		7 binsrch & linsrch of `s` don't give same ptr.
	 *	Args:
	 *		arg0 is the error code
	 *		arg1 is the slab we are about to search 
	 *		arg2 is the subslab we are about to search 
	 *		arg3 is the element we want to insert or find
	 *		arg4 is the layer of the slab we are currently looking
	 *		     for.
	 */
	probe test_find_bubble_up(int e, slab_t *s, subslab_t *ss, slablist_elem_t d,
		int l) :
		(int e, slabinfo_t *s, subslabinfo_t *ss, slablist_elem_t d, int l);
	/*
	 * This probe tests ripple_add() as it is rippling the new slab to the
	 * sublayers.
	 *	Error codes:
	 *		0..5 Same as above
	 *		6 `sb` is not a subslab of `s` / doesn't refer to `s`
	 *	Args:
	 *		arg0 is the error code
	 *		arg1 is the new slab
	 *		arg2 is the immediate subslab
	 *		arg3 is the bread crumb path
	 *		arg4 is the current layer we are on
	 */
	probe test_ripple_add_slab(int e, slab_t *s, subslab_t *sb,
		     void *bc, int b) :
		(int e, slabinfo_t *s, subslabinfo_t *sb, void *bc, int b);
	probe test_ripple_add_subslab(int e, subslab_t *s, subslab_t *sb,
		     slab_t *m, int b) :
		(int e, subslabinfo_t *s, subslabinfo_t *sb, slabinfo_t *m, int b);
	probe test_ripple_update_extrema(int e, subslab_t *s) :
		(int e, subslabinfo_t *s);
	/*
	 * Arg4 is from where in scp we are copying the elem, and arg5 is where
	 * in s2 there is a problem.
	 */
	/*
	 * This probe tests slab_generic_rem(). Specifically, the part of the
	 * function that moves from `s` to `s->next` during the  merge two or
	 * three partially full slabs. 
	 *	Error codes:
	 *		0 Success
	 *		1 There is an inconsistency between `scp` and `sn`
	 *		2 There is an inconsistency between `sncp` and `sn`
	 *	Args:
	 *		arg0 is the error code
	 *		arg1 is the copy of the original slab
	 *		arg2 is the copy of next slab in its current state
	 *		     (after move)
	 *		arg3 is the copy of next slab in its previous state
	 *		     (before move)
	 *		arg4 is the index from which we started copying
	 *		     elements from `scp` to `sn`.
	 *		arg5 is the index at which we have an inconsistency
	 *		     (unequal elems) between `scp`+`sncp` and `sn`.
	 */
	probe test_slab_move_next(int e, slab_t *scp, slab_t *sn, slab_t *sncp, int f, int j) :
		(int e, slabinfo_t *scp, slabinfo_t *sn, slabinfo_t *sncp, int f, int j);
	/*
	 * This probe tests gen_rem_elem(). Specifically, the part of the
	 * function that moves from `s` to `s->prev` during the  merge two or
	 * three partially full slabs. 
	 *	Error codes:
	 *		0 Success
	 *		1 There is an inconsistency between `scp` and `sn`
	 *		2 There is an inconsistency between `sncp` and `sn`
	 *	Args:
	 *		arg0 is the error code
	 *		arg1 is the copy of the original slab
	 *		arg2 is the copy of prev slab in its current state
	 *		     (after move)
	 *		arg3 is the copy of prev slab in its previous state
	 *		     (before move)
	 *		arg4 is the index from which we started copying
	 *		     elements from `scp` to `sp`.
	 *		arg5 is the index at which we have an inconsistency
	 *		     (unequal elems) between `scp`+`spcp` and `sp`.
	 */
	probe test_slab_move_prev(int e, slab_t *scp, slab_t *sp, slab_t *spcp, int f, int j) :
		(int e, slabinfo_t *scp, slabinfo_t *sp, slabinfo_t *spcp, int f, int j);

	probe test_subslab_move_next(int e, subslab_t *scp, subslab_t *sn, subslab_t *sncp, int f, int j) :
		(int e, subslabinfo_t *scp, subslabinfo_t *sn, subslabinfo_t *sncp, int f, int j);
	probe test_subslab_move_prev(int e, subslab_t *scp, subslab_t *sp, subslab_t *spcp, int f, int j) :
		(int e, subslabinfo_t *scp, subslabinfo_t *sp, subslabinfo_t *spcp, int f, int j);
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
	probe test_get_elem_pos(int i, slab_t *s, slab_t *s2, uint64_t i1, uint64_t i2) :
		(int i, slabinfo_t *s, slabinfo_t *s2, uint64_t i1, uint64_t i2);
};

#pragma D attributes Evolving/Evolving/ISA      provider slablist provider
#pragma D attributes Private/Private/Unknown    provider slablist module
#pragma D attributes Private/Private/Unknown    provider slablist function
#pragma D attributes Private/Private/ISA        provider slablist name
#pragma D attributes Evolving/Evolving/ISA      provider slablist args
