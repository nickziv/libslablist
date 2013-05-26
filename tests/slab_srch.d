#pragma D option quiet

dtrace:::BEGIN
{
	fail = 0;
	/* see comment in insert.d */
	heap_test = 0;
}

bcinfo_t bblup_bcs[int];

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

slablist$target:::get_extreme_path
{
	bblup_bcs[arg1].bci_slab = args[0]->bci_slab;
	bblup_bcs[arg1].bci_on_edge = args[0]->bci_on_edge;
	self->layers = arg2;
}

slablist$target:::test_slab_srch
/heap_test && slabs[arg1] == 0/
{
	fail = 1;
	printf("Trying to search unallocated memory (%p) as a slab.\n", arg1);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::test_slab_srch
/heap_test && slabs[arg1] == 2/
{
	fail = 1;
	printf("Trying to search a freed slab.\n");
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::test_slab_srch
/arg0 == 1/
{
	fail = arg0;
	printf("Slab provided to insert_elem() is NULL.\n");
	exit(0);
}

slablist$target:::test_slab_srch
/arg0 == 2/
{
	printf("Slablist backpointer for %p is NULL.\n", arg1);
}

slablist$target:::test_slab_srch
/arg0 == 3/
{
	printf("The nominal extrema of the slab %p don't match what's", arg1); 
	printf("in the array\n");
}

slablist$target:::test_slab_srch
/arg0 == 4/
{
	printf("The nominal min of the subslab %p doesn't match", arg1);
	printf(" what's in the first topslab's array\n\n");
}

slablist$target:::test_slab_srch
/arg0 == 5/
{
	printf("The nominal max of the subslab %p doesn't match", arg1);
	printf(" what's in the last topslab's array\n\n");
}

slablist$target:::test_slab_srch
/arg0 == 4 || arg0 == 5/
{
	printf("Breadcrumb Path Details:\n");
	printf("------------------------\n");
	printf("baselayer is at the bottom.\n");
	printf("'slab: 0' means that we have no slab at that level.\n\n");
}

slablist$target:::test_slab_srch
/arg0 == 4 || arg0 == 5 && self->layers >= 5/
{
	self->s = bblup_bcs[8].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
	self->s = bblup_bcs[7].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
	self->s = bblup_bcs[6].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
	self->s = bblup_bcs[5].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
}

slablist$target:::test_slab_srch
/arg0 == 4 || arg0 == 5 && self->layers <= 4/
{
	self->s = bblup_bcs[4].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
	self->s = bblup_bcs[3].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
	self->s = bblup_bcs[2].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
	self->s = bblup_bcs[1].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);
	self->s = bblup_bcs[0].bci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", self->s != NULL ? slabinfo[self->s]->si_max : 0);
	printf("\t\tmin: %u\n", self->s != NULL ? slabinfo[self->s]->si_min : 0);

}


slablist$target:::test_slab_srch
/arg0 == 6/
{
	printf("The elems of the slab %p are not sorted\n", arg1);
}

slablist$target:::test_slab_srch
/arg0 == 7/
{
	printf("Binary search and linear search for %d give two", arg3);
	printf(" different indexes in slab %p", arg1);
}

slablist$target:::test_slab_srch
/arg0 >= 2/
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

dtrace:::END
/fail == 0/
{
	printf("All tests passed.");
}
