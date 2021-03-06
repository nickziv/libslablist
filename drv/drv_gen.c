#ifdef UUTIL
#include "libuutil.h"
#endif
#include "drv.h"
#include "drv_prov.h"

#ifdef UUTIL
typedef struct node {
	uu_avl_node_t n;
	slablist_elem_t e;
} node_t;
#endif

#ifdef MYSKL
typedef struct myskl_node {
	MySKL_ns	mn_node;
	slablist_elem_t	mn_e;
} myskl_node_t;
#endif

typedef struct avl_table avl_table_t;
typedef struct pavl_table pavl_table_t;
typedef struct tavl_table tavl_table_t;
typedef struct rtavl_table rtavl_table_t;
typedef struct rb_table rb_table_t;
typedef struct prb_table prb_table_t;
typedef struct btree btree_t;
typedef struct skiplist skiplist_t;
typedef struct rbtree redblack_t;
#ifdef UUTIL
typedef struct uu_avl_cont {
	uu_avl_t	*uuc_avl;
	uu_avl_pool_t	*uuc_avl_pool;
} uu_avl_cont_t;
#endif

typedef union container {
	slablist_t	*sl;
#ifdef UUTIL
	uu_avl_cont_t	uuavl;
#endif
	avl_table_t	*gnuavl;
	pavl_table_t	*gnupavl;
	tavl_table_t	*gnutavl;
	rtavl_table_t	*gnurtavl;
	rb_table_t	*gnurb;
	prb_table_t	*gnuprb;
	btree_t		*jmpc_btree;
	skiplist_t	*jmpc_skl;
#ifdef LIBREDBLACK
	redblack_t	*redblack;
#endif
#ifdef MYSKL
	MySKL_t		*myskl;
#endif
} container_t;

void
debug_func(int a, int b)
{
	int x = 0;
	x += 1;
	return;
}

int
gnu_cmpfun(const void *z1, const void *z2, void *private)
{
	slablist_elem_t e1;
	slablist_elem_t e2;
	e1.sle_p = z1;
	e2.sle_p = z2;
	if (e1.sle_u > e2.sle_u) {
		return (1);
	}

	if (e1.sle_u < e2.sle_u) {
		return (-1);
	}

	return (0);
}

/*
 * Global Variables.
 */
int do_subseq_sl;
int do_subseq_arr;
slablist_elem_t subseq[100];
int seq_cap;

void
subseq_reverse(slablist_elem_t *arr)
{
	int i = 0;
	int j = seq_cap - 1;
	slablist_elem_t tmp;
	while (i < j) {
		tmp = arr[i];
		arr[i] = arr[j];
		arr[j] = tmp;
		i++;
		j--;
	}
}

int
sl_cmpfun(slablist_elem_t v1, slablist_elem_t v2)
{
	if (v1.sle_u > v2.sle_u) {
		return (1);
	}

	if (v1.sle_u < v2.sle_u) {
		return (-1);
	}

	return (0);
}

int
bndfun(slablist_elem_t e, slablist_elem_t min, slablist_elem_t max)
{
        if (e.sle_u > max.sle_u) {
                return (1);
        }

        if (e.sle_u < min.sle_u) {
                return (-1);
        }
        return (0);
}

#ifdef UUTIL
int
cmpfun(const void *z1, const void *z2, void *private)
{
	node_t *e1 = (node_t *)z1;
	node_t *e2 = (node_t *)z2;
	slablist_elem_t v1 = e1->e;
	slablist_elem_t v2 = e2->e;
	if (v1.sle_u > v2.sle_u) {
		return (1);
	}

	if (v1.sle_u < v2.sle_u) {
		return (-1);
	}

	return (0);
}
#endif

int
bt_cmpfun(void *z1, void *z2)
{
	slablist_elem_t e1;
	slablist_elem_t e2;
	e1.sle_p = z1;
	e2.sle_p = z2;
	if (e1.sle_u > e2.sle_u) {
		return (1);
	}

	if (e1.sle_u < e2.sle_u) {
		return (-1);
	}

	return (0);
}


/*
int
cmpfun_str(slablist_elem_t *v1, slablist_elem_t *v2)
{
	char *s1 = (char *)v1.sle_p;
	char *s2 = (char *)v2.sle_p;
	int ret;
	if (s1 != NULL && s2 != NULL) {
		ret = strcmp(s1, s2);
	} else {
		return (1);
	}
	if (ret < 0) {
		return (-1);
	}
	if (ret > 0) {
		return (1);
	}
	return (ret);
}
*/


