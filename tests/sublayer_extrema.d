slablist$target:::test
{

}

slablist$target:::add_begin,
slablist$target:::rem_begin
{
	@[probename] = count();
}

/*
pid$target::mk_slab:return
{
	trace(arg1);
}

pid$target::rm_slab:entry
{
	trace(arg0);
}

pid$target::ripple_*:entry
{

}


slablist$target:::slab_inc_*,
slablist$target:::slab_dec_*
{
	trace(arg0);
	trace(args[0]->si_elems);
}

slablist$target:::slab_set_max
{
	trace(arg0);
	trace(args[0]->si_max);
	self->elems = args[0]->si_elems;
	self->max = args[0]->si_arr[(self->elems - 1)];
	trace(self->max);
}

slablist$target:::slab_set_min
{
	trace(arg0);
	trace(args[0]->si_min);
	self->min = args[0]->si_arr[0];
	trace(self->min);
}


slablist$target:::move_*
{
	trace(arg1);
	trace(args[1]->si_elems);
	*trace(args[1]->si_arr);*
	trace(arg2);
	trace(args[2]->si_elems);
	*trace(args[2]->si_arr);*
}

slablist$target:::attach_sublayer,
slablist$target:::detach_sublayer
{
	
}

slablist$target:::set_crumb
{
	trace(arg1);
	trace(arg2);
	trace(args[1]->si_elems);
}
*/

slablist$target:::test_sublayer_extrema
/arg0 == 1/
{
	trace(arg1);
	trace(arg2);
	trace(arg3);
	trace(arg4);
	ustack();
	exit(0);
}
