dtrace:::BEGIN
{
	elems = 0;
}

uulist$target:::add_begin
{
	self->ts = vtimestamp;
	elems += 1;
}

uulist$target:::add_end
{
	self->ets = vtimestamp;
	printf("%lu %lu\n", self->ets, self->ets - self->ts);
}
