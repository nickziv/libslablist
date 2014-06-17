/*
 * We want to measure the global add-performance, so we don't collect metadata.
 * This gives us non-segregated output, and reduces the overhead of copying the
 * names, number of elements, and type of the list.
 */
struc$target:::rem_begin
{
	self->ts = timestamp;
}

struc$target:::rem_end
{
	self->tsdif = timestamp - self->ts;
	@avgt["avg"] = avg(self->tsdif);
	@qtzt["qtz"] = quantize(self->tsdif);
}

dtrace:::END
{
	printf("BREAK\n");
}
