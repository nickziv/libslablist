inline int E_TEST_SLAB_NULL = 1;
inline int E_TEST_SLAB_LIST_NULL = 2;
inline int E_TEST_SLAB_SUBLAYER = 3;
inline int E_TEST_SLAB_EXTREMA = 4;
inline int E_TEST_SLAB_UNSORTED = 5;
inline int E_TEST_INS_ELEM_INDEX = 6;
inline int E_TEST_INS_ELEM_OUT_ORD = 7;
inline int E_TEST_SLAB_BSRCH = 8;
inline int E_TEST_REM_ELEM_EMPTY = 9;
inline int E_TEST_REM_ELEM_BEYOND = 10;
inline int E_TEST_SLAB_PREV = 11;
inline int E_TEST_SLAB_NEXT = 12;
inline int E_TEST_SUBSLAB_NULL = 13;
inline int E_TEST_SUBSLAB_LIST_NULL = 14;
inline int E_TEST_SUBSLAB_SUBARR_NULL = 15;
inline int E_TEST_SUBSLAB_TOPLAYER = 16;
inline int E_TEST_SUBSLAB_UNSORTED = 17;
inline int E_TEST_SUBSLAB_ELEM_NULL = 18;
inline int E_TEST_SUBSLAB_MIN = 19;
inline int E_TEST_SUBSLAB_MAX = 20;
inline int E_TEST_INS_SLAB_INDEX = 21;
inline int E_TEST_INS_SLAB_OUT_ORD = 22;
inline int E_TEST_SUBSLAB_BSRCH = 23;
inline int E_TEST_SUBSLAB_BSRCH_TOP = 24;
inline int E_TEST_REM_SLAB_EMPTY = 25;
inline int E_TEST_REM_SLAB_BEYOND = 26;
inline int E_TEST_RPLA_SUBSLAB_HAS_NOT = 27;
inline int E_TEST_RPLA_SLAB_HAS_NOT = 28;
inline int E_TEST_SUBSLAB_REFERENCES = 29;
inline int E_TEST_SUBSLAB_PREV = 30;
inline int E_TEST_SUBSLAB_NEXT = 31;
inline int E_TEST_INS_SLAB_LAYER = 32;
inline int E_TEST_INS_SUBSLAB_LAYER = 33;
inline int E_TEST_SUBSLAB_MOVE_NEXT_SCP = 34;
inline int E_TEST_SUBSLAB_MOVE_NEXT_SNCP = 35;
inline int E_TEST_SUBSLAB_MOVE_PREV_SCP = 36;
inline int E_TEST_SUBSLAB_MOVE_PREV_SPCP = 37;
inline int E_TEST_SLAB_MOVE_NEXT_SCP = 38;
inline int E_TEST_SLAB_MOVE_NEXT_SNCP = 39;
inline int E_TEST_SLAB_MOVE_PREV_SCP = 40;
inline int E_TEST_SLAB_MOVE_PREV_SPCP = 41;

