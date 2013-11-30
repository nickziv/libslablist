#include "libuutil.h"
#include "drv.h"
#include "drv_prov.h"

typedef struct node {
	uu_avl_node_t n;
	slablist_elem_t e;
} node_t;

typedef struct myskl_node {
	MySKL_ns	mn_node;
	slablist_elem_t	mn_e;
} myskl_node_t;

typedef struct avl_table avl_table_t;
typedef struct pavl_table pavl_table_t;
typedef struct tavl_table tavl_table_t;
typedef struct rtavl_table rtavl_table_t;
typedef struct rb_table rb_table_t;
typedef struct prb_table prb_table_t;
typedef struct btree btree_t;
typedef struct skiplist skiplist_t;
typedef struct rbtree redblack_t;
typedef struct uu_avl_cont {
	uu_avl_t	*uuc_avl;
	uu_avl_pool_t	*uuc_avl_pool;
} uu_avl_cont_t;

typedef union container {
	slablist_t	*sl;
	uu_avl_cont_t	uuavl;
	avl_table_t	*gnuavl;
	pavl_table_t	*gnupavl;
	tavl_table_t	*gnutavl;
	rtavl_table_t	*gnurtavl;
	rb_table_t	*gnurb;
	prb_table_t	*gnuprb;
	btree_t		*jmpc_btree;
	skiplist_t	*jmpc_skl;
	redblack_t	*redblack;
	MySKL_t		*myskl;
} container_t;


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


umem_cache_t *cache_node;
umem_cache_t *cache_myskl_node;

int
node_ctor(void *buf, void *ignored, int flags)
{
	node_t *n = buf;
	bzero(n, sizeof (node_t));
	return (0);
}

int
myskl_node_ctor(void *buf, void *ignored, int flags)
{
	myskl_node_t *n = buf;
	bzero(n, sizeof (myskl_node_t));
	return (0);
}

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

node_t *
mk_node()
{
	node_t *r = umem_cache_alloc(cache_node, UMEM_NOFAIL);
	return (r);
}

myskl_node_t *
mk_myskl_node()
{
	myskl_node_t *r = umem_cache_alloc(cache_myskl_node, UMEM_NOFAIL);
	return (r);
}

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

void
myskl_op(container_t *c, slablist_elem_t elem)
{

	myskl_node_t *node = mk_myskl_node();
	node->mn_e = elem;
	MySKLinsertND(c->myskl, node);
}

void
redblack_op(container_t *c, slablist_elem_t elem)
{
	rbsearch(elem.sle_p, c->redblack);
}

typedef void (*struct_subr_t)(container_t *, slablist_elem_t);

struct_subr_t sadd_f[12];

void
do_ops(container_t *ls, struct_type_t t, uint64_t maxops, int str, int ord)
{
	sadd_f[ST_SL] = &sl_op;
	sadd_f[ST_UUAVL] = &uuavl_op;
	sadd_f[ST_GNUAVL] = &gnuavl_op;
	sadd_f[ST_GNUPAVL] = &gnupavl_op;
	sadd_f[ST_GNURTAVL] = &gnurtavl_op;
	sadd_f[ST_GNUTAVL] = &gnutavl_op;
	sadd_f[ST_GNURB] = &gnurb_op;
	sadd_f[ST_GNUPRB] = &gnuprb_op;
	sadd_f[ST_JMPCBT] = &jmpcbt_op;
	sadd_f[ST_JMPCSKL] = &jmpcskl_op;
	sadd_f[ST_MYSKL] = &myskl_op;
	sadd_f[ST_REDBLACK] = &redblack_op;
	uint64_t ops = 0;
	slablist_elem_t elem;
	while (ops < maxops) {
		if (str) {
			elem.sle_p = get_str(fd);
		} else {
			uint64_t rd; 
			if (is_rand) {
				rd = get_data(fd);
			} else if (is_seq_inc) {
				/*
				 * We use get_data here in order to induce
				 * syscall overhead. This way sequential
				 * insertions can be compared to random
				 * insertions.
				 */
				rd = get_data(fd);
				rd = ops + 1;
			} else if (is_seq_dec) {
				/*
				 * See comment in prev if-block.
				 */
				rd = get_data(fd);
				rd = maxops - ops;
			}
			elem.sle_u = rd;
		}
		STRUC_ADD_BEGIN(NULL, elem.sle_u, 0);

		(*sadd_f[t])(ls, elem);

		STRUC_ADD_END(0);
		ops++;
	}
}


