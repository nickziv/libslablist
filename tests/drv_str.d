dtrace:::BEGIN
{
	dict[0] = 0;
}

pid$target::mk_str:entry
{
	self->follow = 1;
}

pid$target::umem_zalloc:entry
/self->follow == 1/
{
	self->sz = arg0;
}

pid$target::umem_zalloc:return
/self->follow == 1/
{
	self->id = arg1;
}

pid$target::mk_str:return
{
	dict[self->id] = self->sz;
	self->follow = 0;
}

pid$target::rm_str:entry
{
	self->follow = 1;
}

pid$target::umem_free:entry
/self->follow == 1/
{
	self->fid = arg0;
	self->fsz = arg1;
	self->sz = dict[arg0]
}

pid$target::rm_str:return
/self->sz != 0 && self->sz != self->fsz/
{
	printf("No match for %d, alloc sz %d, free sz %d\n", self->fid, self->sz, self->fsz);
}
