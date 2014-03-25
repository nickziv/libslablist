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


#include <stdint.h>
#include <pthread.h>
#include "slablist.h"


/*
 * These are not currently used, but may be used in the future.
 */
#define	NOT_ON_EDGE		0
#define	ON_RIGHT_EDGE		1
#define	ON_LEFT_EDGE		2
#define	ALMOST_ON_RIGHT_EDGE	3
#define	ALMOST_ON_LEFT_EDGE	4

#define	SLADD_BEFORE		0
#define	SLADD_AFTER		1
#define	SLAB_LINK_BEFORE	0
#define	SLAB_LINK_AFTER		1

#define	MAX_LYRS		9

#define	SOI_FWD			1
#define	SOI_BWD			-1

#define	GET_ELEMS(x)\
	((x << 7) >> 7)

#define	SET_ELEMS(x, y)\
		(*(((uint8_t)(&y))+1) = x)


#define	IS_SL_OBJ_SZ_LOCAL(x)\
	(x <= 8)

#define	SLIST_SORTED(x)\
	(x & 0x80)

#define	SLIST_ORDERED(x)\
	!SLIST_SORTED(x)

#define	SLIST_SUBLAYER(x)\
	(x & 0x04)

#define	SLIST_SET_SUBLAYER(x)\
	(x |= 0x04)

#define	SLIST_IS_CIRCULAR(x)\
	(x & 0x10)

#define	SLIST_SET_CIRCULAR(x)\
	(x |= 0x10)

#define	SLIST_IS_SORTING_TEMP(x)\
	(x & 0x10)

#define	SLIST_SET_SORTING_TEMP(x)\
	(x |= 0x10)

#define	FS_IN_RANGE	0
#define	FS_UNDER_RANGE	-1
#define	FS_OVER_RANGE	1

#define	SUBELEM_MAX	(512)
#define	SELEM_MAX	(121)
#define	SMELEM_MAX	(60)

#define	SLAB_FREE_SPACE(s)	((uint64_t)(SELEM_MAX - s->s_elems))
#define	SUBSLAB_FREE_SPACE(s)	((uint64_t)(SUBELEM_MAX - s->ss_elems))

#define	GET_SUBSLAB_ELEM(s, e)		(s->ss_arr->sa_data[e])
#define	SET_SUBSLAB_ELEM(s, e, i)	(s->ss_arr->sa_data[i] = e)

typedef struct slab slab_t;
typedef struct subslab subslab_t;

/*
 * In the beginning, we start adding elements into a singly-linked list,
 * defined in small_list_t. It is not until we reach a list-size of SMELEM_MAX
 * (defined above), that we start using slab_t's to save on memory. A small
 * list can grow to half the size of a slab. This is for memory-efficiency
 * reasons.
 *
 * A singly linked list is _always_ 50% efficient. Once we are half the size of
 * a slab, we can populate a slab with the values in each node, and have a list
 * of 50% efficiency.
 *
 * If we didn't do this, we would start out with a slab that had 1/SELEM_MAX
 * efficiency, then 2/SELEM_MAX, then 3/SELEM_MAX, etc. This way we maintain a
 * minimum efficiency at all times. This is important in cases where one may
 * have a very large number of very small lists.
 *
 * A list can either be sorted or ordered. If inserting into an ordered list,
 * one simply appends the element into it. If inserting into a sorted list, one
 * finds the position at which the element belongs, via a linear search.
 */
typedef struct small_list {
	struct small_list	*sml_next;
	slablist_elem_t		sml_data;
} small_list_t;

