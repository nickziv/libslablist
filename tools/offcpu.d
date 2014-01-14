#pragma D option quiet

dtrace:::BEGIN {
	start = timestamp;
}

sched:::off-cpu
/pid == $target/
{
	@[execname] = lquantize((timestamp - start) / 1000000000, 0, 40, 1);
}

tick-1s
/++i >= 40/
{
	exit(0);
}
