#pragma D option quiet

dtrace:::BEGIN
{
	inter = 0;
	prev = 0;
}

slablist$target:::add_begin
{
	e = args[0]->sli_elems;
}

slablist$target:::rem_begin
{
	exit(0);
}

tick-4hz
{
	inter += 1;
	printf("%d\t%d\n", inter, e);
}
