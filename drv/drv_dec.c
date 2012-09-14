#include <stdlib.h>
#include <stdio.h>
#include "slablist.h"

int
cmpfun(uintptr_t v1, uintptr_t v2)
{
	if (v1 > v2) {
		return (1);
	}

	if (v1 < v2) {
		return (-1);
	}

	return (0);
}

int
main(int ac, char *av[])
{
	slablist_t *sl = slablist_create("test", 8, cmpfun, 0.5, 10, 64,
				SL_SORTED);
	// printf("user's slablist: %p\n", sl);
	uintptr_t times = 1;

	if (ac > 1) {
		times = (uintptr_t) atoi(av[1]);
	}

	uintptr_t ELEMS = times;
	uintptr_t j = 1;
	//uintptr_t i = (j << 20);
	uintptr_t i = ELEMS;
	while (i > 0) {
		slablist_add(sl, i, 0, NULL);
		i--;
	}
	i = ELEMS;
	while (i > 0) {
		slablist_rem(sl, i, 0, NULL);
		i--;
	}
}
