pid$target:libslablist::entry
{
	self->ts = timestamp;
}

pid$target:libslablist::return
{
	@[probefunc] = sum(timestamp - self->ts);
}
