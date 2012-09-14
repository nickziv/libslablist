/*
 * This script is meant to provide detailed error reporting on first failure.
 * So far the error reporting is the same for every test probe, but some day
 * each probe will have its own set of actions and pretty printing.
 */
dtrace:::BEGIN
{
	self->cnt["a", "a"] = 1;
}

slablist$target:::test
{

}

slablist$target:::add_begin,
slablist$target:::rem_begin
{
	self->act = probename;
	self->lse = args[0]->sli_elems;
	self->lss = args[0]->sli_slabs;
	self->ls = args[0]->sli_name;
	self->cnt[self->act, self->ls] += 1;
}

slablist$target:::test_is_sml_list,
slablist$target:::test_slab_to_sml,
slablist$target:::test_is_slab_list,
slablist$target:::test_slab_elems_sorted,
slablist$target:::test_slab_elems_max,
slablist$target:::test_smlist_nelems,
slablist$target:::test_smlist_elems_sorted,
slablist$target:::test_slabs_sorted,
slablist$target:::test_move_next,
slablist$target:::test_slab_bkptr,
slablist$target:::test_slab_extrema,
slablist$target:::test_sublayer_extrema,
slablist$target:::test_sublayer_elems_sorted,
slablist$target:::test_sublayers_sorted,
slablist$target:::test_sublayers_have_all_slabs,
slablist$target:::test_bread_crumbs,
slablist$target:::test_nullarg,
slablist$target:::test_bubble_up,
slablist$target:::test_nelems,
slablist$target:::test_sublayer_nelems
/arg0 == 1/
{
	trace(self->act);
	trace(self->ls);
	trace(self->lse);
	trace(self->cnt[self->act, self->ls]);
	trace(self->lss);
	trace(probename);
	ustack();
	exit(0);
}
