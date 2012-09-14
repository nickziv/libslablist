/*
 * Arbitrary tracing until failure or termination.
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
	printf("%lu\n", (uint64_t)arg1);
	self->lse = args[0]->sli_elems;
	self->lss = args[0]->sli_slabs;
	self->ls = args[0]->sli_name;
	trace(self->act);
	trace(self->lse);
	trace(self->lss);
	trace(self->ls);
	trace(timestamp);
	self->cnt[self->act, self->ls] += 1;
}

slablist$target:::add_*
{

}

pid$target::ripple_update_extrema:entry,
pid$target::ripple_update_extrema:return
{

}

slablist$target:::ripple_rem_slab,
slablist$target:::ripple_add_slab
{

}

slablist$target:::reap_begin,
slablist$target:::reap_end
{

}

slablist$target:::attach_sublayer,
slablist$target:::to_slab
{
	trace(self->ls);
}

slablist$target:::add_head,
slablist$target:::rem_head
{
	trace(args[0]->sli_head);
}

slablist$target:::slab_inc_elems
{
	trace(args[0]->si_elems);
}

slablist$target:::sl_inc_slabs,
slablist$target:::sl_dec_slabs
{
	trace(args[0]->sli_slabs);
}

slablist$target:::sl_inc_elems,
slablist$target:::sl_dec_elems
{
	trace(args[0]->sli_elems);
}

slablist$target:::slab_set_max
{
	trace(arg0);
	printf("max = %lu\n", args[0]->si_max);
}

slablist$target:::slab_set_min
{
	trace(arg0);
	printf("min = %lu\n", args[0]->si_min);
}

slablist$target:::test_is_sml_list,
slablist$target:::test_slab_to_sml,
slablist$target:::test_is_slab_list,
slablist$target:::test_slab_elems_sorted,
slablist$target:::test_slab_elems_max,
slablist$target:::test_smlist_nelems,
slablist$target:::test_smlist_elems_sorted,
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

slablist$target:::test_slabs_sorted
/arg0 == 1/
{
	trace(self->act);
	trace(self->ls);
	trace(self->lse);
	trace(self->cnt[self->act, self->ls]);
	trace(self->lss);
	trace(arg1);
	trace(arg2);
	trace(probename);
	ustack();
	exit(0);
}
