pid$target::mk_slab:entry
{
	self->ts = timestamp;
}

pid$target::mk_slab:return
{
	@[probefunc] = quantize(timestamp - self->ts);
}
