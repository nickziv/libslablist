#include <strings.h>
#include <stdlib.h>
#include "slablist_impl.h"


int
slablist_umem_init()
{
	return (1);
}

slablist_t *
mk_slablist()
{
	return (calloc(1, sizeof (slablist_t)));
}

void
rm_slablist(slablist_t *s)
{
	free(s);
}


slab_t *
mk_slab()
{
	slab_t *s = calloc(1, sizeof (slab_t));
	return (s);
}

void
rm_slab(slab_t *s)
{
	free(s);
}

small_list_t *
mk_sml_node()
{
	small_list_t *s = calloc(1, sizeof (small_list_t));
	return (s);
}

void
rm_sml_node(small_list_t *s)
{
	free(s);
}

void *
mk_buf(size_t sz)
{
	return (malloc(sz));
}

void *
mk_zbuf(size_t sz)
{
	return (calloc(1, sz));
}

void
rm_buf(void *s, size_t sz)
{
	free(s);
}
