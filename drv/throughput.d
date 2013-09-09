#pragma D option quiet

dtrace:::BEGIN
{
	inter = 0;
	prev = 0;
	e = 0;
}

struc$target:::add_begin
{
	e += 1;
}

pid$target::end:entry
{
	exit(0);
}

tick-1ms
{
	inter += 1;
	printf("%d\t%d\n", inter, e);
}
