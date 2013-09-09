dtrace:::BEGIN
{
	elems = 0;
}

struc$target:::add_begin
{
	self->ts = vtimestamp;
	elems += 1;
}

struc$target:::add_end
{
	self->ets = vtimestamp;
	printf("%d %lu\n", elems, self->ets - self->ts);
}
