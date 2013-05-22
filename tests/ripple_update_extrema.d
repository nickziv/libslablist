#pragma D option quiet

dtrace:::BEGIN
{
	fail = 0;
	/* see comment in insert.d */
	heap_test = 0;
}

pid$target::mk_slab:return
/heap_test/
{
	slabs[arg1] = 1;
}

pid$target::rm_slab:entry
/heap_test && slabs[arg0] == 2/
{
	fail = 1;
	printf("Trying to free freed slab.\n");
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

pid$target::rm_slab:entry
/heap_test && slabs[arg0] == 1/
{
	slabs[arg0] = 2;
}

slablist$target:::test_ripple_update_extrema
/heap_test && slabs[arg1] == 0/
{
	fail = 1;
	printf("Trying to search unallocated memory (%p) as a slab.\n", arg1);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::test_ripple_update_extrema
/heap_test && slabs[arg1] == 2/
{
	fail = 1;
	printf("Trying to search a freed slab.\n");
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::test_ripple_update_extrema,
slablist$target:::test_insert_slab
{
	fail = arg0;
}

/*
slablist$target:::test_ripple_update_extrema
/arg0 == 0/
{
	printf("%p has %u elems\n", arg1, args[1]->ssi_elems);
}
*/

/*
slablist$target:::test_ripple_update_extrema
/args[1]->ssi_elems > 490/
{
	printf("err = %d\n", arg0);
	self->sa = args[1]->ssi_arr;
	trace(subarrinfo[self->sa]->sai_data);
}
*/

slablist$target:::test_ripple_update_extrema
/arg0 == E_TEST_SUBSLAB_IS_NULL/
{
	fail = arg0;
	printf("Slab provided to insert_elem() is NULL.\n");
	exit(0);
}

slablist$target:::test_ripple_update_extrema
/arg0 == E_TEST_SUBSLAB_LIST_NULL/
{
	printf("Slablist backpointer for %p is NULL.\n", arg1);
}


slablist$target:::test_ripple_update_extrema
/arg0 == E_TEST_SUBSLAB_ELEM_NULL/
{
	printf("An elem in the subslab %p is NULL\n", arg1);
}

slablist$target:::test_ripple_update_extrema
/arg0 == E_TEST_SUBSLAB_UNSORTED/
{
	printf("The elems of the slab %p are not sorted\n", arg1);
}

slablist$target:::test_ripple_update_extrema
/arg0 == E_TEST_SUBSLAB_MIN/
{
	printf("The min of subslab %p does not match what's in the first slab...\n", arg1);
}

slablist$target:::test_insert_slab
/arg0 == E_TEST_SUBSLAB_MAX/
{
	printf("INS\n");
	printf("The max of subslab %p does not match what's in the first slab...\n", arg1);
}

slablist$target:::test_ripple_update_extrema
/arg0 == E_TEST_SUBSLAB_MAX/
{
	printf("The max of subslab %p does not match what's in the first slab...\n", arg1);
}

/*
slablist$target:::test_ripple_update_extrema
/arg0 == 7/
{
	printf("Binary search and linear search for %d give two", arg3);
	printf(" different indexes in slab %p", arg1);
}
*/

slablist$target:::test_ripple_update_extrema,
slablist$target:::test_insert_slab
/arg0 >= E_TEST_SUBSLAB_LIST_NULL/
{
	fail = arg0;
	printf("%s\n", probename);
	printf("\nSubslab details:\n");
	printf("-----------------\n");
	printf("\tmin: %u\n", args[1]->ssi_min);
	printf("\tmax: %u\n", args[1]->ssi_max);
	printf("\telems: %u\n", args[1]->ssi_elems);
	printf("\tnext: %p\n", args[1]->ssi_next);
	printf("\tprev: %p\n\n", args[1]->ssi_prev);
	printf("Subslab array:\n");
	printf("--------------\n");
	self->sa = args[1]->ssi_arr;
	trace(subarrinfo[self->sa]->sai_data);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

dtrace:::END
/fail == 0/
{
	printf("All tests passed.");
}
