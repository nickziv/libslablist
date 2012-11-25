slablist$target:::add_begin
{
	self->ts = vtimestamp;
	self->elems = args[0]->sli_elems;
}

pid$target::try_reap_all:entry
{
	self->rts = vtimestamp;
}

pid$target::try_reap_all:return
{
	self->rets = vtimestamp - self->rts;
}

pid$target::mk_slab:entry
{
	self->mts = vtimestamp;
}

pid$target::mk_slab:return
{
	self->mets = vtimestamp - self->mts;
}

slablist$target:::add_end
{
	self->ets = vtimestamp - self->ts;
	printf("%d %d %d %d\n", self->elems, self->ets, self->rets, self->mets);
	self->mets = 0;
	self->rets = 0;
}
