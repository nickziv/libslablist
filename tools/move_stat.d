pid$target::mk_slab:return
{
	slabs[arg1] = 1;
}

pid$target::rm_slab:entry
{
	slabs[arg0] = 2;
}

slablist$target:::move_mid_to_next,
slablist$target:::move_prev_to_next
{
	@m[probename, args[0]->sli_layer, args[1]->si_elems, args[2]->si_elems]
		= count();
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
