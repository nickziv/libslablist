#pragma D option quiet

dtrace:::BEGIN
{
	fail = 0;
	/*
	 * Change this to 1 to enable heap-testing. Heap testing is great for
	 * small volumes of data, relative to the amount of RAM you have
	 * available. Once you get to large volumes, these tests will start
	 * dropping variables, and will no longer be reliable. The only way to
	 * prevent this from happening is to get more RAM.
	 */
}

pid$target::mk_slab:return
{
	slabs[arg1] = 1;
}

pid$target::mk_subslab:return
{
	subslabs[arg1] = 1;
}

pid$target::rm_slab:entry
/slabs[arg0] == 2/
{
	fail = 1;
	printf("Trying to free freed subslab. %p\n", arg0);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

pid$target::rm_subslab:entry
/subslabs[arg0] == 2/
{
	fail = 1;
	printf("Trying to free freed subslab. %p\n", arg0);
	printf("\nStack trace:\n");
	printf("------------\n");
	ustack();
	exit(0);
}

pid$target::rm_slab:entry
/slabs[arg0] == 1/
{
	slabs[arg0] = 2;
}

pid$target::rm_subslab:entry
/subslabs[arg0] == 1/
{
	subslabs[arg0] = 2;
}

slablist$target:::add_begin,
slablist$target:::add_end
{
	printf("%s\n", probename);
}


slablist$target:::sl_inc_layer,
slablist$target:::sl_inc_sublayers
{
	printf("%s: %x\n", probename, arg0);
}

slablist$target:::attach_sublayer
{
	self->att++;
}

/*
slablist$target:::bubble_up,
slablist$target:::bubble_up_top
/self->att == 2/
{
	printf("%s: %x\n", probename, arg1);
}

slablist$target:::slab_add_*
{
	printf("%s : %s (%x, %x)\n", probefunc, probename, arg1, arg2);
}

slablist$target:::subslab_add_*
{
	printf("%s : %s (%x, %x, %x)\n", probefunc, probename, arg1, arg2, arg3);
}
*/

pid$target::is_elem_in_range:entry,
pid$target::insert_slab:entry
/slabs[arg1] == 2/
{
	printf("[%s] freed slab %p!\n", probefunc, arg1);
	printf("subslabs[arg1] == %d\n", subslabs[arg1]);
	ustack();
	exit(0);
}

pid$target::is_elem_in_range:entry,
pid$target::insert_slab:entry
/subslabs[arg1] == 1/
{
	printf("[%s] subslab %p instead of a slab!\n", probefunc, arg1);
	ustack();
	exit(0);
}

pid$target::is_elem_in_range:entry
/slabs[arg1] == 0/
{
	printf("[%s] unallocated slab %p!\n", probefunc, arg1);
	ustack();
	exit(0);
}
