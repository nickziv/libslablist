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

slablist$target:::test_remove_elem
/heap_test && slabs[arg1] == 0/
{
	fail = 1;
	printf("Trying to remove from unallocated memory (%p) as a slab.\n",
		arg1);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::slab_move_mid_to_next,
slablist$target:::slab_move_mid_to_prev,
slablist$target:::slab_move_next_to_mid,
slablist$target:::slab_move_prev_to_mid,
slablist$target:::subslab_move_mid_to_next,
slablist$target:::subslab_move_mid_to_prev,
slablist$target:::subslab_move_next_to_mid,
slablist$target:::subslab_move_prev_to_mid
{
	self->from = arg1;
	self->to = arg2;
	self->context = probename;
}

slablist$target:::test_slab_move_next,
slablist$target:::test_slab_move_prev
/arg0/
{
	self->fail = arg0;
	self->error = e_test_descr[arg0];
	printf("ERROR: %s\n", self->error);
	printf("context: %s\n", self->context);
	printf("scp[from] = scp[%d]\n", arg4);
	printf("sn[i] = sn[%d]\n", arg5);
	printf("\nFrom slab %p, pre-move:\n", arg1);
	printf("-----------------------\n");
	printf("\tmin: %u\n", args[1]->si_min.sle_u);
	printf("\tmax: %u\n", args[1]->si_max.sle_u);
	printf("\telems: %u\n", args[1]->si_elems);
	printf("\tnext: %p\n", args[1]->si_next);
	printf("\tprev: %p\n\n", args[1]->si_prev);
	printf("Arr:\n");
	printf("----\n");
	trace(args[1]->si_arr);
	printf("\nTo slab %p, pre-move:\n", arg3);
	printf("---------------------\n");
	printf("\tmin: %u\n", args[3]->si_min.sle_u);
	printf("\tmax: %u\n", args[3]->si_max.sle_u);
	printf("\telems: %u\n", args[3]->si_elems);
	printf("\tnext: %p\n", args[3]->si_next);
	printf("\tprev: %p\n\n", args[3]->si_prev);
	printf("Arr:\n");
	printf("----\n");
	trace(args[3]->si_arr);
	printf("\nTo slab %p, post-move:\n", arg2);
	printf("----------------------\n");
	printf("\tmin: %u\n", args[2]->si_min.sle_u);
	printf("\tmax: %u\n", args[2]->si_max.sle_u);
	printf("\telems: %u\n", args[2]->si_elems);
	printf("\tnext: %p\n", args[2]->si_next);
	printf("\tprev: %p\n\n", args[2]->si_prev);
	printf("Arr:\n");
	printf("----\n");
	trace(args[2]->si_arr);
	exit(0);
}

slablist$target:::test_subslab_move_next,
slablist$target:::test_subslab_move_prev
/arg0/
{
	self->fail = arg0;
	self->error = e_test_descr[arg0];
	printf("ERROR: %s\n", self->error);
	printf("context: %s\n", self->context);
	printf("scp[from] = scp[%d]\n", arg4);
	printf("sn[i] = sn[%d]\n", arg5);
	printf("\nFrom subslab %p, pre-move:\n", arg1);
	printf("------------------------------\n");
	printf("\tmin: %u\n", args[1]->ssi_min.sle_u);
	printf("\tmax: %u\n", args[1]->ssi_max.sle_u);
	printf("\telems: %u\n", args[1]->ssi_elems);
	printf("\tnext: %p\n", args[1]->ssi_next);
	printf("\tprev: %p\n\n", args[1]->ssi_prev);
	printf("Arr: %p\n", args[1]->ssi_arr);
	printf("----\n");
	trace(subarrinfo[args[1]->ssi_arr]->sai_data);
	printf("\nTo subslab %p, pre-move:\n", arg3);
	printf("----------------------------\n");
	printf("\tmin: %u\n", args[3]->ssi_min.sle_u);
	printf("\tmax: %u\n", args[3]->ssi_max.sle_u);
	printf("\telems: %u\n", args[3]->ssi_elems);
	printf("\tnext: %p\n", args[3]->ssi_next);
	printf("\tprev: %p\n\n", args[3]->ssi_prev);
	printf("Arr: %p\n", args[3]->ssi_arr);
	printf("----\n");
	trace(subarrinfo[args[3]->ssi_arr]->sai_data);
	printf("\nTo subslab %p, post-move:\n", arg2);
	printf("-----------------------------\n");
	printf("\tmin: %u\n", args[2]->ssi_min.sle_u);
	printf("\tmax: %u\n", args[2]->ssi_max.sle_u);
	printf("\telems: %u\n", args[2]->ssi_elems);
	printf("\tnext: %p\n", args[2]->ssi_next);
	printf("\tprev: %p\n\n", args[2]->ssi_prev);
	printf("Arr: %p\n", args[2]->ssi_arr);
	printf("----\n");
	trace(subarrinfo[args[2]->ssi_arr]->sai_data);
	exit(0);
}



slablist$target:::test_remove_elem
/heap_test && slabs[arg1] == 2/
{
	fail = 1;
	printf("Trying to remove from a freed slab.\n");
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

slablist$target:::test_remove_elem
/arg0 == 1/
{
	fail = arg0;
	printf("Slab provided to remove_elem() is NULL.\n");
	exit(0);
}

slablist$target:::test_remove_elem,
slablist$target:::test_remove_slab
/arg0/
{
	printf("ERROR: %d  %s\n", arg0, e_test_descr[arg0]);
	printf("SLAB[i]: %p[%d]\n", arg1, arg2);
}

ssbcinfo_t bblup_ssbcs[int];
sbcinfo_t bblup_sbc;

slablist$target:::get_extreme_path
/arg0 != NULL/
{
	bblup_ssbcs[arg1].ssbci_subslab = args[0]->ssbci_subslab;
	bblup_ssbcs[arg1].ssbci_on_edge = args[0]->ssbci_on_edge;
	self->layers = arg2;
}

slablist$target:::get_extreme_path
/arg1 != NULL/
{
	bblup_sbc.sbci_slab = args[1]->sbci_slab;
	bblup_sbc.sbci_on_edge = args[1]->sbci_on_edge;
}

slablist$target:::test_remove_slab
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

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 7/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 6/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 5/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 4/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 3/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 2/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 1/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

slablist$target:::test_remove_slab
/(arg0 == E_TEST_SUBSLAB_MIN || arg0 == E_TEST_SUBSLAB_MAX) && self->layers == 0/
{
        self->ss = bblup_ssbcs[self->layers].ssbci_subslab;
        printf("\tsubslab: %p\n", self->ss);
        printf("\t\tmax: %u\n", subslabinfo[self->ss]->ssi_max.sle_u);
        printf("\t\tmin: %u\n", subslabinfo[self->ss]->ssi_min.sle_u);
        self->layers--;
}

/* OLD */

slablist$target:::test_remove_elem,
slablist$target:::test_remove_slab
/arg0 == E_TEST_SLAB_NULL || arg0 == E_TEST_SUBSLAB_NULL/
{
	fail = arg0;
	exit(0);
}

slablist$target:::test_remove_elem
/arg0 != 0 && arg0 != E_TEST_SLAB_NULL/
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
	printf("\tremove_elem(%p, %lu)\n", arg1, (uint64_t)arg2);
	exit(0);
}

slablist$target:::test_remove_slab
/arg0 != 0 && arg0 != E_TEST_SUBSLAB_NULL &&
 arg0 != E_TEST_SUBSLAB_SUBARR_NULL/
{
	fail = arg0;
	printf("\nSubslab details:\n");
	printf("-------------\n");
	printf("\tmax: %u\n", args[1]->ssi_max.sle_u);
	printf("\tmin: %u\n", args[1]->ssi_min.sle_u);
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
	printf("\nFunction Arguments:\n");
	printf("-------------------\n");
	printf("\tremove_slab(%p, %lu)\n", arg1, (uint64_t)arg2);
	exit(0);
}


dtrace:::END
/fail == 0/
{
}