/*
 * Once we pass SMELEM_MAX, (which is ~1/2 SELEM_MAX), we start using slabs. A
 * slab is a 1K chunk of memory that contains both meta-data and user-data.
 * Slabs are doubly-linked to each other, so that they can be traversed from
 * left to right and vice-versa. It may be easiest to think of a chain of slabs
 * as a doubly-linked list of arrays.
 *
 * Each slab has a back-pointer to the list that it belongs to. This is
 * neccessary, because the list-structure contains comparison callbacks and
 * bounds-checking callbacks. Each slab also stores copies of its extrema,
 * making it easier to fetch them for bounds-checking (no need to calculate
 * location of maximum), and making it slightly faster (fewer data-cache
 * misses). That the extrema are stored at the very beginning of the slab
 * structure is also supposed to reduce cache misses when checking bounds.
 *
 * When adding `E` to an ordered list, we simply get to the last slab and
 * insert the element. When adding to a sorted list, we use a linear search
 * that checks if `E` is in the bounds of any of the slabs. If so, we insert
 * into the slab[1]. If not, we go to the next one.  If we are over the bounds
 * of the last slab, insert into the last slab. If the slab we wish to insert
 * to is full, we have to 'spill' one element from the target slab into one of
 * the adjacent slabs. If the adjacent slabs are full, we have to create a new,
 * empty adjacent slab and spill into that. See `slablist_add.c` for details.
 *
 * Naturally, the method in the previous paragraph gets slower at a linear
 * rate. At a certain number of slabs, it no longer pays off to use this
 * method. So, we attach a sublayer of subslabs beneath all of the slabs we've
 * added thus far. The subslabs contain pointers to slabs above them.
 *
 * Here is a conceptual representation (that isn't quite accurate of what the
 * layout actually looks like. Say that once we reach 5 slabs out linear-search
 * method starts to cost too much. We do the following:
 *
 *	[S] <-> [S] <-> [S] <-> [S] <-> [S]
 *	    This list is getting costly.
 *
 *	[S] <-> [S] <-> [S] <-> [S] <-> [S]
 *       ^       ^       ^       ^       ^
 *       |       |       |       |       |
 *       |       |       |       |       |
 *       \       \       |       /       /
 *        \       \      |      /       /
 *         \       \     |     /       /
 *          \       \    |    /       /
 *           \       \   |   /       /
 *            \       \  |  /       /
 *             \       \ | /       /
 *              \       \|/       /
 *               +-----+|||+-----+
 *                     [ SS ]
 *          We add a sublayer, which acts as an index, to keep things fast.
 *
 * Now, in subslab SS we can do a binary search for the slab of the appropriate
 * range, instead of doing a linear search on the five slabs at the top. This
 * baselayer expands, just as the toplayer expands. We do a linear search on
 * the baselayer, and then continue with binary search when we find a baseslab
 * which `E` is in bounds of. Once the baselayer reaches the maximum number of
 * (sub)slabs allowed for baselayers (i.e. 5), we attach a new sublayer to what
 * was previously the baselayer. This way we can gaurantee a lg(N) search time
 * and insertion time.
 *
 * The anatomy of the subslab is quite different from the anatomy of a slab.
 * See the following comments for more details.
 *
 * As far as memory efficiency goes, a slab_t's meta-data takes up 56 bytes,
 * while the user-data takes up 968 bytes. This makes the efficiency of a full
 * slab 968/1024 = 94.5%. It has been found empirically that 1K-slabs result in
 * better performance than larger-sized slabs.
 *
 * Large numbers of random insertions and removals, result in the proliferation
 * of partially-full slabs. If it goes on for long enough, the memory savings
 * of a slab list will diminish. In order to maintain some minimum
 * memory-efficiency we can shift all of the elements to the left, and collect
 * any remaining empty slabs at the end of the list --- this is what we call
 * reaping.
 *
 *
 * Take this three-slab slab list for example:
 *
 *	[1|2|3| | | ] <--> [4|5| | | | ] <--> [6|7|8|9| | ]
 *
 * The slabs can hold 6 elements each, and we've inserted 9 elements into the
 * list. The list currently has 9/18 = 50% efficiency. This is unacceptable, so
 * we reap:
 *
 *	[1|2|3|4|5|6] <--> [7|8|9| | | ] <--> [ | | | | | ]
 *
 * Once everything has been shifted to the left, we can delete the leftover
 * slab. Reaping is very expensive, and we try not to do it frequently. We let
 * the user specify how important the efficiency of a single list is with two
 * paramaters. Minimum _percentage_ of slabs that can be collected and minimum
 * _number_ of slabs that can be collected. If we reach the point where we can
 * reap BOTH the minimum number and percentage of slabs, we do so. For small
 * slab lists, the amount of space saved wouldn't be worth the cycles/time we'd
 * spend doing so --- hence the minimum-number requirement. By default we set
 * the percentage to 30% because slab lists very rarely drop below 70%
 * efficiency. And even at 70% efficiency, they use less RAM than any other
 * data structure I've benchmarked (see `drv/drv_gen.c` for details on that).
 *
 * [1]: We find the correct position/index to insert into by using binary
 * search on the slab's embedded array. If not inserting to the end of the
 * slab, we have to shift all elements that follow `E` down by one, using
 * bcopy(). Slabs never have any gaps.
 */
