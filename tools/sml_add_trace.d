pid$target::small_list_add:entry
{
	self->follow = 1;
}

pid$target:libslablist::entry,
pid$target:libslablist::return
/self->follow == 1/
{

}

pid$target::small_list_add:return
/self->follow == 1/
{
	self->follow = 0;
}

