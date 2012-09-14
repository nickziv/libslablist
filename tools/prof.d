pid$target:libslablist.so::entry
{
	this->fdepth = ++fdepth[probefunc];

	/* set start details */
	self->start[probefunc,this->fdepth] = timestamp;
	self->vstart[probefunc,this->fdepth] = vtimestamp;

	/* count occurances */
	@ct[probemod,probefunc] = count();

}

pid$target:libslablist.so::return
/self->start[probefunc,fdepth[probefunc]]/
{
	/* fetch function depth */
	this->fdepth = fdepth[probefunc];

	/* calculate elapsed time */
	this->elapsed = timestamp - self->start[probefunc,this->fdepth];
	self->start[probefunc,this->fdepth] = 0;
	this->cpu = vtimestamp - self->vstart[probefunc,this->fdepth];
	self->vstart[probefunc,this->fdepth] = 0;

	/* save elapsed times */
	@el[probemod,probefunc] = sum(this->elapsed);

	/* save cpu times */
	@CPU[probemod,probefunc] = sum(this->cpu);

}

dtrace:::END
{
	/* print counts */
	printf("Counts\n");
	printa("%-16s %-32s %@16d\n",@ct);

	/* print elapsed times */
	printf("Elapsed\n");
	printa("%-16s %-32s %@16d\n",@el);

	/* print cpu times */
	printf("CPU\n");
	printa("%-16s %-32s %@16d\n",@CPU);
}

