slablist$target:::sl_inc_slabs
/args[0]->sli_is_sublayer == 0/
{
	self->follow = 1;
	self->nslabs = args[0]->sli_slabs;
	self->nslabs -= 1;
}

slablist$target:::sl_inc_elems
/self->follow == 1 && args[0]->sli_is_sublayer == 0 && self->nslabs != 0/
{
	self->min = args[0]->sli_elems;
	@i[self->nslabs, self->min] = count();
	self->follow = 0;
}
