slablist$target:::add_begin
{
	self->elems = args[0]->sli_elems;
}

slablist$target:::link_slab_after
/arg0 > arg1/
{
	printf("%d %lu\n", self->elems, ((uint64_t)arg0 - (uint64_t)arg1));
}

slablist$target:::link_slab_after
/arg0 < arg1/
{
	printf("%d %lu\n", self->elems, ((uint64_t)arg1 - (uint64_t)arg0));
}

slablist$target:::link_slab_before
/arg0 > arg1/
{
	printf("%d %lu\n", self->elems, ((uint64_t)arg0 - (uint64_t)arg1));
}

slablist$target:::link_slab_before
/arg0 < arg1/
{
	printf("%d %lu\n", self->elems, ((uint64_t)arg1 - (uint64_t)arg0));
}

slablist$target:::add_end
{
}
