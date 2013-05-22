#!/usr/sbin/dtrace -s

#pragma D option quiet

self int endds;

dtrace:::BEGIN
{
	allocated = 0;
	sec = 0;
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

tick-1s
{
	sec++;
	printf("%d\t%d\n", sec, allocated);
}
