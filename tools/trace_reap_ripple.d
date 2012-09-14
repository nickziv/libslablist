pid$target::sublayer_slab_ptr_srch:return
{
	trace(arg1);
}

slablist$target:::ripple_rem_slab
{
	self->rip = 1;
	trace(arg1);
}

slablist$target:::bwdshift_begin
/self->rip/
{
	/* trace slab here */
	trace(arg2);
	trace(args[1]->si_elems);
	trace(args[1]->si_arr);
}

slablist$target:::bwdshift_end
/self->rip/
{
	self->rip = 0;
}

pid$target::mk_slab:return
{
	trace(arg1);
}

pid$target::rm_slab:entry
{
	trace(arg0);
}

pid$target::is_elem_in_range:entry
{
	trace(arg0);
	trace(arg1);
} 