#ifdef UMEM
#ifdef UUTIL
umem_cache_t *cache_node;
#endif
#ifdef MYSKL
umem_cache_t *cache_myskl_node;
#endif
#endif /* UMEM */

#ifdef UMEM
#ifdef UUTIL
int
node_ctor(void *buf, void *ignored, int flags)
{
	node_t *n = buf;
	bzero(n, sizeof (node_t));
	return (0);
}
#endif
#endif

#ifdef MYSKL
int
myskl_node_ctor(void *buf, void *ignored, int flags)
{
	myskl_node_t *n = buf;
	bzero(n, sizeof (myskl_node_t));
	return (0);
}
#endif

#ifdef UMEM
#ifdef UUTIL
void
uuavl_umem_init()
{
	cache_node = umem_cache_create("node",
		sizeof (node_t),
		0,
		node_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);
}
#endif

#ifdef MYSKL
void
myskl_umem_init()
{
	cache_myskl_node = umem_cache_create("mysklnode",
		sizeof (myskl_node_t),
		0,
		myskl_node_ctor,
		NULL,
		NULL,
		NULL,
		NULL,
		0);
}
#endif

node_t *
mk_node()
{
	node_t *r = umem_cache_alloc(cache_node, UMEM_NOFAIL);
	return (r);
}

void
rm_node(node_t *n)
{
	bzero(n, sizeof (node_t));
	umem_cache_free(cache_node, n);
}

#ifdef MYSKL
myskl_node_t *
mk_myskl_node()
{
	myskl_node_t *r = umem_cache_alloc(cache_myskl_node, UMEM_NOFAIL);
	return (r);
}
#endif
#endif /* UMEM */

#ifdef UUTIL
void
uuavl_op(container_t *c, slablist_elem_t elem)
{
	node_t *fnd = NULL;
	uu_avl_t *ls = c->uuavl.uuc_avl;
	uu_avl_pool_t *lp = c->uuavl.uuc_avl_pool;
	node_t *node = mk_node();
	uu_avl_node_init(node, &node->n, lp);
	node->e = elem;
	uu_avl_index_t where;
	fnd = uu_avl_find(ls, node, NULL, &where);
	uu_avl_insert(ls, node, where);
}

int
uuavl_rem(container_t *c, slablist_elem_t elem, uint64_t unused,
    slablist_rem_cb_t *rcb)
{
	uu_avl_t *ls = c->uuavl.uuc_avl;
	uu_avl_pool_t *lp = c->uuavl.uuc_avl_pool;
	node_t *node = mk_node();
	uu_avl_node_init(node, &node->n, lp);
	node->e = elem;
	uu_avl_remove(ls, node);
	rm_node(node);
	return (0);
}
#endif

void
gnuavl_op(container_t *c, slablist_elem_t elem)
{
	avl_insert(c->gnuavl, elem.sle_p);
}

void
gnupavl_op(container_t *c, slablist_elem_t elem)
{
	pavl_insert(c->gnupavl, elem.sle_p);
}

void
gnutavl_op(container_t *c, slablist_elem_t elem)
{
	tavl_insert(c->gnutavl, elem.sle_p);
}

void
gnurtavl_op(container_t *c, slablist_elem_t elem)
{
	rtavl_insert(c->gnurtavl, elem.sle_p);
}

void
gnurb_op(container_t *c, slablist_elem_t elem)
{
	rb_insert(c->gnurb, elem.sle_p);
}

void
gnuprb_op(container_t *c, slablist_elem_t elem)
{
	prb_insert(c->gnuprb, elem.sle_p);
}

void
sl_op(container_t *c, slablist_elem_t elem)
{
	slablist_add(c->sl, elem, 0);
}

int
sl_rem(container_t *c, slablist_elem_t elem, uint64_t pos,
    slablist_rem_cb_t *rcb)
{
	int r = slablist_rem(c->sl, elem, pos, rcb);
	return (r);
}


selem_t
sl_sumr(selem_t z, selem_t *e, uint64_t sz)
{
	selem_t r;
	r.sle_u = z.sle_u;
	uint64_t i = 0;
	while (i < sz) {
		r.sle_u += e[i].sle_u;
		i++;
	}
	return (r);
}

selem_t
sl_suml(selem_t z, selem_t *e, uint64_t sz)
{
	selem_t r;
	r.sle_u = z.sle_u;
	uint64_t i = sz;
	while (i > 0) {
		r.sle_u += e[(i - 1)].sle_u;
		i--;
	}
	return (r);
}

