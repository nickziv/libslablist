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
	@avgt["avg", self->name, self->type] = avg(self->ets - self->ts);
	@qtzt["qtz", self->name, self->type] = quantize(self->ets - self->ts);
}
