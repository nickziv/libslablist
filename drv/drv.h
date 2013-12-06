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

#define	SEED 1997

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
