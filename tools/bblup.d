slablist$target:::bblup_loop
{
	trace(arg0);
}

slablist$target:::bblup_bc
{
	tracemem(copyin(arg0, 256*8), 256*8);
}
