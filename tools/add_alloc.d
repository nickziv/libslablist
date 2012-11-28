slablist$target:::add_begin
{
	self->ts = vtimestamp;
	self->elems = args[0]->sli_elems;
}

pid$target::mk_slab:return
{
	self->retd = (uint64_t)arg1;
}

slablist$target:::add_end
{
	printf("%d %lu\n", self->elems, self->retd);
}
