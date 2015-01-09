#pragma D option quiet

self int endss;

dtrace:::BEGIN
{
	allocated = (uint64_t)0;
	inter = 0;
	e = 0;
}

syscall::brk:entry
/pid == $target && !self->endds/
{
	self->endds = arg0;
}

syscall::brk:entry
/pid == $target && self->endds < arg0/
{
	allocated += (uint64_t)((uint64_t)arg0 - (uint64_t)self->endds);
	/* printf("%d\n", self->allocated); */
	self->endds = arg0;
}

syscall::brk:entry
/pid == $target && self->endds > arg0/
{
	printf("self->endss is > to arg0\n");
	exit(0);
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
	printf("%u\t%u\t%u\n", inter, e, allocated);
}
