dtrace:::BEGIN
{
	reaps = 0;
}

slablist$target:::reap_begin
{
	self->follow = 1;
	self->rts = timestamp;
	self->slabs = args[0]->sli_slabs;
}

pid$target::move_to_next:entry,
pid$target::move_to_prev:entry
/self->follow == 1/
{
	self->follow = 2;
}

pid$target::bcopy:entry
/self->follow == 2/
{
	@mvdavg["avg"] = avg(arg2);
	@mvdqtz["qtz"] = lquantize(arg2, 0, 2000, 100);
}

pid$target::bcopy:return
/self->follow == 2/
{
	self->follow = 1;
}

slablist$target:::reap_end
/self->follow == 1/
{
	self->rtsdiff = timestamp - self->rts;
	self->slabs_collected = self->slabs - args[0]->sli_slabs;
	self->follow = 0;
	reaps += 1;
	@rpavg["rpavg"] = avg(self->rtsdiff);
	@rpqtz["rpqtz"] = quantize(self->rtsdiff);
	@rpcol["rpcol", self->slabs_collected, self->slabs] = count();
}

dtrace:::END
{
	printf("Reaps: %d\n", reaps);
}
