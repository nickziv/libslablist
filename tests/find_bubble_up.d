#pragma D option quiet

dtrace:::BEGIN
{
	fail = 0;
	/*
	 * Change this to 1 to enable heap-testing. Heap testing is great for
	 * small volumes of data, relative to the amount of RAM you have
	 * available. Once you get to large volumes, these tests will start
	 * dropping variables, and will no longer be reliable. The only way to
	 * prevent this from happening is to get more RAM.
	 */
	heap_test = 0;
}

ssbcinfo_t bblup_ssbcs[int];
sbcinfo_t bblup_sbc;

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
/arg0 != NULL/
{
	bblup_ssbcs[arg2].ssbci_subslab = args[0]->ssbci_subslab;
	bblup_ssbcs[arg2].ssbci_on_edge = args[0]->ssbci_on_edge;
	self->layers = arg2;
}

slablist$target:::get_extreme_path
/arg1 != NULL/
{
	bblup_sbc.sbci_slab = args[1]->sbci_slab;
	bblup_sbc.sbci_on_edge = args[1]->sbci_on_edge;
}


slablist$target:::test_find_bubble_up
/arg0 != 0/
{
	printf("ERROR: %d  %s\n", arg0, e_test_descr[arg0]);
}

slablist$target:::test_find_bubble_up
/arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX/
{
	printf("Breadcrumb Path Details:\n");
	printf("------------------------\n");
	printf("baselayer is at the bottom.\n");
	self->s = bblup_sbc.sbci_slab;
	printf("\tslab: %p\n", self->s);
	printf("\t\tmax: %u\n", slabinfo[self->s]->si_max.sle_u);
	printf("\t\tmin: %u\n", slabinfo[self->s]->si_min.sle_u);
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 7/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 6/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 5/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 4/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 3/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 2/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 1/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 0/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_find_bubble_up
/arg0 != 0 && arg0 != E_TEST_SLAB_NULL && arg1 != NULL/
{
	fail = arg0;
	printf("\nSlab details:\n");
	printf("-------------\n");
	printf("\tmax: %u\n", args[1]->si_max.sle_u);
	printf("\tmin: %u\n", args[1]->si_min.sle_u);
	printf("\telems: %u\n", args[1]->si_elems);
	printf("\tnext: %p\n", args[1]->si_next);
	printf("\tprev: %p\n\n", args[1]->si_prev);
	printf("Slab array:\n");
	printf("-----------\n");
	trace(args[1]->si_arr);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	printf("\nFunction Arguments:\n");
	printf("-------------------\n");
	printf("\tinsert_elem(%p, %lu, %lu)\n", arg1, (uintptr_t)arg2,
			(uint64_t)arg3);
	exit(0);
}

slablist$target:::test_find_bubble_up
/arg0 != 0 && arg0 != E_TEST_SUBSLAB_NULL &&
 arg0 != E_TEST_SUBSLAB_SUBARR_NULL && arg2 != NULL/
{
	fail = arg0;
	printf("\nSubslab details:\n");
	printf("-------------\n");
	printf("\tmax: %u\n", args[2]->ssi_max.sle_u);
	printf("\tmin: %u\n", args[2]->ssi_min.sle_u);
	printf("\telems: %u\n", args[2]->ssi_elems);
	printf("\tnext: %p\n", args[2]->ssi_next);
	printf("\tprev: %p\n\n", args[2]->ssi_prev);
	printf("Subslab array:\n");
	printf("--------------\n");
	self->sa = args[2]->ssi_arr;
	trace(subarrinfo[self->sa]->sai_data);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	printf("\nFunction Arguments:\n");
	printf("-------------------\n");
	printf("\tinsert_slab(%p, %p, %p, %lu)\n", arg1, arg2, arg3,
			(uint64_t)arg4);
	exit(0);
}

dtrace:::END
/fail == 0/
{
}
