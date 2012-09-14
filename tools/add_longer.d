/*
 * Gives a list of all functions called between begin and end of an addition,
 * if the addition took longer than or equal to $1.
 */
#pragma D option specsize=128m
#pragma D option nspec=10

slablist$target:::add_begin
/args[0]->sli_is_small_list == 0/
{
	self->ts = vtimestamp;
	self->elems = args[0]->sli_slabs;
	self->name = args[0]->sli_name;
	self->type = args[0]->sli_is_small_list;
	self->follow = 1;
	trace(self->name);
	trace(self->type);
	self->maybe = speculation();
}

pid$target:libslablist::entry,
pid$target:libslablist::return
/self->follow == 1/
{
	speculate(self->maybe);
}

slablist$target:::add_end
/vtimestamp - self->ts >= $1/
{
	commit(self->maybe);
	self->follow = 0;
}

slablist$target:::add_end
/vtimestamp - self->ts < $1/
{
	discard(self->maybe);
	self->follow = 0;
}
