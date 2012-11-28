slablist$target:::add_begin
{
	self->ts = vtimestamp;
	self->elems = args[0]->sli_elems;
}

slablist$target:::bubble_up_begin
{
	self->mts = vtimestamp;
}

slablist$target:::bubble_up_end
{
	self->mets = vtimestamp - self->mts;
}

pid$target::mk_bc:entry
{
	self->rts = vtimestamp;
}

pid$target::mk_bc:return
{
	self->rets = vtimestamp - self->rts;
}

pid$target::mk_slab:entry
{
	self->sts = vtimestamp;
}

pid$target::mk_slab:return
{
	self->sets = vtimestamp - self->sts;
}

slablist$target:::add_end
{
	self->ets = vtimestamp - self->ts;
	printf("%d %d %d %d %d\n", self->elems, self->ets, self->mets, self->rets, self->sets);
	self->mets = 0;
	self->rets = 0;
	self->sets = 0;
}
