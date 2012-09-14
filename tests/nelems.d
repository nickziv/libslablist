slablist$target:::test_nelems
/arg0 == 0/
{
	self->pass = arg0;
	self->pass_elems = arg1;
	self->pass_selems = arg2;
}

slablist$target:::test_nelems
/self->pass == 0 && arg0 == 1/
{
	self->pass = 1;
	printf("good nums: %d, %d\n", self->pass_elems, self->pass_selems);
}

slablist$target:::test_nelems
/self->pass == 1 && arg0 == 1/
{
	@s[probename, ustack(), timestamp, arg0, arg1, arg2] = count();
}

slablist$target:::split_*
{
	trace(arg0); trace(arg1);
}

pid$target::percolate_rem_to_sublayers:entry
{

}

pid$target::slablist_rem:entry
{

}

