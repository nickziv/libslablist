#pragma D option quiet

struc$target:::foldr_begin
{
	self->ts = timestamp;
}

struc$target:::foldr_end
{
	printf("%u\n", timestamp - self->ts);
}
