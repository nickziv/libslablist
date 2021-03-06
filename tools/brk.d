#!/usr/sbin/dtrace -s

#pragma D option quiet

self int endds;

syscall::brk:entry
/pid == $target && !self->endds/
{
	self->endds = arg0;
}

syscall::brk:entry
/pid == $target && self->endds != arg0/
{
	printf("%d\n", arg0 - self->endds);
	self->endds = arg0;
}
