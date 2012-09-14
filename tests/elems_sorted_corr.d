slablist$target:::test_slab_elems_sorted,
slablist$target:::test_smlist_elems_sorted
/arg0 != 1/
{
	trace(probename);
	trace(arg0);
}

slablist$target:::test_slab_elems_sorted,
slablist$target:::test_smlist_elems_sorted
/arg0 == 1/
{
	trace(probename);
	trace(arg0);
	exit(0);
}

slablist$target:::add_*,
slablist$target:::rem_*,
slablist$target:::split_*
{

}

slablist$target:::add_into
{
	trace(arg0);
}
