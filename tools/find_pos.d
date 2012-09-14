slablist$target:::find_slab_pos_begin
{
	self->ts = vtimestamp;
	self->ul = arg0;
}

slablist$target:::find_slab_pos_end
{
	@[self->ul, arg0] = quantize(vtimestamp - self->ts);
}