struct slab {
	slablist_elem_t		s_min;
	slablist_elem_t		s_max;
	slab_t			*s_next;
	slab_t 			*s_prev;
	subslab_t		*s_below;
	slablist_t		*s_list;
#if SELEM_MAX < 256
	uint8_t			s_elems;
#else
	uint16_t		s_elems;
#endif
	slablist_elem_t		s_arr[SELEM_MAX];
};

/*
 * Unlike normal slabs (slab_t's), subslabs meta-data is disembodied from the
 * user-data (that is, pointers to slabs). The user-data is stored in the
 * subarr_t struct which is pointed to from the subslab_t struct. See below.
 */
typedef struct subarr {
	void			*sa_data[SUBELEM_MAX];
} subarr_t;

/*
 * The subslab_t operates on the same principals as the slab_t, except that it
 * is not a 1K chunk. It is a 110-byte chunk of meta-data with a pointer to a
 * 4K chunk of user-data (ss_arr). This arrangement was found to be the most
 * efficient. We read and modify the pointers by using the GET_SUBSLAB_ELEM()
 * and SET_SUBSLAB_ELEM() macros.
 *
 * So the pointer chain, from baselayer to toplayer would look something like
 * this:
 *
 *	subslab_t -> subarr_t -> ... -> subslab_t -> subarr_t -> slab_t
 *
 * It is important to note that when we change the extrema of one or more
 * slab_t's, we have to update the extrema of the subslab_t's as well. This is
 * why each subslab_t also has extrema members. We don't want to follow the
 * pointers to the top-layer to figure out what the extrema are when we do
 * linear (or binary) search. Thus, we store copies in the subslab_t's
 * themselves. These copies of course have to be updated in a process known a
 * rippling. Also, if we create a new slab_t at the toplayer, we have to make
 * sure it is reachable from the baselayer. This means inserting the slab_t
 * into a subslab_t. If this insertion doesn't result in the creation of new
 * subslab_t's we're done. But if it does we have to insert the new subslab_t
 * into a subslab_t in the sublayer below (reachable from the ss_below
 * pointer), and so on. See the slablist_add.c code for details on extrema
 * updates and rippling new slab_t's and subslab_t's down the layers.
 *
 * Empirically, the subslabs, their data-arrays, and their associated
 * slablist_t's take up almost 1% of the total memory allocated to a slab list.
 *
 * To speed up random access on a sorted list, each subslab also stores the
 * number of user-data it references at the top layer in `ss_usr_elems`. This
 * allows us traverse the baselayer and the work our way up the superlayers,
 * instead of traversing the toplayer. Speeds us up by some constant factor.
 * Naturally this member has to be updated after adds and removals. Grep around
 * slablist_add.c and slablist_rem.c for instances where we do this.
 */
struct subslab {
	slablist_elem_t		ss_min;
	slablist_elem_t		ss_max;
	subslab_t		*ss_next;
	subslab_t		*ss_prev;
	pthread_mutex_t		ss_mutex;
	subslab_t		*ss_below;
	slablist_t		*ss_list;
	uint16_t		ss_elems;
	uint64_t		ss_usr_elems;
	subarr_t		*ss_arr;
};

/*
 * When adding into a slab or subslab, our functions return a context
 * (add_ctx_t). This context must tell the caller _how_ an element was added.
 * There are 5 ways an element can be added: (1) directly into a non-full slab,
 * (2) into a full slab, spilling the last element to the next slab [which
 * isn't full] (3) inta a full slab, spilling the first element into the
 * previous slab, (4) into the slab before the current slab, (5) into the slab
 * after the current slab. An element can also FAIL to be added due to the
 * presence of an equivalent/duplicate element --- equivalence is determined by
 * the user-provided comparison function.
 */
