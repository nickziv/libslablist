pid$target:libslablist::entry,
pid$target:libslablist::return
{

}

pid$target:libslablist:slablist_add:entry
{
	trace(arg1);
}

slablist$target:::test_slabs_sorted
/arg0 == 1/
{
	exit(0);
}
