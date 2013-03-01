#pragma D option quiet

inline int MID_TO_NEXT =  0;
inline int MID_TO_PREV =  1;
inline int NEXT_TO_MID = 2;
inline int NEXT_TO_PREV = 3;
inline int PREV_TO_MID = 4;
inline int PREV_TO_NEXT = 5;

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
	self->layers = arg1;
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

slablist$target:::move_mid_to_next,
slablist$target:::move_mid_to_prev,
slablist$target:::move_next_to_prev,
slablist$target:::move_next_to_mid,
slablist$target:::move_prev_to_mid,
slablist$target:::move_prev_to_next
{
	self->from = arg1;
	self->to = arg2;
}

slablist$target:::move_mid_to_next
{
	self->move = MID_TO_NEXT;
}

slablist$target:::move_mid_to_prev
{
	self->move = MID_TO_PREV;
}

slablist$target:::move_next_to_mid
{
	self->move = NEXT_TO_MID;
}

slablist$target:::move_next_to_prev
{
	self->move = NEXT_TO_PREV;
}

slablist$target:::move_prev_to_next
{
	self->move = PREV_TO_NEXT;
}

slablist$target:::move_prev_to_mid
{
	self->move = PREV_TO_MID;
}


/* mid to next and mid to prev */
slablist$target:::test_move_next
/arg0 && self->move == MID_TO_NEXT/
{
	printf("Error in move_to_next() when trying to move elems from mid ");
	printf("(%p) to next (%p)\n", self->from, self->to);
}

slablist$target:::test_move_prev
/arg0 && self->move == MID_TO_PREV/
{
	printf("Error in move_to_prev() when trying to move elems from mid ");
	printf("(%p) to prev (%p)\n", self->from, self->to);
}

/* prev to mid and prev to max */
slablist$target:::test_move_next
/arg0 && self->move == PREV_TO_MID/
{
	printf("Error in move_to_next() when trying to move elems from prev ");
	printf("(%p) to mid (%p)\n", self->from, self->to);
}

slablist$target:::test_move_next
/arg0 && self->move == PREV_TO_NEXT/
{
	printf("Error in move_to_next() when trying to move elems from prev ");
	printf("(%p) to mid (%p)\n", self->from, self->to);
}

/* next to mid and next to prev */
slablist$target:::test_move_prev
/arg0 && self->move == NEXT_TO_MID/
{
	printf("Error in move_to_prev() when trying to move elems from next ");
	printf("(%p) to mid (%p)\n", self->from, self->to);
}

slablist$target:::test_move_prev
/arg0 && self->move == NEXT_TO_PREV/
{
	printf("Error in move_to_prev() when trying to move elems from next ");
	printf("(%p) to prev (%p)\n", self->from, self->to);
}


slablist$target:::test_move_next,
slablist$target:::test_move_prev
/arg0 == 1/
{
	printf("Inconsistency between pre-move version of %p and", self->from);
	printf(" current version of %p\n", self->to);
	printf("We started copying from index %d, and have a problem", arg4);
	printf(" at index %d in %p\n\n", arg5, self->to);
	printf("Slab %p pre-move:\n", self->from);
	printf("-----------------\n");
	printf("\tmin: %u\n", args[1]->si_min);
	printf("\tmax: %u\n", args[1]->si_max);
	printf("\telems: %u\n", args[1]->si_elems);
	printf("\tnext: %p\n", args[1]->si_next);
	printf("\tprev: %p\n\n", args[1]->si_prev);
	printf("Slab array:\n");
	printf("-----------\n\n");
	trace(args[1]->si_arr);
	printf("Slab %p post-move:\n", self->to);
	printf("-----------------\n");
	printf("\tmin: %u\n", args[2]->si_min);
	printf("\tmax: %u\n", args[2]->si_max);
	printf("\telems: %u\n", args[2]->si_elems);
	printf("\tnext: %p\n", args[2]->si_next);
	printf("\tprev: %p\n\n", args[2]->si_prev);
	printf("Slab array:\n");
	printf("-----------\n\n");
	trace(args[2]->si_arr);
}

slablist$target:::test_move_next,
slablist$target:::test_move_prev
/arg0 == 2/
{
	printf("Inconsistency between pre-move version of %p and", self->to);
	printf(" current version of %p\n", self->to);
	printf("We started copying from index %d, and have a problem", arg4);
	printf(" at index %d in %p\n\n", arg5, self->to);
	printf("Slab %p pre-move:\n", self->to);
	printf("-----------------\n");
	printf("\tmin: %u\n", args[3]->si_min);
	printf("\tmax: %u\n", args[3]->si_max);
	printf("\telems: %u\n", args[3]->si_elems);
	printf("\tnext: %p\n", args[3]->si_next);
	printf("\tprev: %p\n\n", args[3]->si_prev);
	printf("Slab array:\n");
	printf("-----------\n\n");
	trace(args[3]->si_arr);
	printf("Slab %p post-move:\n", self->to);
	printf("------------------\n");
	printf("\tmin: %u\n", args[2]->si_min);
	printf("\tmax: %u\n", args[2]->si_max);
	printf("\telems: %u\n", args[2]->si_elems);
	printf("\tnext: %p\n", args[2]->si_next);
	printf("\tprev: %p\n\n", args[2]->si_prev);
	printf("Slab array:\n");
	printf("-----------\n\n");
	trace(args[2]->si_arr);
}

slablist$target:::test_remove_elem
/slabs[arg1] == 2/
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

slablist$target:::test_remove_elem
/arg0 == 2/
{
	printf("Slablist backpointer for %p is NULL.\n", arg1);
}

slablist$target:::test_remove_elem
/arg0 == 3/
{
	printf("The nominal extrema of the slab %p don't match what's", arg1); 
	printf("in the array\n");
}

slablist$target:::test_remove_elem
/arg0 == 4/
{
	printf("The nominal min of the subslab %p doesn't match", arg1);
	printf(" what's in a first superslab's array\n\n");
}

slablist$target:::test_remove_elem
/arg0 == 5/
{
	printf("The nominal max of the subslab %p doesn't match", arg1);
	printf(" what's in a last superslab's array\n\n");
}

slablist$target:::test_remove_elem
/arg0 == 4 || arg0 == 5/
{
	printf("Breadcrumb Path Details:\n");
	printf("------------------------\n");
	printf("baselayer is at the bottom.\n");
	printf("'slab: 0' means that we have no slab at that level.\n\n");
}

slablist$target:::test_remove_elem
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

slablist$target:::test_remove_elem
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


slablist$target:::test_remove_elem
/arg0 == 6/
{
	printf("The elems of the slab %p are not sorted\n", arg1);
}

slablist$target:::test_remove_elem
/arg0 == 7/
{
	printf("We are trying to remove from empty slab %p\n", arg1);
}

slablist$target:::test_remove_elem
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
