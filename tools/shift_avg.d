slablist$target:::fwdshift_begin
{
	self->fs = 1;
	self->fts = timestamp;
	self->flist = args[0]->sli_name;
	self->fsz = args[1]->si_elems - arg2;
}

slablist$target:::fwdshift_end
/self->fs == 1/
{
	self->fs = 0;
	@[self->flist, probename, self->fsz] = avg(timestamp - self->fts)
}

slablist$target:::bwdshift_begin
{
	self->bs = 1;
	self->bts = timestamp;
	self->blist = args[0]->sli_name;
	self->bsz = args[1]->si_elems - arg2;
}

slablist$target:::bwdshift_end
/self->bs == 1/
{
	self->bs = 0;
	@[self->blist, probename, self->bsz] = avg(timestamp - self->bts)
}