void
do_free_remaining(slablist_t *sl, int str, int ord)
{
/*
	uint64_t remaining = slablist_get_elems(sl);
	uint64_t type = slablist_get_type(sl);
	char *name = slablist_get_name(sl);
	printf("%s: %d\n", name, type);
	slablist_elem_t *elem;
	slablist_elem_t *randrem;
	slablist_elem_t *zero_rem;
	zero_rem.sle_u = 0;
	int ret;
	while (remaining > 0) {
		if (type == SL_SORTED) {
			randrem = slablist_get_rand(sl);
			if (str) {
				ret = slablist_rem(sl, randrem, 0, rm_cb_str);
			} else {
				ret = slablist_rem(sl, randrem, 0, NULL);
			}
		} else {
			if (str) {
				ret = slablist_rem(sl, zero_rem, 0, rm_cb_str);
			} else {
				ret = slablist_rem(sl, zero_rem, 0, NULL);
			}
		}
		remaining--;
	}

	slablist_destroy(sl);
*/
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
	is_rand = 0;
	is_seq_inc = 0;
	is_seq_dec = 0;
	int aci = 3;
	if (strcmp("sl", av[1]) == 0) {
		struct_type = ST_SL;
	} else if (strcmp("uuavl", av[1]) == 0) {
		struct_type = ST_UUAVL;
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
	} else if (strcmp("jmpc-btree-512", av[1]) == 0)  {
		struct_type = ST_JMPCBT;
		nodesize = 512;
	} else if (strcmp("jmpc-btree-1024", av[1]) == 0)  {
		struct_type = ST_JMPCBT;
		nodesize = 1024;
	} else if (strcmp("jmpc-btree-4096", av[1]) == 0)  {
		struct_type = ST_JMPCBT;
		nodesize = 4096;
	} else if (strcmp("jmpc-skl-16", av[1]) == 0)  {
		struct_type = ST_JMPCSKL;
		maxlvl = 16;
	} else if (strcmp("myskl-16", av[1]) == 0) {
		struct_type = ST_MYSKL;
		maxlvl = 16;
	} else if (strcmp("redblack", av[1]) == 0) {
		struct_type = ST_REDBLACK;
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
		}
		if (strcmp("seqinc", av[aci]) == 0) {
			is_seq_inc = 1;
		}
		if (strcmp("seqdec", av[aci]) == 0) {
			is_seq_dec = 1;
		}
		aci++;
	}
	uuavl_umem_init();
	myskl_umem_init();
	uint64_t maxops = times;
	uu_avl_pool_t *lp = uu_avl_pool_create("lsp", sizeof (node_t), 0,
		cmpfun, 0);
	uu_avl_t *uuavl_str_s;
	uu_avl_t *uuavl_int_s;
	uu_avl_t *uuavl_str_o;
	uu_avl_t *uuavl_int_o;
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
		cis.sl = slablist_create("intlistsrt", 8, sl_cmpfun, bndfun, SL_SORTED);
		break;
	case ST_UUAVL:
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
		cis.myskl = MySKLinit(maxlvl, bt_cmpfun, NULL, NULL);
		break;
	case ST_REDBLACK:
		cis.redblack = rbinit(bt_cmpfun, NULL);
		break;
	}

	if (strsrt) {
/*
		sl_str_s = slablist_create("strlistsrt", STRMAXSZ, cmpfun_str,
					bndfun_str, SL_SORTED);
		do_ops(sl_str_s, maxops, STR, SRT);
		do_free_remaining(sl_str_s, STR, SRT);
*/
	}
	if (intsrt) {
		do_ops(&cis, struct_type, maxops, INT, SRT);
		//do_free_remaining(sl_int_s, INT, SRT);
	}
	if (strord) {
/*
		sl_str_o = slablist_create("strlistord", STRMAXSZ, cmpfun_str,
					bndfun_str, SL_ORDERED);
		do_ops(sl_str_o, maxops, STR, ORD);
		do_free_remaining(sl_str_o, STR, ORD);
*/
	}
	if (intord) {
/*
		sl_int_o = slablist_create("intlistord", 8, cmpfun, bndfun,
					SL_ORDERED);
		do_ops(sl_int_o, maxops, INT, ORD);
		do_free_remaining(sl_int_o, INT, ORD);
*/
	}
	end();
	return (0);
}