void
sl_foldr(container_t *c)
{
	selem_t z;
	selem_t sum;
	z.sle_u = 0;
	sum = slablist_foldr(c->sl, sl_sumr, z);
}

void
sl_foldl(container_t *c)
{
	selem_t z;
	selem_t sum;
	z.sle_u = 0;
	sum = slablist_foldl(c->sl, sl_suml, z);
}

void *
skl_sum(void *z, void *e)
{
	uint64_t zz = (uint64_t)z;
	uint64_t ee = (uint64_t)e;
	uint64_t ret = zz + ee;
	return ((void *)ret);
}

void
jmpcskl_foldr(container_t *c)
{
	uint64_t z = 0;
	void *sum = skl_foldr(c->jmpc_skl, skl_sum, (void *)z);
}

void *
bt_sum(bt_data_t z, bt_data_t e)
{
	uint64_t zz = (uint64_t)z;
	uint64_t ee = (uint64_t)e;
	uint64_t ret = zz + ee;
	bt_data_t btret = (bt_data_t)ret;
	return (btret);
}

void
jmpcbt_foldr(container_t *c)
{
	bt_data_t z = 0;
	bt_data_t sum = bt_foldr(c->jmpc_btree, bt_sum, z);
}

void
jmpcbt_foldl(container_t *c)
{
	bt_data_t z = 0;
	bt_data_t sum = bt_foldl(c->jmpc_btree, bt_sum, z);
}

/*
 * We implement the uuavl folds manually using uu_avl_walk_next, instead of
 * using uu_avl_walk, because uu_avl_walk callbacks have no obvious way of
 * keeping a running sum or anything like that, which is kind of the whole
 * point of a fold. On the bright side, we have inlined the summing. By virtue
 * of operating on single nodes, we can use the same code to fold left and
 * right. Just the flag argument changes.
 */
#ifdef UUTIL
void
uuavl_foldr(container_t *c)
{
	uu_avl_walk_t *wp = uu_avl_walk_start(c->uuavl.uuc_avl, 0);
	void *e;
	uint64_t sum = 0;
	node_t *n;
	while ((e = uu_avl_walk_next(wp)) != NULL) {
		n = (node_t *)e;
		sum += n->e.sle_u;
	}
	uu_avl_walk_end(wp);
}

void
uuavl_foldl(container_t *c)
{
	uu_avl_walk_t *wp = uu_avl_walk_start(c->uuavl.uuc_avl, UU_WALK_REVERSE);
	void *e;
	uint64_t sum = 0;
	node_t *n;
	while ((e = uu_avl_walk_next(wp)) != NULL) {
		n = (node_t *)e;
		sum += n->e.sle_u;
	}
	uu_avl_walk_end(wp);
}
#endif

