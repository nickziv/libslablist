/*
 * This command's first argument is the number of operations that need to be
 * executed, before exit.
 *
 * Each operation is either an add or a rem. To a single slab list. Whether to
 * add or remove is chosen at random, as is the element to insert.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <strings.h>
#include <umem.h>
#include <slablist.h>
#ifdef MYSKL
#include <myskl.h>
#endif
#ifdef LIBREDBLACK
#include <redblack.h>
#endif
#include "avl.h"
#include "pavl.h"
#include "tavl.h"
#include "rtavl.h"
#include "rb.h"
#include "prb.h"
#include "bst.h"
#include "pbst.h"
#include "libuutil.h"
#include "btree.h"
#include "btreepriv.h"
#include "skiplist.h"
#include "skiplist_priv.h"

#define	STRMAXSZ 100

#define	SEED 1
#define	STATE_SIZE 128

#define	STR	1
#define INT	0
#define	SRT	1
#define ORD	0

typedef enum struct_type {
	ST_SL,
	ST_UUAVL,
	ST_GNUAVL,
	ST_GNUPAVL,
	ST_GNURTAVL,
	ST_GNUTAVL,
	ST_GNURB,
	ST_GNUPRB,
	ST_JMPCBT,
	ST_JMPCSKL,
	ST_MYSKL,
	ST_REDBLACK,
} struct_type_t;

static unsigned int state0[32];
static unsigned int state1[32] = {
    3,
    0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342,
    0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
    0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86,
    0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7,
    0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc,
    0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb,
    0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b,
    0xf5ad9d0e, 0x8999220b, 0x27fb47b9
};



int fd;
int is_rand;
int is_seq_inc;
int is_seq_dec;

/*
 * This is for debug purposes. We can set up DTrace to stop() the process when
 * end() is called. Once stopped, mdb can be attached.
 */
int
end()
{
	return (10);
}

void
init_rand()
{
	(void)initstate(SEED, (char *)state0, STATE_SIZE);
}

uint64_t
get_data(int unused)
{
	uint64_t ret = random();
	return (ret);
}

uint64_t
old_get_data(int fd)
{
	uint64_t ret;
	int total = 0;
	int red = 0;
	while (total < sizeof (uint64_t)) {
		red = read(fd, &ret, sizeof (uint64_t));
		total += red;
	}

	return (ret);
}

uint8_t
get_data_8b(int fd)
{
	uint8_t ret;
	int total = 0;
	int red = 0;
	while (total < sizeof (uint8_t)) {
		red = read(fd, &ret, sizeof (uint8_t));
		total += red;
	}

	return (ret);
}

char *
mk_str(size_t sz)
{
	return (umem_zalloc(sz, UMEM_DEFAULT));
}

void
rm_str(char *str)
{
	size_t sz = strlen(str);
	sz++;
	umem_free(str, sz);
}

char *
get_str(int fd)
{
	size_t strsize = get_data(fd);
	while (strsize > STRMAXSZ) {
		strsize = strsize/2;
	}
	// XXX v v v this is for experiment only; remove.
	strsize = STRMAXSZ;
	char *s = mk_str(strsize);
	if (s == NULL) {
		return (NULL);
	}
	int i = 0;
	while (i < (strsize - 1)) {
retry:;
		s[i] = get_data_8b(fd);
		if (s[i] == 0 || s[i] > 126 || s[i] < 33) {
			goto retry;
		}
		i++;
	}
	return (s);
}
