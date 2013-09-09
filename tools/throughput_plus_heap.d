#pragma D option quiet

self int endss;

dtrace:::BEGIN
{
	allocated = 0;
	inter = 0;
	prev = 0;
	e = 0;
}

syscall::brk:entry
/pid == $target && !self->endds/
{
	self->endds = arg0;
}

syscall::brk:entry
/pid == $target && self->endds != arg0/
{
	allocated += arg0 - self->endds;
	/* printf("%d\n", self->allocated); */
	self->endds = arg0;
}

slablist$target:::add_begin
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
	printf("%d\t%d\t%d\n", inter, e, allocated);
}
