slablist$target:::test
{

}

slablist$target:::move_mid_to_prev,
slablist$target:::move_next_to_mid,
slablist$target:::reap_begin
{

}

slablist$target:::test_move_prev
/arg0 == 1/
{
	printf("Move prev inconsistent\n");
	self->from = arg4;
	self->prob = arg5;
	trace(self->from);
	trace(self->prob);
	trace(args[1]->si_elems);
	trace(args[1]->si_arr);
	trace(args[2]->si_elems);
	trace(args[2]->si_arr);
	trace(args[3]->si_elems);
	trace(args[3]->si_arr);
	exit(0);
}
