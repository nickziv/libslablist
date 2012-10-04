slablist$target:::bubble_up_begin
{
	self->ts = timestamp;
	self->follow = 1;
	self->ct = 0;
}

slablist$target:::bubble_up_end
{
	self->tsdif = timestamp - self->ts;
	self->follow = 0;
	@[self->ct] = avg(self->tsdif);
}
