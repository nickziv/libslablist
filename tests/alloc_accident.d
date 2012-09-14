pid$target::mk_buf:return
{
	buf[arg1] = 1;
}

pid$target::rm_buf:entry
/buf[arg0] == 0/
{
	printf("%s freed %p by accident\n", probefunc, arg0);
	ustack();
}

pid$target::mk_slab:return
{
	slab[arg1] = 1;
}

pid$target::rm_slab:entry
/slab[arg0] == 0/
{
	printf("%s freed %p by accident\n", probefunc, arg0);
	ustack();
}


pid$target::mk_slablist:return
{
	slablist[arg1] = 1;
}

pid$target::rm_slablist:entry
/slablist[arg0] == 0/
{
	printf("%s freed %p by accident\n", probefunc, arg0);
	ustack();
}

pid$target::mk_sml_node:return
{
	sml[arg1] = 1;
}

pid$target::rm_sml_node:entry
/sml[arg0] == 0/
{
	printf("%s freed %p by accident\n", probefunc, arg0);
	ustack();
}
