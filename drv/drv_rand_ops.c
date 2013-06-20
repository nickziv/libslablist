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
cmpfun(slablist_elem_t v1, slablist_elem_t v2)
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

slablist_elem_t
add_foldr(slablist_elem_t accumulator, slablist_elem_t *arr, uint64_t elems)
{
	uint64_t i = 0;
	while (i < elems) {
		accumulator.sle_u += arr[i].sle_u;
		i++;
	}
	return (accumulator);
}

slablist_elem_t
add_foldl(slablist_elem_t accumulator, slablist_elem_t *arr, uint64_t elems)
{
	uint64_t i = elems - 1;
	while (i > 0) {
		accumulator.sle_u += arr[i].sle_u;
		i--;
	}
	accumulator.sle_u += arr[i].sle_u;
	return (accumulator);
}

void
inc_map(slablist_elem_t *arr, uint64_t elems)
{
	uint64_t i = 0;
	while (i < elems) {
		arr[i].sle_u += 1;
		i++;
	}
}


int
cmpfun_str(slablist_elem_t v1, slablist_elem_t v2)
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

int
bndfun_str(slablist_elem_t e, slablist_elem_t min, slablist_elem_t max)
{
	int cmax = strcmp(e.sle_p, max.sle_p);
	int cmin = strcmp(e.sle_p, min.sle_p);
	if (cmax > 0) {
		return (1);
	}
	if (cmin < 0) {
		return (-1);
	}
	return (0);
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
	slablist_elem_t elem;
	slablist_elem_t randrem;
	slablist_elem_t remd;
	remd.sle_u = 0;
	while (ops < maxops) {
		if (str) {
			elem.sle_p = get_str(fd);
		} else {
			uint64_t rd = get_data(fd);
			elem.sle_u = rd;
		}

		slablist_elem_t found;

		slablist_add(sl, elem, 0);
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
	uint64_t remaining = slablist_get_elems(sl);
	uint64_t type = slablist_get_type(sl);
	char *name = slablist_get_name(sl);
	printf("%s: %d\n", name, type);
	slablist_elem_t elem;
	slablist_elem_t randrem;
	slablist_elem_t zero_rem;
	zero_rem.sle_u = 0;
	slablist_elem_t remd;
	remd.sle_p = NULL;
	int ret;
	while (remaining > 0) {
		if (type == SL_SORTED) {
			randrem = slablist_get_rand(sl);
			ret = slablist_rem(sl, randrem, 0, &remd);
		} else {
			ret = slablist_rem(sl, zero_rem, 0, &remd);
		}
		if (str && remd.sle_p != NULL) {
			rm_str(remd.sle_p);
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

	int intsrt = 0;
	int intord = 0;
	int strsrt = 0;
	int strord = 0;
	int aci = 2;
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
		aci++;
	}
	uint64_t maxops = times;
	slablist_t *sl_str_s = NULL;
	slablist_t *sl_int_s = NULL;
	slablist_t *sl_str_o = NULL;
	slablist_t *sl_int_o = NULL;

	if (strsrt) {
		sl_str_s = slablist_create("strlistsrt", STRMAXSZ, cmpfun_str,
					bndfun_str, SL_SORTED);
		do_ops(sl_str_s, maxops, STR, SRT);
		do_free_remaining(sl_str_s, STR, SRT);
	}
	if (intsrt) {
		sl_int_s = slablist_create("intlistsrt", 8, cmpfun, bndfun,
					SL_SORTED);
		do_ops(sl_int_s, maxops, INT, SRT);
		do_free_remaining(sl_int_s, INT, SRT);
	}
	if (strord) {
		sl_str_o = slablist_create("strlistord", STRMAXSZ, cmpfun_str,
					bndfun_str, SL_ORDERED);
		do_ops(sl_str_o, maxops, STR, ORD);
		do_free_remaining(sl_str_o, STR, ORD);
	}
	if (intord) {
		sl_int_o = slablist_create("intlistord", 8, cmpfun, bndfun,
					SL_ORDERED);
		do_ops(sl_int_o, maxops, INT, ORD);
		do_free_remaining(sl_int_o, INT, ORD);
	}
	end();
	close(fd);
	return (0);
}