inline string e_test_descr[int err] =
	err == 0 ? "[ PASS ]" :
	err == E_TEST_SLAB_NULL ? "[null slab]" :
	err == E_TEST_SLAB_LIST_NULL ? "[null slab list bptr]" :
	err == E_TEST_SLAB_SUBLAYER ? "[slab in sublayer]" :
	err == E_TEST_SLAB_EXTREMA ? "[slab extrema/array mismatch]" :
	err == E_TEST_SLAB_UNSORTED ? "[slab elems unsorted]" :
	err == E_TEST_INS_ELEM_INDEX ? "[slab non zero index]" :
	err == E_TEST_INS_ELEM_OUT_ORD ? "[slab disordered index]" :
	err == E_TEST_SLAB_BSRCH ? "[slab bsrch != slab lsrch]" :
	err == E_TEST_REM_ELEM_EMPTY ? "[rem from empty slab]" :
	err == E_TEST_REM_ELEM_BEYOND ? "[rem index > slab-elems - 1]" :
	err == E_TEST_SLAB_PREV ? "[prev slab > current slab]" :
	err == E_TEST_SLAB_NEXT ? "[next slab < current slab]" :
	err == E_TEST_SUBSLAB_NULL ? "[null subslab]" :
	err == E_TEST_SUBSLAB_LIST_NULL ? "[null subslab list bptr]" :
	err == E_TEST_SUBSLAB_SUBARR_NULL ? "[null subslab subarr ptr]" :
	err == E_TEST_SUBSLAB_TOPLAYER ? "[subslab in toplayer]" :
	err == E_TEST_SUBSLAB_UNSORTED ? "[subslab refs unsorted]" :
	err == E_TEST_SUBSLAB_ELEM_NULL ? "[null subslab ref]" :
	err == E_TEST_SUBSLAB_MIN ? "[subslab min != top slab min]" :
	err == E_TEST_SUBSLAB_MAX ? "[subslab max != top slab max]" :
	err == E_TEST_INS_SLAB_INDEX ? "[subslab non-zero index]" :
	err == E_TEST_INS_SLAB_OUT_ORD ? "[subslab disordered index]" :
	err == E_TEST_SUBSLAB_BSRCH ? "[subslab bsrch != subslab lsrch]" :
	err == E_TEST_SUBSLAB_BSRCH_TOP ?
		"[subslab bsrch top != subslab lsrch top]" :
	err == E_TEST_REM_SLAB_EMPTY ? "[rem from empty subslab]" :
	err == E_TEST_REM_SLAB_BEYOND ? "[rem index > subslab-elems - 1]" :
	err == E_TEST_RPLA_SUBSLAB_HAS_NOT ? "[subslab missing subslab-crumb]" :
	err == E_TEST_RPLA_SLAB_HAS_NOT ? "[subslab missing slab-crumb]" :
	err == E_TEST_SUBSLAB_REFERENCES ? "[subslab missing references]" :
	err == E_TEST_SUBSLAB_PREV ? "[prev subslab > current subslab]" :
	err == E_TEST_SUBSLAB_NEXT ? "[next subslab < current subslab]":
	err == E_TEST_INS_SLAB_LAYER ? "[insert slab-ref bad layer]":
	err == E_TEST_INS_SUBSLAB_LAYER ? "[insert subslab-ref bad layer]":
	err == E_TEST_SUBSLAB_MOVE_NEXT_SCP ? "[subslab move-next-scp]":
	err == E_TEST_SUBSLAB_MOVE_NEXT_SNCP ? "[subslab move-next-sncp]":
	err == E_TEST_SUBSLAB_MOVE_PREV_SCP ? "[subslab move-prev-scp]":
	err == E_TEST_SUBSLAB_MOVE_PREV_SPCP ? "[subslab move-prev-spcp]":
	err == E_TEST_SLAB_MOVE_NEXT_SCP ? "[slab move-next-scp]":
	err == E_TEST_SLAB_MOVE_NEXT_SNCP ? "[slab move-next-sncp]":
	err == E_TEST_SLAB_MOVE_PREV_SCP ? "[slab move-prev-scp]":
	err == E_TEST_SLAB_MOVE_PREV_SPCP ? "[slab move-prev-spcp]":
	"[[BAD ERROR CODE]]";


typedef struct slab slab_t;
typedef struct subslab subslab_t;
typedef struct subarr subarr_t;
typedef struct slablist slablist_t;
typedef struct sbc sbc_t;
typedef struct ssbc ssbc_t;


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
	uintptr_t		si_next;
	uintptr_t		si_prev;
	uintptr_t		si_arr[122];
} slabinfo_t;

typedef struct subslabinfo {
	uint16_t		ssi_elems;
	uintptr_t		ssi_max;
	uintptr_t		ssi_min;
	uintptr_t		ssi_next;
	uintptr_t		ssi_prev;
	subarr_t		*ssi_arr;
} subslabinfo_t;

typedef struct subarrinfo {
	uintptr_t		sai_data[512];
} subarrinfo_t;


typedef struct slinfo {
	uint16_t		sli_req_sublayer;
	uint8_t			sli_sublayers;
	uint8_t			sli_layer;
	uint8_t			sli_is_small_list;
	uintptr_t		sli_head;
	uintptr_t		sli_end;
	string			sli_name;
	size_t			sli_obj_sz;
	uint8_t 		sli_mpslabs;
	uint64_t 		sli_mslabs;
	uint64_t		sli_slabs;
	uint64_t		sli_elems;
	uint8_t			sli_is_sorted;
	uint8_t			sli_is_circular;
	uint8_t			sli_is_sublayer;
} slinfo_t;

typedef struct sbcinfo {
	slab_t			*sbci_slab;
	uint8_t			sbci_on_edge;
} sbcinfo_t;

typedef struct ssbcinfo {
	subslab_t		*ssbci_subslab;
	uint8_t			ssbci_on_edge;
} ssbcinfo_t;



