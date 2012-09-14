slablist$target:::add_begin
{
	printf("add: %lu\n", (uint64_t)arg1);
}

slablist$target:::add_end
{

}

slablist$target:::add_*
{
	printf("slab: %p, ", arg1);
	printf("elem: %lx\n", arg2);
	/*trace(args[1]->si_arr);*/
}

slablist$target:::reap_begin
{

}

slablist$target:::reap_end
{

}

slablist$target:::ripple_add_slab
{
	trace(arg1);
}

slablist$target:::ripple_rem_slab
{
	trace(arg1);
}

slablist$target::slab_gen_insert:got_here
{

}

slablist$target:::linear_scan
{
	printf("slab: %p\n", arg1);
}

slablist$target:::subslab_bin_srch
{
	printf("subslab: %p, ", arg0);
	printf("slab: %p, ", arg1);
	printf("min: %lu, ", args[1]->si_min);
	printf("max: %lu\n", args[1]->si_max);
}

slablist$target:::slab_bin_srch
{
	printf("subslab: %p, ", arg0);
	printf("mid: %lu\n", (uint64_t)arg1);
}

slablist$target:::bubble_up
{
	printf("slab: %p\n", arg1);
}

pid$target::mk_slab:return
{
	printf("slab: %p\n", arg1);
}

pid$target::rm_slab:entry
{
	printf("slab: %p\n", arg0);
}

slablist$target:::test_slabs_sorted
/arg0 == 1/
{
	printf("Yeah, we failed\n");
	exit(0);
}