#define	AC_HOW_INTO		0
#define	AC_HOW_SP_NX		1
#define	AC_HOW_SP_PV		2
#define	AC_HOW_BEFORE		3
#define	AC_HOW_AFTER		4
#define	AC_HOW_EDUP		5

/*
 * Additionally, if a new slab or subslab was created as a result of an
 * addition, one of the pointers `ac_slab_new` and `ac_subslab_new` is set to
 * point to the new slab. One of them MUST be NULL. If the user has specified
 * that a duplicate can be replaced, the `ac_repd_elem` member holds the value
 * of the element that has been replaced. The `ac_subslab_common` member saves
 * the highest subslab that is commonly shared by the newly created subslab and
 * prexisting adjacent subslab.
 *
 * For example:
 *
 *	[s1.0]<->[s1.1]
 *	 ^         ^
 *	 |         |
 *	[s2.0]<->[s2.1]
 *          ^      ^
 *	     \    /
 *	     [s3.0]
 *
 * We want to add s1.2 _after_ s1.0. But it can't fit in s2.0, so we store its
 * pointer in s2.1
 *
 *	[s1.0]<->[s1.2]<->[s1.1]
 *	 ^         ^	    ^
 *	 |         |	    |
 *	 |         |+-------+
 *	 |         ||
 *	[s2.0]<->[s2.1]
 *          ^      ^
 *	     \    /
 *	     [s3.0]
 *
 * In this example, s[3.0] is the common subslab between s1.0 and s1.2. This
 * member, while always updated, isn't currently actually used. It may be used
 * in the future, so we keep it and the code in slablist_add.c around.
 */
typedef struct add_ctx {
	int			ac_how;
	slab_t			*ac_slab_new;
	subslab_t		*ac_subslab_new;
	slablist_elem_t		ac_repd_elem;
	subslab_t		*ac_subslab_common;
} add_ctx_t;

/*
 * Same idea as add_ctx, but the code slablist_rem.c doesn't use it yet.
 */
typedef struct rem_ctx {
	int			rc_how;
	slab_t			*rc_slab_remd;
	subslab_t		*rc_subslab_remd;
	subslab_t		*rc_below;
} rem_ctx_t;

/*
 * This is the handle that stores the state of the slablist. It contains bounds
 * and comparison functions supplied by the user. Every sublayer has one of
 * these that contains meta-data about the sublayer. The baselayer is
 * accessible from the toplayer through the `sl_baselayer` pointer. Also, the
 * first and and last elements are accessible through `sl_head` and `sl_end`.
 * See slablist_cons.c and slablist_add.c for details on how all of the members
 * are initialized and updated. A slablist can have a maximum of 8 sublayers.
 */
struct slablist {
	pthread_mutex_t		sl_mutex;
	slablist_t		*sl_prev;
	slablist_t		*sl_next;
	slablist_t		*sl_sublayer;
	slablist_t		*sl_baselayer;
	slablist_t		*sl_superlayer;
	uint16_t		sl_req_sublayer;
	uint8_t			sl_brk;
	uint8_t			sl_sublayers;
	uint8_t			sl_layer;
	uint8_t			sl_is_small_list;
	void			*sl_head;
	void			*sl_end;
	char			*sl_name;
	size_t			sl_obj_sz;
	uint8_t			sl_mpslabs;
	uint64_t		sl_mslabs;
	uint64_t		sl_slabs;
	uint64_t		sl_elems;
	uint8_t			sl_flags;
	int			(*sl_cmp_elem)(slablist_elem_t,
					slablist_elem_t);
	int			(*sl_bnd_elem)(slablist_elem_t, slablist_elem_t,
					slablist_elem_t);
};


/*
 * Memory allocation functions.
 */
slablist_t *mk_slablist(void);
void rm_slablist(slablist_t *);
slab_t *mk_slab(void);
subslab_t *mk_subslab(void);
subarr_t *mk_subarr(void);
void rm_slab(slab_t *);
void rm_subslab(subslab_t *);
void rm_subarr(subarr_t *);
void *mk_buf(size_t);
void *mk_zbuf(size_t);
void rm_buf(void*, size_t);
small_list_t *mk_sml_node(void);
void rm_sml_node(small_list_t *);
add_ctx_t *mk_add_ctx(void);
void rm_add_ctx(add_ctx_t *);
