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

/*
 * The Slab List Data Structure
 *
 * Overview and Comparison
 * -----------------------
 *
 * The slab list is designed for configurability and maximum memory efficiency,
 * while maintaining the semantics of inserting, removing, and retrieving
 * elements to/from a list or collection. We aim to leverage the advantages of
 * both doubly linked lists and arrays.
 *
 * The main structure of the slab list, is the slab. It is a structure that can
 * hold 952 bytes  of data plus some metadata. The metadata is 66 bytes in size
 * (80 bytes with compiler-inserted padding). This makes the slab exactly 1K in
 * size, meaning that we can fit 4 slabs on one page. The 952 bytes of data is
 * carved into 8-byte elements, meaning that each slab can hold 119 elements.
 *
 * Elements can be any 8-byte sequence (64-bit integers, 64-bit double
 * precision floating point numbers, 64-bit pointers, and so on).
 *
 * The data to total-data ratio per slab is 952/1024 which is 93% efficiency.
 * Increasing the number of elements per-slab increases the efficiency, but
 * decreases performace. Empiracally, the 1K slab offers the best performace
 * improvement per percentage of memory efficiency sacrificed.
 *
 * To get a more visual perspective of what the ratio between data and metadata
 * in a slab is look at the diagrams below:
 *
 * In the slab list ASCII diagram below, the stars and arrows are pointers.
 * "MD" is meta data, that are not next and prev pointers.
 *
 * [*|MD|////952b////|*] -><- [*|MD|////952b////|*]-><-[*|MD|////952b////|*]
 *
 * If we zoom into one of the slabs:
 *
 *		Legend:
 *			nextslab: ptr to the next slab (8 bytes)
 *			prevslab: ptr to the prev slab (8 bytes)
 *			slistptr: ptr to slab list slab belongs to (8 bytes)
 *			NE: number of elements in slab (2 bytes)
 *				^ the compiler may pad this to be 8 bytes
 *			maxvalue: largest element in the slab (8 bytes)
 *			minvalue: smallest element in the slab (8 bytes)
 *			rest: user-given data (952 bytes)
 *
 *   base of slab structure --->  +--------------------------+----------+
 *                                | mutex structure 24 bytes | nextslab |
 *                                +----------+----------+----+----------+
 *                                | prevslab | slistptr | NE | maxvalue |
 *                                +----------+----------+----+----------+
 *                                | minvalue | 952 bytes of user data   |
 *                                +----------+--------------------------+
 *                                |                ||                   |
 *                                |                ||                   |
 *                                |                \/                   |
 *                                :                                     :
 *                                .                                     .
 *                                .                                     .
 *                                .                                     .
 *                                :                                     :
 *                                |                                     |
 *     end of slab structure--->  +-------------------------------------+
 *
 * Every slab has a mutex structure. This structure is needed to support locking
 * concurrency. Currently, slab lists are single-threaded, but support for
 * multiple threads is in the pipeline.
 *
 * As can be seen by the diagram above, user data is an overwhelming portion of
 * the entire slab.
 *
 * Compare slab lists to a doubly linked list. Each node has 2 pointers and an
 * 8-byte element which is 24-bytes per node.
 *
 * The data to total-data ratio per node is 8/24 which is 33% efficiency.
 *
 * The following two diagrams illustrate why this is so:
 *
 * [*|8b|*] -><- [*|8b|*]-><-[*|8b|*]
 *
 * Zooming in on one of the doubly linked list nodes:
 *
 *	base of node ------> +----------+
 *                           | prevnode | (8 bytes)
 *                           +----------+
 *                           | nextnode | (8 bytes)
 *                           +----------+
 *                           | userdata | (8 bytes)
 *      end of node ------>  +----------+
 *
 * As you can see userdata is 1/3 of the whole node.
 *
 * Compare slab lists to the Illumos kernel's AVL Tree implementation, where
 * the metadata per node is 24 bytes in size, and we have 8/32 which is 25%
 * efficiency.
 *
 * The following diagrams illustrate why this is so.
 *
 * In the AVL tree diagram below, MD is meta data which takes up 16 bytes.
 *
 *                        [*|MD..|8b|*]
 *                        /           \
 *                       /             \
 *           [*|MD..|8b|*]             [*|MD..|8b|*]
 *
 *
 * Zooming in on one of the AVL nodes:
 *
 *                               +-----------+
 *                               | lf child  | (8 bytes)
 *                               +-----------+
 *                               | rt child  | (8 bytes)
 *                               +-----------+
 *                               |  avl_pcb  | (8 bytes)
 *                               +-----------+
 *                               |  userdata | (8 bytes)
 *                               +-----------+
 *
 *     The avl_pcb is 64 bits containing:
 *		pointer to the parent (60 bits),
 *		a bit indicating if the node is left or right of the parent,
 *		and a balance value (2 bits).
 *
 *     Zooming in on avl_pcb:
 *
 *     |63                               3|        2        |1          0 |
 *     |----------------------------------|-----------------|-------------|
 *     |   avl_parent hi order bits       | avl_child_index | avl_balance |
 *     |----------------------------------|-----------------|-------------|
 *
 *
 * As can be seen in the second AVL diagram, userdata is 1/4 of the node, which
 * is indeed 15%.
 *
 * To summarize the memory-efficiency of each data strucure's node, from best
 * to worst:
 *
 *		Slab list: 		93%
 *		Doubly linked list: 	33%
 *		AVL Tree: 		25%
 *
 * This is all good and well, when a slab is _full_. However partially full
 * slabs could end up having worse memory efficiency per slab than both doubly
 * linked lists and AVL trees.
 *
 * In order to gaurantee a minimal memory efficiency of 50%, we transparently
 * use a singly-linked list until the number of elements in a slab list reaches
 * 60 (1/2 of a slab). After we reach 1/2 of a slab we copy all of the
 * elements into a new slab, and continue using slabs thereafter.
 *
 * Ranges
 * ------
 *
 * Each slab is ranged. That is, it knows what its maximum and minimum elements
 * are.
 *
 * The usefulness of the range depend on the _type_ of the slab list. A slab
 * list can be either sorted or ordered. A sorted slab list keeps all of the
 * elements sorted (according to a user-given comparison function), whereas an
 * ordered slab list merely inserts each new element to the end of the list.
 *
 * Ranges on slabs are useful for sorted slab lists, as we can use it to
 * traverse the list 119 times more quickly than a linked list, when searching
 * for an element. When we get to a slab that _may_ contain an element, we try
 * to find the index of the element via binary search.
 *
 * Sublayers
 * ---------
 *
 * If the slab list exceeds a certain number of slabs (user-defined), we create
 * a sublayer for that slab list. A sublayer is an internal slab list that
 * contains pointers to slabs as elements. All of the slab-pointers are sorted,
 * and the sublayer's slabs are also ranged. A sublayer is only created for
 * sorted slab lists. Slabs in a sublayer are called sub-slabs. Superslabs are
 * subslabs that are refered to from a lower subslab. Slabs at the highest
 * superlayer (slabs that contain the user's data) are simply known as slabs,
 * or superslabs from the perspective of the highest sublayer. The lowest
 * sublayer is called the baselayer.
 *
 *
 *    slabs from superlayer -------->  [ ][ ][ ][ ][ ][ ][ ][ ] .....
 *                                      ^  ^  ^  ^  ^  ^  ^  ^
 *                                      |  |  |  |  |  |  |  |
 *                                      |  |  |  |  |  |  |  |
 *                               +.....+--+--+--+--+--+--+--+--+......+
 *    a slab from sublayer ----> |.....|* |* |* |* |* |* |* |* |......|
 *                               +.....+--+--+--+--+--+--+--+--+......+
 *                           metadata   slab elems (slab ptrs)
 *
 *
 * This sublayering of slabs gives us a structure that looks like an inverted
 * pyramid.
 *
 *
 *	  highest superlayer --->   _________
 *				    \       /
 *				     \     /
 *				      \   /
 *        baselayer -------------->    \_/
 *
 * If the maximum number of slabs the baselayer can have is N, then it
 * can have at most N*119 elems. If we have only 1 sublayer attached to the
 * slab list, then we have N*119 elems. If we have two we have N*(119^2)
 * user-inserted elems. If we have 3, N*(119^3) elems. And so on.
 *
 * Slablists are limited to a maximum of 8 sublayers. With a completely full
 * slab list, of 8-byte integers, we would consume 0.365 _zettabytes_ of RAM.
 * That exceeds the largest _hard drives_ on the marked today (2012), let alone
 * any amount _RAM_ one could cram into computer.
 *
 * If we wanted to find a slab with element `E`, and our slab list has
 * sublayers, we 1) go to the baselayer , 2) we do a linear search on that
 * sublayer, until we find a slab with the range into which `E` could belong
 * to.
 *
 * The slab that has been found at the baselayer, contains pointers to slabs in
 * the superlayer. We use binary search find the slab in the superlayer into
 * which `E` could belong to. We keep doing this repeatedly until we get to the
 * highest superlayer. The highest superlayer is the layer that contains data
 * that the _user_ gave it.
 *
 * The use of sublayers of slab lists, and binary search gives a search time
 * comparable to balanced binary trees (that is, logarithmic time).
 *
 * Information specific to individual slab lists is encoded in the slabist_t
 * struct.
 *
 * The user can specify the minimum capacity ratio (number of elems in entire
 * slab list, to number of possible elems [119*nslabs]), name, slabs for
 * sublayer, object size, and comparison function.
 *
 * If a slab is added to or removed from the superlayer, we have to ripple the
 * changes down to the sublayer by removeing or adding a reference to the slab.
 * If that sublayer, gains or loses a slab as a result of the previous ripple,
 * we have to ripple that change down to the next sublayer, and so on.
 *
 * DTrace Probes / Test Suite
 * --------------------------
 *
 * This slab list implementation has a profusion of DTrace probes. There are
 * two kinds of probes: tracing probes and testing probes.
 *
 * Tracing probes are ordinary DTrace probes that notify the user of events
 * within libslablist.
 *
 * Testing probes are DTrace probes that, when enabled, trigger tests that
 * verify the correctness of the slab list data structure after each
 * modification, and report failure or success via dtrace. All testing probes
 * have a test_* prefix.
 *
 * Tests are driven by driver programs that barrage slab lists with random data
 * (integers and variable length strings).
 *
 * Memory Allocation
 * -----------------
 *
 * Slab lists use the `libumem` slab allocator available on Illumos, but can be
 * compiled against any libc malloc implementation. This is acheived by
 * abstracting away the allocation code.
 *
 * Instead of doing:
 *
 *	slab_t *s = malloc(sizeof(slab_t));
 * or
 *	slab_t *s = umem_cache_alloc(cache_slab, UMEM_NOFAIL);
 *
 * we do:
 *
 *	slab_t *s = mk_slab();
 *
 * Where `mk_slab` calls the appropriate allocation function, depending on the
 * compile time options.
 *
 * Code Organization
 * -----------------
 *
 * See src/slablist_provider.d for all of the slablist probes (testing and
 * tracing).
 *
 * See src/slablist_add.c and src/slablist_rem.c for the addition and removal
 * algorithms.
 *
 * See src/slablist_find.c for search algorithms used.
 *
 * See src/slablist_umem.c and src/slablist_malloc.c for the specifics behind
 * memory allocation.
 *
 * See src/slablist_test.c for the test functions. They are only called when a
 * corresponding dtrace test-probe is enabled. All telemetry is consumed via
 * dtrace probes.
 *
 * See src/slablist_provider.d for comments regarding individual probes. See
 * slablist_test.c for test function implementation.
 *
 * See src/slablist.d for translators used by the probes.
 *
 * See build/build_lib for build script implementation. If you have primary
 * admin access on Illumos you can run `./build_install_all`. If not, you'll
 * figure something out.
 *
 * See drv/ for various driver programs. drv/drv_rand_ops executes an arbitrary
 * number of random insertions and removals, on 4 kinds of slab lists (sorted
 * int and str list, and ordered int and str list).
 *
 * See tests/ for various test D scripts.
 *
 * See tools/ for various tool D scripts.
 *
 */

