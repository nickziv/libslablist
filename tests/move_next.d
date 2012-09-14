slablist$target:::test
{

}

slablist$target:::move_mid_to_next
{

}

slablist$target:::test_move_next
/arg0 == 1/
{
	/* do stuff */
	printf("Move next inconsistent\n");
	self->from = arg4;
	self->prob = arg5;
	trace(args[1]->si_elems);
	trace(self->from);
	trace(self->prob);
	trace(args[1]->si_arr);
	trace(args[2]->si_elems);
	trace(args[2]->si_arr);
	trace(args[3]->si_elems);
	trace(args[3]->si_arr);
	exit(0);
}
