slablist$target:::test
{

}
/*
 * This file runs all the tests and aggregates on whether the tests failed or
 * not. If a test failed, the user can use other test scripts to determine the
 * path to failure, and other details about the failure.
 */
slablist$target:::test_slab_elems_sorted,
slablist$target:::test_slab_to_sml,
slablist$target:::test_smlist_nelems,
slablist$target:::test_smlist_elems_sorted,
slablist$target:::test_slabs_sorted,
slablist$target:::test_is_sml_list,
slablist$target:::test_is_slab_list,
slablist$target:::test_slab_bkptr,
slablist$target:::test_move_next
{
	@a[probename, arg0] = count();
}

slablist$target:::test_slab_extrema,
slablist$target:::test_sublayer_elems_sorted,
slablist$target:::test_sublayers_sorted,
slablist$target:::test_sublayers_have_all_slabs,
slablist$target:::test_bread_crumbs,
slablist$target:::test_nullarg,
slablist$target:::test_bubble_up
{
	@s[probename, arg0, arg1] = count();
}

slablist$target:::test_sublayer_extrema
{
	@[probename, arg0, arg1, arg2, arg3] = count();
}

slablist$target:::test_nelems
{
	@d[probename, arg0] = count();
}

slablist$target:::test_sublayer_nelems
{
	@f[probename, arg0, arg3] = count();
}