#include <stdint.h>
#include <pthread.h>
#include "slablist.h"


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

#define	SLIST_ALWAYS_VACUUMED(x)\
	(x & 0x40)

#define	SLIST_IS_SPARSE_SLAB(x)\
	(x & 0x20)

#define	SLIST_SET_SPARSE_SLAB(x)\
	(x |= 0x20)

#define	SLIST_IS_CIRCULAR(x)\
	(x & 0x10)

#define	SLIST_SET_CIRCULAR(x)\
	(x |= 0x10)

#define	FS_IN_RANGE	0
#define	FS_UNDER_RANGE	-1
#define	FS_OVER_RANGE	1

#define	SELEM_MAX	(119)
#define	SMELEM_MAX	(60)

typedef struct slab slab_t;

typedef struct small_list {
	struct small_list	*sml_next;
	uintptr_t		sml_data;
} small_list_t;

struct slab {
	pthread_mutex_t		s_mutex;
	slab_t			*s_next;
	slab_t 			*s_prev;
	slablist_t		*s_list;
	uint8_t			s_dens;
	uint8_t			s_elems;
	uintptr_t		s_max;
	uintptr_t		s_min;
	uintptr_t		s_arr[SELEM_MAX];
};

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
	int			(*sl_cmp_elem)(uintptr_t, uintptr_t);
	int			(*sl_cmp_super)(uintptr_t, uintptr_t);
};


/*
 * Memory allocation functions.
 */
slablist_t *mk_slablist(void);
void rm_slablist(slablist_t *);
slab_t *mk_slab(void);
void rm_slab(slab_t *);
void *mk_bc();
void rm_bc(void *);
void *mk_buf(size_t);
void *mk_zbuf(size_t);
void rm_buf(void*, size_t);
small_list_t *mk_sml_node(void);
void rm_sml_node(small_list_t *);

/*
 * Shared ripple functions.
 */
void ripple_update_extrema(slab_t **, int);



/*
 * Search functions
 */
int find_slab_in_slab(slab_t *s, uintptr_t elem, slab_t **l);
