/*
slablist$target:::add_begin,
slablist$target:::rem_begin
{
	self->name = (args[0]->sli_name);
	self->sml = (args[0]->sli_is_small_list);
	self->pn = probename;
}
*/

/*
slablist$target::small_list_add:got_here
/self->name == "intlistsrt"/
{
	trace(arg0);
}
*/

/*
pid$target::cmpfun:entry,
pid$target::cmpfun_str:entry
{
	@[self->name, self->sml, self->pn, probefunc] = count();
}
*/

slablist$target:::add_begin
{
	self->ts = timestamp;
	self->elems = args[0]->sli_slabs;
	self->name = args[0]->sli_name;
	self->type = args[0]->sli_is_small_list;
}

slablist$target:::add_end
{
	self->ets = timestamp;
	@avgt["avg", self->name, self->type] = avg(self->ets - self->ts);
	@qtzt["qtz", self->name, self->type] = quantize(self->ets - self->ts);
}
