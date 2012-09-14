pid$target::mk_slab:return
{
	slabs[arg1] = 1;
}

pid$target::rm_slab:entry
{
	slabs[arg0] = 2;
}

slablist$target:::ripple_add_slab
/slabs[arg1] == 0/
{
	printf("rippled add of %d; it is uninitialized\n", arg1);
	@[ustack()] = count();
	exit(0);
}

slablist$target:::ripple_add_slab
/slabs[arg1] == 2/
{
	printf("rippled add of %d; it is freed\n", arg1);
	@s[ustack()] = count();
	exit(0);
}
