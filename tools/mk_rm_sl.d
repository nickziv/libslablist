pid$target::mk_slablist:return
{
	@[arg1, probename, ustack()] = count();
}

pid$target::rm_slablist:entry
{
	@[arg0, probename, ustack()] = count();
}
