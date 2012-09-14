pid$target::mk_slab:return
{
	slabs[arg1] = 1;
}

pid$target::rm_slab:entry
{
	slabs[arg0] = 0;
}

slablist$target::move_to_next:got_here
/slabs[arg0] == 0 && arg0 != 0/
{
	printf("Slab %p was previously freed\n", arg0);
	exit(0);
}
