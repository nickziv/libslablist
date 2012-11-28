slablist$target:::add_begin
{
	self->ts = vtimestamp;
	self->elems = args[0]->sli_elems;
}

slablist$target:::slab_bin_srch
{
	self->target = (uint64_t)arg0;
	printf("%d %lu 0\n", self->elems, self->target);
}

slablist$target:::subslab_bin_srch
{
	self->starget = (uint64_t)arg0;
	printf("%d 0 %lu\n", self->elems, self->starget);
}

slablist$target:::add_end
{
}
