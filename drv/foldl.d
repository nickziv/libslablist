#pragma D option quiet

struc$target:::foldl_begin
{
	self->ts = timestamp;
}

struc$target:::foldl_end
{
	printf("%u\n", timestamp - self->ts);
}