/*
 * These are the structs that are used by libslablist, internally. Because they
 * may change between releases, it is important to update this file with the
 * file from the latest release. Otherwise the translators won't work.
 */
struct slab {
        slab_t                  *s_next;
        slab_t                  *s_prev;
        slablist_t              *s_list;
        uint8_t                 s_elems;
        uintptr_t               s_max;
        uintptr_t               s_min;
        uintptr_t               s_arr[122];
};

struct subarr {
        void                    *sa_data[512];
};

struct subslab {
        pthread_mutex_t         ss_mutex;
        subslab_t               *ss_next;
        subslab_t               *ss_prev;
        slablist_t              *ss_list;
        uint16_t                ss_elems;
        uintptr_t               ss_max;
        uintptr_t               ss_min;
        subarr_t                *ss_arr;
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
        void                    *sl_end;
        char                    *sl_name;
        size_t                  sl_obj_sz;
        uint8_t			sl_mpslabs;
        uint64_t		sl_mslabs;
        uint64_t                sl_slabs;
        uint64_t                sl_elems;
        uint8_t                 sl_flags;
        int                     (*sl_cmp_elem)(uintptr_t, uintptr_t);
        int                     (*sl_cmp_super)(uintptr_t, uintptr_t);
        void                    (*sl_ser_list)(int, uintptr_t);
};

struct sbc {
	slab_t			*sbc_slab;
	uint8_t			sbc_on_edge;
};

struct ssbc {
	subslab_t		*ssbc_subslab;
	uint8_t			ssbc_on_edge;
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
translator subslabinfo_t < subslab_t *s >
{
	ssi_elems = *(uint64_t *)copyin((uintptr_t)&s->ss_elems,
			sizeof (s->ss_elems));
	ssi_max = *(uintptr_t *)copyin((uintptr_t)&s->ss_max,
			sizeof (s->ss_max));
	ssi_min = *(uintptr_t *)copyin((uintptr_t)&s->ss_min,
			sizeof (s->ss_min));
	ssi_next = *(uintptr_t *)copyin((uintptr_t)&s->ss_next,
			sizeof (uintptr_t));
	ssi_prev = *(uintptr_t *)copyin((uintptr_t)&s->ss_prev,
			sizeof (uintptr_t));
	ssi_arr = *(subarr_t **)copyin((uintptr_t)&s->ss_arr,
			sizeof (uintptr_t));
};

#pragma D binding "1.6.1" translator
translator subarrinfo_t < subarr_t *s >
{
	sai_data = copyin((uintptr_t)&(s->sa_data[0]), sizeof (s->sa_data));
};



inline slabinfo_t slabinfo[slab_t *s] = xlate <slabinfo_t *>(s);
inline subslabinfo_t subslabinfo[subslab_t *s] = xlate <subslabinfo_t *>(s);
inline subarrinfo_t subarrinfo[subarr_t *s] = xlate <subarrinfo_t *>(s);

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
	sli_end = *(uintptr_t *)copyin((uintptr_t)&sl->sl_end,
				sizeof (sl->sl_end));
	sli_name = copyinstr(
		*(uintptr_t *)
		copyin(
		(uintptr_t)&sl->sl_name,
		sizeof (sl->sl_name)));
	sli_obj_sz = *(size_t *)copyin((uintptr_t)&sl->sl_obj_sz,
				sizeof (size_t));
	sli_mpslabs = *(uint8_t *)copyin((uintptr_t)&sl->sl_mpslabs,
	 			sizeof (sl->sl_mpslabs));
	sli_mslabs = *(uint64_t *)copyin((uintptr_t)&sl->sl_mslabs,
				sizeof (sl->sl_mslabs));
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

#pragma D binding "1.6.1" translator
translator ssbcinfo_t < ssbc_t *ssbc >
{
	ssbci_subslab = *(subslab_t **)copyin((uintptr_t)&ssbc->ssbc_subslab,
				sizeof (slab_t *));
	ssbci_on_edge = *(uint8_t *)copyin((uintptr_t)&ssbc->ssbc_on_edge,
			sizeof (ssbc->ssbc_on_edge));
};

#pragma D binding "1.6.1" translator
translator sbcinfo_t < sbc_t *sbc >
{
	sbci_slab = *(slab_t **)copyin((uintptr_t)&sbc->sbc_slab,
				sizeof (slab_t *));
	sbci_on_edge = *(uint8_t *)copyin((uintptr_t)&sbc->sbc_on_edge,
			sizeof (sbc->sbc_on_edge));
};
