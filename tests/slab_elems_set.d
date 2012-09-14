/*
 * This test makes sure that each time we remove an elem or add an
 * elem, we increase or decrease the number of elems in a slab.
 */

slablist$target:::slab_inc_elems
{
	/*@r[probename, args[0]->si_elems] = count();*/
	@r[probename, ustack()] = count();
}

slablist$target:::slab_dec_elems
{
	/*@R[probename, args[0]->si_elems] = count();*/
	@R[probename,ustack()] = count();
}
