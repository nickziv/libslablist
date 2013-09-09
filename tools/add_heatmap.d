slablist$target:::add_begin
{
	self->ts = vtimestamp;
	self->elems = args[0]->sli_elems;
	self->name = args[0]->sli_name;
	self->type = args[0]->sli_is_small_list;
}

slablist$target:::add_end
{
	self->ets = vtimestamp;
	printf("%lu %lu\n", self->ets, self->ets - self->ts);
}
