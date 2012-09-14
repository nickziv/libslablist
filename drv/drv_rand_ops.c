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
#include <fcntl.h>
#include <strings.h>
#include <umem.h>
#include "slablist.h"

#define	STRMAXSZ 100

int fd;

/*
 * This is for debug purposes. We can set up DTrace to stop() the process when
 * end() is called. Once stopped, mdb can be attached.
 */
int
end()
{
	return (10);
}

int
cmpfun(uintptr_t v1, uintptr_t v2)
{
	uint64_t u1 = (uint64_t)v1;
	uint64_t u2 = (uint64_t)v2;
	if (u1 > u2) {
		return (1);
	}

	if (u1 < u2) {
		return (-1);
	}

	return (0);
}


int
cmpfun_str(uintptr_t v1, uintptr_t v2)
{
	char *s1 = (char *)v1;
	char *s2 = (char *)v2;
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

uint64_t
get_data(int fd)
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

void
do_ops(slablist_t *sl, uint64_t maxops, int str, int ord)
{
	uint64_t ops = 0;
	uintptr_t elem;
	uintptr_t randrem;
	uintptr_t remd = NULL;
	while (ops < maxops) {
		if (str) {
			elem = (uintptr_t)get_str(fd);
		} else {
			elem = get_data(fd);
		}

		uintptr_t found;

		slablist_add(sl, elem, 0, NULL);
		/*
		if (!ord) {
			slablist_find(sl, elem, &found);
		}
		*/

		/*
		if (!(ops % 3) && !ord) {
			*
			 * Remove after every 3rd addition.
			randrem = slablist_get_rand(sl);
			slablist_rem(sl, randrem, 0, &remd);
			if (str && remd != NULL) {
				rm_str((char *)remd);
			}
		}
		*/
		ops++;
	}
}


void
do_free_remaining(slablist_t *sl, int str, int ord)
{
	uint64_t remaining = slablist_getelems(sl);
	uint64_t type = slablist_gettype(sl);
	char *name = slablist_getname(sl);
	printf("%s: %d\n", name, type);
	uintptr_t elem;
	uintptr_t randrem;
	uintptr_t remd = NULL;
	int ret;
	while (remaining > 0) {
		if (type == SL_SORTED) {
			randrem = slablist_get_rand(sl);
			ret = slablist_rem(sl, randrem, 0, &remd);
		} else {
			ret = slablist_rem(sl, NULL, 0, &remd);
		}
		if (str && remd != NULL) {
			rm_str((char *)remd);
		}
		remaining--;
	}

	slablist_destroy(sl);
}


#define	STR	1
#define INT	0
#define	SRT	1
#define ORD	0

int
main(int ac, char *av[])
{
	fd = open("/dev/urandom", O_RDONLY);
	uintptr_t times = 1;

	if (ac > 1) {
		times = (uintptr_t) atoi(av[1]);
	}

	uint64_t maxops = times;
	slablist_t *sl_str_s = NULL;
	slablist_t *sl_int_s = NULL;
	slablist_t *sl_str_o = NULL;
	slablist_t *sl_int_o = NULL;

	printf("=2=\n");
	sl_int_s = slablist_create("intlistsrt", 8, cmpfun, 10, 70, 8,
				SL_SORTED);
	do_ops(sl_int_s, maxops, INT, SRT);
	do_free_remaining(sl_int_s, INT, SRT);
	printf("=1=\n");
	sl_str_s = slablist_create("strlistsrt", STRMAXSZ, cmpfun_str,
				10, 70, 8, SL_SORTED);
	do_ops(sl_str_s, maxops, STR, SRT);
	do_free_remaining(sl_str_s, STR, SRT);
	printf("=4=\n");
	sl_int_o = slablist_create("intlistord", 8, cmpfun, 10, 70,
				8, SL_ORDERED);
	do_ops(sl_int_o, maxops, INT, ORD);
	do_free_remaining(sl_int_o, INT, ORD);
	printf("=3=\n");
	sl_str_o = slablist_create("strlistord", STRMAXSZ, cmpfun_str, 10, 70,
				8, SL_ORDERED);
	do_ops(sl_str_o, maxops, STR, ORD);
	do_free_remaining(sl_str_o, STR, ORD);
	end();
	close(fd);
	return (0);
}