void
gnuavl_foldr(container_t *c)
{
	struct avl_traverser trav;
	avl_t_init(&trav, c->gnuavl);
	void *node;
	uint64_t sum = 0;
	while ((node = avl_t_next(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnuavl_foldl(container_t *c)
{
	struct avl_traverser trav;
	avl_t_init(&trav, c->gnuavl);
	void *node;
	uint64_t sum = 0;
	while ((node = avl_t_prev(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnupavl_foldr(container_t *c)
{
	struct pavl_traverser trav;
	pavl_t_init(&trav, c->gnupavl);
	void *node;
	uint64_t sum = 0;
	while ((node = pavl_t_next(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnupavl_foldl(container_t *c)
{
	struct pavl_traverser trav;
	pavl_t_init(&trav, c->gnupavl);
	void *node;
	uint64_t sum = 0;
	while ((node = pavl_t_prev(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnurtavl_foldr(container_t *c)
{
	struct rtavl_traverser trav;
	rtavl_t_init(&trav, c->gnurtavl);
	void *node;
	uint64_t sum = 0;
	while ((node = rtavl_t_next(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnurtavl_foldl(container_t *c)
{
	struct rtavl_traverser trav;
	rtavl_t_init(&trav, c->gnurtavl);
	void *node;
	uint64_t sum = 0;
	while ((node = rtavl_t_prev(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnutavl_foldr(container_t *c)
{
	struct tavl_traverser trav;
	tavl_t_init(&trav, c->gnutavl);
	void *node;
	uint64_t sum = 0;
	while ((node = tavl_t_next(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnutavl_foldl(container_t *c)
{
	struct tavl_traverser trav;
	tavl_t_init(&trav, c->gnutavl);
	void *node;
	uint64_t sum = 0;
	while ((node = tavl_t_prev(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnurb_foldr(container_t *c)
{
	struct rb_traverser trav;
	rb_t_init(&trav, c->gnurb);
	void *node;
	uint64_t sum = 0;
	while ((node = rb_t_next(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnurb_foldl(container_t *c)
{
	struct rb_traverser trav;
	rb_t_init(&trav, c->gnurb);
	void *node;
	uint64_t sum = 0;
	while ((node = rb_t_prev(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnuprb_foldr(container_t *c)
{
	struct prb_traverser trav;
	prb_t_init(&trav, c->gnuprb);
	void *node;
	uint64_t sum = 0;
	while ((node = prb_t_next(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
gnuprb_foldl(container_t *c)
{
	struct prb_traverser trav;
	prb_t_init(&trav, c->gnuprb);
	void *node;
	uint64_t sum = 0;
	while ((node = prb_t_prev(&trav)) != NULL) {
		uint64_t num  = (uint64_t)node;
		sum += num;
	}
}

void
jmpcbt_op(container_t *c, slablist_elem_t elem)
{
	bt_insert(c->jmpc_btree, elem.sle_p);
}

void
jmpcskl_op(container_t *c, slablist_elem_t elem)
{
	insertkey(c->jmpc_skl, elem.sle_p);
}

#ifdef MYSKL
void
myskl_op(container_t *c, slablist_elem_t elem)
{

	myskl_node_t *node = mk_myskl_node();
	node->mn_e = elem;
	MySKLinsertND(c->myskl, node);
}
#endif

#ifdef LIBREDBLACK
void
redblack_op(container_t *c, slablist_elem_t elem)
{
	rbsearch(elem.sle_p, c->redblack);
}
#endif

typedef void (*struct_subr_t)(container_t *, slablist_elem_t);
typedef void (*struct_subr_fold_t)(container_t *c);
typedef int (*struct_subr_rem_t)(container_t *, slablist_elem_t, uint64_t,
    slablist_rem_cb_t *);


struct_subr_t sadd_f[12];
struct_subr_rem_t srem_f[12];
struct_subr_fold_t sfdl_f[12];
struct_subr_fold_t sfdr_f[12];

void
set_add_callbacks(void)
{
	sadd_f[ST_SL] = &sl_op;
#ifdef UUTIL
	sadd_f[ST_UUAVL] = &uuavl_op;
#endif
	sadd_f[ST_GNUAVL] = &gnuavl_op;
	sadd_f[ST_GNUPAVL] = &gnupavl_op;
	sadd_f[ST_GNURTAVL] = &gnurtavl_op;
	sadd_f[ST_GNUTAVL] = &gnutavl_op;
	sadd_f[ST_GNURB] = &gnurb_op;
	sadd_f[ST_GNUPRB] = &gnuprb_op;
	sadd_f[ST_JMPCBT] = &jmpcbt_op;
	sadd_f[ST_JMPCSKL] = &jmpcskl_op;
#ifdef MYSKL
	sadd_f[ST_MYSKL] = &myskl_op;
#else
	sadd_f[ST_MYSKL] = NULL;
#endif
#ifdef LIBREDBLACK
	sadd_f[ST_REDBLACK] = &redblack_op;
#else
	sadd_f[ST_REDBLACK] = NULL;
#endif

}

void
set_rem_callbacks(void)
{
	srem_f[ST_SL] = &sl_rem;
#ifdef UUTIL
	srem_f[ST_UUAVL] = &uuavl_rem;
#endif
/*
	srem_f[ST_GNUAVL] = &gnuavl_rem;
	srem_f[ST_GNUPAVL] = &gnupavl_rem;
	srem_f[ST_GNURTAVL] = &gnurtavl_rem;
	srem_f[ST_GNUTAVL] = &gnutavl_rem;
	srem_f[ST_GNURB] = &gnurb_rem;
	srem_f[ST_GNUPRB] = &gnuprb_rem;
	srem_f[ST_JMPCBT] = &jmpcbt_rem;
	srem_f[ST_JMPCSKL] = &jmpcskl_rem;
#ifdef MYSKL
	srem_f[ST_MYSKL] = &myskl_rem;
#else
	srem_f[ST_MYSKL] = NULL;
#endif
#ifdef LIBREDBLACK
	srem_f[ST_REDBLACK] = &redblack_rem;
#else
	srem_f[ST_REDBLACK] = NULL;
#endif

*/
}

void
set_foldr_callbacks(void)
{
	sfdr_f[ST_SL] = &sl_foldr;
#ifdef UUTIL
	sfdr_f[ST_UUAVL] = &uuavl_foldr;
#endif
	sfdr_f[ST_GNUAVL] = gnuavl_foldr;
	sfdr_f[ST_GNUPAVL] = gnupavl_foldr;
	sfdr_f[ST_GNURTAVL] = gnurtavl_foldr;
	sfdr_f[ST_GNUTAVL] = gnutavl_foldr;
	sfdr_f[ST_GNURB] = gnurb_foldr;
	sfdr_f[ST_GNUPRB] = gnuprb_foldr;
	sfdr_f[ST_JMPCBT] = jmpcbt_foldr;
	sfdr_f[ST_JMPCSKL] = jmpcskl_foldr;
#ifdef MYSKL
	sfdr_f[ST_MYSKL] = NULL;
#else
	sfdr_f[ST_MYSKL] = NULL;
#endif
#ifdef LIBREDBLACK
	sfdr_f[ST_REDBLACK] = NULL;
#else
	sfdr_f[ST_REDBLACK] = NULL;
#endif

}

void
set_foldl_callbacks(void)
{
	sfdl_f[ST_SL] = &sl_foldl;
#ifdef UUTIL
	sfdl_f[ST_UUAVL] = &uuavl_foldl;
#endif
	sfdl_f[ST_GNUAVL] = gnuavl_foldl;
	/*
	 * TODO implement the rest of these...
	 */
	sfdl_f[ST_GNUPAVL] = gnupavl_foldl;
	sfdl_f[ST_GNURTAVL] = gnurtavl_foldl;
	sfdl_f[ST_GNUTAVL] = gnutavl_foldl;
	sfdl_f[ST_GNURB] = gnurb_foldl;
	sfdl_f[ST_GNUPRB] = gnuprb_foldl;
	sfdl_f[ST_JMPCBT] = jmpcbt_foldl;
	sfdl_f[ST_JMPCSKL] = NULL;
#ifdef MYSKL
	sfdl_f[ST_MYSKL] = NULL;
#else
	sfdl_f[ST_MYSKL] = NULL;
#endif
#ifdef LIBREDBLACK
	sfdl_f[ST_REDBLACK] = NULL;
#else
	sfdl_f[ST_REDBLACK] = NULL;
#endif
}

void
do_ops(container_t *ls, struct_type_t t, uint64_t maxops, int str, int ord,
    int do_dups)
{

	/*
	 * Currently slab lists are the only structure that can support
	 * _either_ sorted or ordered data.
	 */
	if (t != ST_SL && ord == ORD) {
		return;
	}

	set_add_callbacks();
	set_rem_callbacks();
	set_foldr_callbacks();
	set_foldl_callbacks();
	uint64_t ops = 1;
	slablist_elem_t elem;
	while (ops <= maxops) {
		if (str) {
			elem.sle_p = get_str(fd);
		} else {
			uint64_t rd; 
			if (is_rand) {
				rd = get_data(fd);
			} else if (is_seq_inc) {
				/*
				 * We use get_data here in order to induce
				 * overhead. This way sequential insertions can
				 * be compared to random insertions.
				 */
				rd = get_data(fd);
				rd = ops + 1;
			} else if (is_seq_dec) {
				/*
				 * See comment in prev if-block.
				 */
				rd = get_data(fd);
				rd = (maxops + 1) - ops;
			}
			elem.sle_u = rd;
		}
		STRUC_ADD_BEGIN(NULL, elem.sle_u, 0);

		(*sadd_f[t])(ls, elem);
		STRUC_ADD_END(0);
		debug_func(do_subseq_sl, do_subseq_arr);
		if (do_subseq_sl || do_subseq_arr) {
			subseq[seq_cap] = elem;
			seq_cap++;
		}
		int has_subseq;
		/*
		 * If we can do subseq testing, and we have a subseq of 100, we
		 * test the slablist_subseq call on a slablist.
		 * We test a subseq that is definitely going to be there, and a
		 * reversed subseq that is most likely not going to be in the
		 * list.
		 */
		debug_func(do_subseq_sl, seq_cap);
		if (do_subseq_sl && seq_cap == 100) {
			int l = 0;
			slablist_t *sl_ss = slablist_create("subseq", NULL,
			    NULL, SL_ORDERED);
			while (l < 100) {
				slablist_add(sl_ss, subseq[l], 0);
				l++;
			}
			/* this should evaluate to true for ordered slablists */
			has_subseq = slablist_subseq(ls->sl, sl_ss, NULL, 0);
			slablist_reverse(sl_ss);
			/* this is _likely_ to evaluate to false */
			has_subseq = slablist_subseq(ls->sl, sl_ss, NULL, 0);
		}
		/* Same as the previous but for subseq arrays */
		if (do_subseq_arr && seq_cap == 100) {
			/* this should evaluate to true for ordered slablists */
			has_subseq = slablist_subseq(ls->sl, NULL, subseq,
			    100);
			subseq_reverse(subseq);
			/* this is _likely_ to evaluate to false */
			has_subseq = slablist_subseq(ls->sl, NULL, subseq,
			    100);
			
		}
		if (seq_cap == 100) {
			seq_cap = 0;
		}
		if (do_dups && ops % 2) {
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
			(*sadd_f[t])(ls, elem);
		}

		ops++;
	}
}

void
do_foldrs(container_t *ls, struct_type_t t, int str)
{
	if (str) {
		/* bail, till we figure out what to do */
		return;
	}

	/*
	 * Bail if this structure has no fold function.
	 */
	if (sfdr_f[t] == NULL) {
		return;
	}
	STRUC_FOLDR_BEGIN();
	(*sfdr_f[t])(ls);
	STRUC_FOLDR_END();
}

void
do_foldls(container_t *ls, struct_type_t t, int str)
{
	if (str) {
		/* bail, till we figure out what to do */
		return;
	}
	/*
	 * Bail if this structure has no fold function.
	 */
	if (sfdl_f[t] == NULL) {
		return;
	}
	STRUC_FOLDL_BEGIN();
	(*sfdl_f[t])(ls);
	STRUC_FOLDL_END();
}

void
rm_cb_str(slablist_elem_t e)
{
	return; /* TODO implement this function! */
}

void
do_free_remaining(container_t *ls,  struct_type_t t, int str, int ord,
    uint64_t maxops)
{
	/* TODO handle structure destruction */
	/* TODO implement removal procedures for non-slablists */
	uint64_t ops = 0;
	/*
	 * If we used a random input pattern, we reset the random number
	 * generator. The idea is to remove the same elements we inserted, in
	 * the order they were inserted.
	 */
	uint64_t rd;
	if (is_rand) {
		init_rand();
		while (ops < maxops) {
			rd = get_data(fd);
			slablist_elem_t elem;
			elem.sle_u = rd;
			STRUC_REM_BEGIN(NULL, elem.sle_u, 0);
			srem_f[t](ls, elem, 0, NULL);
			STRUC_REM_END(0);
			ops++;
		}
	} else if (is_seq_inc) {
		while (ops < maxops) {
			/* as with adds, we use get_data to induce overhead */
			rd = get_data(fd);
			slablist_elem_t elem;
			elem.sle_u = ops + 1;
			STRUC_REM_BEGIN(NULL, elem.sle_u, 0);
			srem_f[t](ls, elem, 0, NULL);
			STRUC_REM_END(0);
			ops++;
		}
	} else if (is_seq_dec) {
		while (ops < maxops) {
			/* as with adds, we use get_data to induce overhead */
			rd = get_data(fd);
			slablist_elem_t elem;
			elem.sle_u = maxops - ops;
			STRUC_REM_BEGIN(NULL, elem.sle_u, 0);
			srem_f[t](ls, elem, 0, NULL);
			STRUC_REM_END(0);
			ops++;
		}
	}
}


int
main(int ac, char *av[])
{
	fd = 0;
	uint64_t times = 1;

	if (ac > 1) {
		times = (uint64_t) atoi(av[2]);
	}

	int intsrt = 0;
	int intord = 0;
	int strsrt = 0;
	int strord = 0;
	int nodesize;
	int maxlvl;
	struct_type_t struct_type = ST_SL;
	int do_rem = 0;
	do_subseq_sl = 0;
	do_subseq_arr = 0;
	seq_cap = 0;

	int do_post_sort = 0;
	int do_map = 0;
	int do_foldr = 0;
	int do_foldl = 0;
	int do_dups = 0;
	is_rand = 0;
	is_seq_inc = 0;
	is_seq_dec = 0;
	int aci = 3;
	if (strcmp("sl", av[1]) == 0) {
		struct_type = ST_SL;
	} else if (strcmp("uuavl", av[1]) == 0) {
		struct_type = ST_UUAVL;
#ifndef UUTIL
		printf("%s is not supported on this particular version.\n",
			av[1]);
		exit(0);
#endif
	} else if (strcmp("gnuavl", av[1]) == 0) {
		struct_type = ST_GNUAVL;
	} else if (strcmp("gnupavl", av[1]) == 0) {
		struct_type = ST_GNUPAVL;
	} else if (strcmp("gnutavl", av[1]) == 0) {
		struct_type = ST_GNUTAVL;
	} else if (strcmp("gnurtavl", av[1]) == 0) {
		struct_type = ST_GNURTAVL;
	} else if (strcmp("gnurb", av[1]) == 0) {
		struct_type = ST_GNURB;
	} else if (strcmp("gnuprb", av[1]) == 0) {
		struct_type = ST_GNUPRB;
	} else if (strcmp("jmpc_btree_512", av[1]) == 0)  {
		struct_type = ST_JMPCBT;
		nodesize = 512;
	} else if (strcmp("jmpc_btree_1024", av[1]) == 0)  {
		struct_type = ST_JMPCBT;
		nodesize = 1024;
	} else if (strcmp("jmpc_btree_4096", av[1]) == 0)  {
		struct_type = ST_JMPCBT;
		nodesize = 4096;
	} else if (strcmp("jmpc_skl_16", av[1]) == 0)  {
		struct_type = ST_JMPCSKL;
		maxlvl = 16;
	} else if (strcmp("myskl_16", av[1]) == 0) {
		struct_type = ST_MYSKL;
		maxlvl = 16;
#ifndef MYSKL
		printf("%s is not supported on this particular version.\n",
			av[1]);
		exit(0);
#endif
	} else if (strcmp("redblack", av[1]) == 0) {
		struct_type = ST_REDBLACK;
#ifndef LIBREDBLACK
		printf("%s is not supported on this particular version.\n",
			av[1]);
		exit(0);
#endif
	}
	while (aci < ac) {
		if (strcmp("intsrt", av[aci]) == 0) {
			intsrt++;
		}
		if (strcmp("intord", av[aci]) == 0) {
			intord++;
		}
		if (strcmp("strsrt", av[aci]) == 0) {
			strsrt++;
		}
		if (strcmp("strord", av[aci]) == 0) {
			strord++;
		}
		if (strcmp("rand", av[aci]) == 0) {
			is_rand = 1;
			init_rand();
		}
		if (strcmp("seqinc", av[aci]) == 0) {
			is_seq_inc++;
		}
		if (strcmp("seqdec", av[aci]) == 0) {
			is_seq_dec++;
		}
		if (strcmp("rem", av[aci]) == 0) {
			do_rem++;
		}
		if (strcmp("sort", av[aci]) == 0) {
			do_post_sort++;
		}
		if (strcmp("subseqsl", av[aci]) == 0) {
			do_subseq_sl++;
		}
		if (strcmp("subseqarr", av[aci]) == 0) {
			do_subseq_arr++;
		}
		if (strcmp("map", av[aci]) == 0) {
			do_map++;
		}
		if (strcmp("foldr", av[aci]) == 0) {
			do_foldr++;
		}
		if (strcmp("foldl", av[aci]) == 0) {
			do_foldl++;
		}
		if (strcmp("dup", av[aci]) == 0) {
			do_dups++;
		}
		aci++;
	}

	if (!(is_rand || is_seq_inc || is_seq_dec)) {
		printf("ERROR: Must specify insertion pattern");
		exit(0);
	}
	if (is_rand + is_seq_inc + is_seq_dec > 1) {
		printf("ERROR: Must specifiy ONLY ONE insertion pattern");
		exit(0);
	}
	if (do_dups && (do_subseq_sl || do_subseq_arr)) {
		printf("ERROR: Can't do duplicates and subsequences at the same time");
		exit(0);
	}
	int sl_flag = 0;
	if (intsrt || strsrt) {
		sl_flag = SL_SORTED;
	}
	if (intord || strord) {
		sl_flag = SL_ORDERED;
	}
#ifdef UUTIL
	uuavl_umem_init();
#endif
#ifdef MYSKL
	myskl_umem_init();
#endif
	uint64_t maxops = times;
#ifdef UUTIL
	uu_avl_pool_t *lp = uu_avl_pool_create("lsp", sizeof (node_t), 0,
		cmpfun, 0);
	uu_avl_t *uuavl_str_s;
	uu_avl_t *uuavl_int_s;
	uu_avl_t *uuavl_str_o;
	uu_avl_t *uuavl_int_o;
#endif
	avl_table_t *gnuavl_int_s;
	pavl_table_t *gnupavl_int_s;
	tavl_table_t *gnutavl_int_s;
	rtavl_table_t *gnurtavl_int_s;
	rb_table_t *gnurb_int_s;
	prb_table_t *gnuprb_int_s;
	container_t cis;
	container_t cio;
	container_t css;
	container_t cso;

	printf("%s\n", av[1]);
	switch (struct_type) {


	case ST_SL:
		cis.sl = slablist_create("intlistsrt", sl_cmpfun, bndfun, sl_flag);
		break;
	case ST_UUAVL:
#ifdef UUTIL
		cis.uuavl.uuc_avl_pool = uu_avl_pool_create("lsp", sizeof (node_t), 0,
			cmpfun, 0);

		cis.uuavl.uuc_avl_pool = cis.uuavl.uuc_avl_pool;
		cio.uuavl.uuc_avl_pool = cis.uuavl.uuc_avl_pool;
		css.uuavl.uuc_avl_pool = cis.uuavl.uuc_avl_pool;
		cso.uuavl.uuc_avl_pool = cis.uuavl.uuc_avl_pool;

		css.uuavl.uuc_avl = uu_avl_create(cis.uuavl.uuc_avl_pool, NULL, 0);
		cis.uuavl.uuc_avl = uu_avl_create(cis.uuavl.uuc_avl_pool, NULL, 0);
		cso.uuavl.uuc_avl = uu_avl_create(cis.uuavl.uuc_avl_pool, NULL, 0);
		cio.uuavl.uuc_avl = uu_avl_create(cis.uuavl.uuc_avl_pool, NULL, 0);
#endif
		break;

	case ST_GNUAVL:
		cis.gnuavl = avl_create(gnu_cmpfun, NULL, NULL);
		break;
	case ST_GNUPAVL:
		cis.gnupavl = pavl_create(gnu_cmpfun, NULL, NULL);
		break;
	case ST_GNUTAVL:
		cis.gnutavl = tavl_create(gnu_cmpfun, NULL, NULL);
		break;
	case ST_GNURTAVL:
		cis.gnurtavl = rtavl_create(gnu_cmpfun, NULL, NULL);
		break;
	case ST_GNURB:
		cis.gnurb = rb_create(gnu_cmpfun, NULL, NULL);
		break;
	case ST_GNUPRB:
		cis.gnuprb = prb_create(gnu_cmpfun, NULL, NULL);
		break;
	case ST_JMPCBT:
		cis.jmpc_btree = bt_create(bt_cmpfun, nodesize);
		break;
	case ST_JMPCSKL:
		cis.jmpc_skl = createskiplist(bt_cmpfun, maxlvl, 1, (void *)0,
				(void *)UINT64_MAX);
		break;
	case ST_MYSKL:
#ifdef MYSKL
		cis.myskl = MySKLinit(maxlvl, bt_cmpfun, NULL, NULL);
#endif
		break;
	case ST_REDBLACK:
#ifdef LIBREDBLACK
		cis.redblack = rbinit(bt_cmpfun, NULL);
#endif
		break;
	}

	if (strsrt) {
/*
		sl_str_s = slablist_create("strlistsrt", cmpfun_str,
					bndfun_str, SL_SORTED);
		do_ops(sl_str_s, maxops, STR, SRT);
		do_free_remaining(sl_str_s, STR, SRT);
*/
	}
	if (intsrt) {
		do_ops(&cis, struct_type, maxops, INT, SRT, do_dups);
		if (do_rem) {
			do_free_remaining(&cis, struct_type, INT, SRT, maxops);
		}
		if (do_foldr) {
			do_foldrs(&cis, struct_type, INT);
		}
		if (do_foldl) {
			do_foldls(&cis, struct_type, INT);
		}
	}
	if (strord) {
/*
		sl_str_o = slablist_create("strlistord", cmpfun_str,
					bndfun_str, SL_ORDERED);
		do_ops(sl_str_o, maxops, STR, ORD);
		do_free_remaining(sl_str_o, STR, ORD);
*/
	}
	if (intord) {
		do_ops(&cis, struct_type, maxops, INT, ORD, do_dups);
		if (struct_type == ST_SL && do_post_sort) {
			slablist_sort(cis.sl, sl_cmpfun, bndfun);
		}
/*
		sl_int_o = slablist_create("intlistord", cmpfun, bndfun,
					SL_ORDERED);
		do_ops(sl_int_o, maxops, INT, ORD);
		do_free_remaining(sl_int_o, INT, ORD);
*/
	}
	end();
	return (0);
}
