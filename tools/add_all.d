/*
 * We want to measure the global add-performance, so we don't collect metadata.
 * This gives us non-segregated output, and reduces the overhead of copying the
 * names, number of elements, and type of the list.
 */
slablist$target:::add_begin
{
	self->ts = timestamp;
}

slablist$target:::add_end
{
	self->tsdif = timestamp - self->ts;
	@avgt["avg"] = avg(self->tsdif);
	@qtzt["qtz"] = quantize(self->tsdif);
}
