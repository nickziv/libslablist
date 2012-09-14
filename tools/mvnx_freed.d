slablist$target:::add_begin
{
	elems[arg1] = 1;
}

pid$target::mk_slab:return
{
	slabs[arg1] = 1;
}

pid$target::rm_slab:entry
{
	slabs[arg0] = 2;
}

pid$target::ripple_rem_to_sublayers:entry
{
	printf("rippled\n");
}

slablist$target:::ripple_rem_slab
{
	printf("rippled rem of %d\n", arg1);
}

slablist$target:::move_mid_to_next,
slablist$target:::move_prev_to_next
{
	printf("slab0->elems = %d\n", args[1]->si_elems);
	printf("slab1->elems = %d\n", args[2]->si_elems);
}


slablist$target::move_to_next:got_here
/elems[arg0] == 1/
{
	printf("Elem in subslab, not slab-ptr\n");
}

slablist$target::move_to_next:got_here
/slabs[arg0] == 0/
{
	printf("Trying to move from uninitialized memory %u\n", arg0);
	@[ustack()] = count();
	exit(0);
}

slablist$target::move_to_next:got_here
/slabs[arg1] == 2/
{
	printf("Trying to move from freed slab %u\n", arg0);
	@[ustack()] = count();
	exit(0);
}
