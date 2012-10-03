typedef struct slab slab_t;
typedef struct slablist slablist_t;

/*
 * These are the structs that the DTrace consumers see. In slablinfo, we only
 * have a reference (si_arr) to the 4K of data in the slab, which the user can
 * get at by using tracemem. In slinfo, we distill the the `sl->flags` member
 * into 3 different members, so that the user does not have to.
 */
typedef struct slabinfo {
	uint16_t		si_elems;
	uintptr_t		si_max;
	uintptr_t		si_min;
	uintptr_t		si_arr[119];
	uintptr_t		si_next;
	uintptr_t		si_prev;
} slabinfo_t;

typedef struct slinfo {
	uint16_t		sli_req_sublayer;
	uint8_t			sli_sublayers;
	uint8_t			sli_layer;
	uint8_t			sli_is_small_list;
	uintptr_t		sli_head;
	string			sli_name;
	size_t			sli_obj_sz;
	uint8_t 		sli_mcap;
	uint64_t		sli_slabs;
	uint64_t		sli_elems;
	uint8_t			sli_is_sorted;
	uint8_t			sli_is_circular;
	uint8_t			sli_is_sublayer;
} slinfo_t;

/*
 * These are the structs that are used by libslablist, internally. Because they
 * may change between releases, it is important to update this file with the
 * file from the latest release. Otherwise the translators won't work.
 */
struct slab {
        pthread_mutex_t         s_mutex;
        slab_t                  *s_next;
        slab_t                  *s_prev;
        slablist_t              *s_list;
        uint8_t                 s_elems;
        uintptr_t               s_max;
        uintptr_t               s_min;
        uintptr_t               s_arr[119];
};


struct slablist {
        pthread_mutex_t         sl_mutex;
        slablist_t              *sl_prev;
        slablist_t              *sl_next;
        slablist_t              *sl_sublayer;
        slablist_t              *sl_baselayer;
        slablist_t              *sl_superlayer;
        uint16_t                sl_req_sublayer;
        uint8_t                 sl_brk;
        uint8_t                 sl_sublayers;
        uint8_t                 sl_layer;
        uint8_t                 sl_is_small_list;
        void                    *sl_head;
        char                    *sl_name;
        size_t                  sl_obj_sz;
        uint8_t			sl_mcap;
        uint64_t                sl_slabs;
        uint64_t                sl_elems;
        uint8_t                 sl_flags;
        int                     (*sl_cmp_elem)(uintptr_t, uintptr_t);
        int                     (*sl_cmp_super)(uintptr_t, uintptr_t);
        void                    (*sl_ser_list)(int, uintptr_t);
};

/*
 * Here we translate the libslablist structures into the DTrace consumer
 * structures. In slinfo, we use copyin and `&` to get the sl_is_* members.
 */
/*
s->s_arr[0];
(*(s->s_arr+0));
&s->s_arr[0];
&(*(s->s_arr+0));
*/
#pragma D binding "1.6.1" translator
translator slabinfo_t < slab_t *s >
{
	si_elems = *(uint64_t *)copyin((uintptr_t)&s->s_elems,
			sizeof (s->s_elems));
	si_max = *(uintptr_t *)copyin((uintptr_t)&s->s_max,
			sizeof (s->s_max));
	si_min = *(uintptr_t *)copyin((uintptr_t)&s->s_min,
			sizeof (s->s_min));
	si_arr = copyin((uintptr_t)&(s->s_arr[0]),
			sizeof (s->s_arr));
	si_next = *(uintptr_t *)copyin((uintptr_t)&s->s_next,
			sizeof (uintptr_t));
	si_prev = *(uintptr_t *)copyin((uintptr_t)&s->s_prev,
			sizeof (uintptr_t));
};

#pragma D binding "1.6.1" translator
translator slinfo_t < slablist_t *sl >
{
	sli_req_sublayer = *(uint16_t *)copyin((uintptr_t)&sl->sl_req_sublayer,
				sizeof (sl->sl_req_sublayer));
	sli_sublayers = *(uint8_t *)copyin((uint8_t)&sl->sl_sublayers,
				sizeof (sl->sl_sublayers));
	sli_layer = *(uint8_t *)copyin((uintptr_t)&sl->sl_layer,
				sizeof (sl->sl_layer));
	sli_is_small_list = *(uint8_t *)copyin((uintptr_t)&sl->sl_is_small_list,
				sizeof (sl->sl_is_small_list));
	sli_head = *(uintptr_t *)copyin((uintptr_t)&sl->sl_head,
				sizeof (sl->sl_head));
	sli_name = copyinstr(
		*(uintptr_t *)
		copyin(
		(uintptr_t)&sl->sl_name,
		sizeof (sl->sl_name)));
	sli_obj_sz = *(size_t *)copyin((uintptr_t)&sl->sl_obj_sz,
				sizeof (size_t));
	sli_mcap = *(uint8_t *)copyin((uintptr_t)&sl->sl_mcap,
	 			sizeof (sl->sl_mcap));
	sli_slabs = *(uint64_t *)copyin((uintptr_t)&sl->sl_slabs,
				sizeof (sl->sl_slabs));
	sli_elems = *(uint64_t *)copyin((uintptr_t)&sl->sl_elems,
				sizeof (sl->sl_elems));
	sli_is_sorted = (*(uint8_t *)copyin((uintptr_t)&sl->sl_flags,
				sizeof (sl->sl_flags))) & 0x80;
	sli_is_circular = (*(uint8_t *)copyin((uintptr_t)&sl->sl_flags,
				sizeof (sl->sl_flags))) & 0x10;
	sli_is_sublayer = (*(uint8_t *)copyin((uintptr_t)&sl->sl_flags,
				sizeof (sl->sl_flags))) & 0x04;
};
