#pragma D option quiet

dtrace:::BEGIN
{
	inter = 0;
	prev = 0;
	e = 0;
}

slablist$target:::add_end
{
	/*e = args[0]->sli_elems;
	*/
	e += 1;
}

slablist$target:::rem_begin
{
	exit(0);
}

tick-1ms
{
	inter += 1;
	printf("%d\t%d\n", inter, e);
}
