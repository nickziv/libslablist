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

/*
slablist$target:::test_ripple_add
/heap_test && slabs[arg1] == 0/
{
	fail = 1;
	printf("Trying to walk over unallocated memory (%p) as a slab.\n", arg1);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::test_ripple_add
/heap_test && slabs[arg1] == 2/
{
	fail = 1;
	printf("Trying to walk over a freed slab.\n");
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}
*/

slablist$target:::test_ripple_add_slab
/arg0 == E_TEST_SLAB_IS_NULL/
{
	fail = arg0;
	printf("Slab provided to in bread crumbs is NULL.\n");
	exit(0);
}

slablist$target:::test_ripple_add_subslab
/arg0 == E_TEST_SUBSLAB_IS_NULL/
{
	fail = arg0;
	printf("Subslab provided to in bread crumbs is NULL.\n");
	exit(0);
}

slablist$target:::test_ripple_add_slab
/arg0 == E_TEST_SLAB_LIST_NULL/
{
	printf("Slablist backpointer for %p is NULL.\n", arg1);
}
slablist$target:::test_ripple_add_subslab
/arg0 == E_TEST_SUBSLAB_LIST_NULL/
{
	printf("Slablist backpointer for %p is NULL.\n", arg1);
}

slablist$target:::test_ripple_add_slab
/arg0 == E_TEST_SLAB_EXTREMA/
{
	printf("The nominal extrema of the slab %p don't match what's", arg1);
	printf("in the array\n");
}


slablist$target:::test_ripple_add_slab
/arg0 == E_TEST_SLAB_UNSORTED/
{
	printf("The elems of the slab %p are not sorted\n", arg1);
}

slablist$target:::test_ripple_add_subslab
/arg0 == E_TEST_SUBSLAB_UNSORTED/
{
	printf("The elems of the slab %p are not sorted\n", arg1);
}

slablist$target:::test_ripple_add_slab
/arg0 == E_TEST_RPLA_SLAB_HAS_NOT/
{
	printf("Supposed subslab %p doesn't contain reference to ", arg2);
	printf("slab %p\n", arg1);
}

slablist$target:::test_ripple_add_subslab
/arg0 == E_TEST_RPLA_SUBSLAB_HAS_NOT/
{
	printf("Supposed subslab %p doesn't contain reference to ", arg2);
	printf("subslab %p\n", arg1);
}


slablist$target:::test_ripple_add_slab
/arg0 >= E_TEST_SLAB_IS_NULL/
{
	fail = arg0;
	printf("\nSlab details:\n");
	printf("-------------\n");
	printf("\tmin: %u\n", args[1]->si_min);
	printf("\tmax: %u\n", args[1]->si_max);
	printf("\telems: %u\n", args[1]->si_elems);
	printf("\tnext: %p\n", args[1]->si_next);
	printf("\tprev: %p\n\n", args[1]->si_prev);
	printf("Slab array:\n");
	printf("-----------\n");
	trace(args[1]->si_arr);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::test_ripple_add_subslab
/arg0 >= E_TEST_SUBSLAB_IS_NULL/
{
	fail = arg0;
	printf("\nSlab details:\n");
	printf("-------------\n");
	printf("\tmin: %u\n", args[1]->ssi_min);
	printf("\tmax: %u\n", args[1]->ssi_max);
	printf("\telems: %u\n", args[1]->ssi_elems);
	printf("\tnext: %p\n", args[1]->ssi_next);
	printf("\tprev: %p\n\n", args[1]->ssi_prev);
	printf("Slab array:\n");
	printf("-----------\n");
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

dtrace:::END
/fail/
{
	printf("Failed %d.", fail);
}
