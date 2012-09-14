/*
slablist$target:::add_begin,
slablist$target:::rem_begin
{
	self->name = args[0]->sli_name;
}

slablist$target:::add_end,
slablist$target:::rem_end
{
	@a[self->name, probename, (int)arg0] = count();
}


slablist$target:::sl_inc_elems,
slablist$target:::sl_dec_elems
{
	@b[self->name, probename] = count();
}

slablist$target:::add_replace
{
	@c[self->name, probename, arg3, ustack()] = count();
}
*/

slablist$target:::test_sublayers_have_all_slabs
/arg0 == 1/
{
	printf("FAIL\n");
	exit(0);
}


/*
 * CREATE, DESTROY, ADD, REM.
 */
slablist$target:::create,
slablist$target:::destroy
{
	printf("\n");
}

slablist$target:::add_begin,
slablist$target:::rem_begin
{
	printf("%s: %u %d\n", args[0]->sli_name, arg1, arg2);
}

slablist$target:::add_end,
slablist$target:::rem_end
{
	printf("%d\n", arg1);
}

/*
 * The details of an addition. Which slabs are we adding into? Values of slabs'
 * extrema.
 */
slablist$target:::add_into_*,
slablist$target:::add_before_*,
slablist$target:::add_after_*
{
	self->paddr = args[1]->si_prev;
	self->naddr = args[1]->si_next;
	self->maddr = arg1;
	printf("%x <-> %x <-> %x\n", self->paddr, self->maddr, self->naddr);
}


/*
 * Splits, subalyers, conversions, replacements.
 */
slablist$target:::split_*
{

	printf("\n");
}

slablist$target:::attach_sublayer,
slablist$target:::detach_sublayer
{

	printf("\n");
}

slablist$target:::to_*
{

	printf("\n");
}

slablist$target:::add_replace
{
	printf("\n");
}

slablist$target:::slab_inc_max,
slablist$target:::slab_dec_max
{
	printf("%x[%u]\n", arg0, args[0]->si_max);
}

slablist$target:::slab_inc_min,
slablist$target:::slab_dec_min
{
	printf("%x[%u]\n", arg0, args[0]->si_min);
}

slablist$target:::slab_inc_elems,
slablist$target:::slab_dec_elems
{
	printf("%u\n", args[0]->si_elems);
}

slablist$target:::sl_inc_elems,
slablist$target:::sl_dec_elems
{
	printf("%s(%d) -> %s: %u\n", args[0]->sli_name,
		args[0]->sli_is_sublayer, probename, args[0]->sli_elems);
}

slablist$target:::sl_inc_slabs,
slablist$target:::sl_dec_slabs
{
	printf("%s(%d) -> %s: %u\n", args[0]->sli_name,
		args[0]->sli_is_sublayer, probename, args[0]->sli_slabs);
}

slablist$target:::linear_scan,
slablist$target:::bubble_up
{

	printf("\n");
}

slablist$target:::ripple_add_slab
{
}

slablist$target:::slab_mk,
slablist$target:::slab_rm
{

	printf("\n");
}
