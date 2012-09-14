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

pid$target::remove_elem:entry
/slabs[arg1] == 0/
{
	printf("Trying to remove %d from uninitialized memory %u\n", arg0,
		arg1);
	@[ustack()] = count();
	exit(0);
}

pid$target::remove_elem:entry
/slabs[arg1] == 2/
{
	printf("Trying to remove %d from freed slab %u\n", arg0,
		arg1);
	@[ustack()] = count();
	exit(0);
}
