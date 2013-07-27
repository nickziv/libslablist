#pragma flowindent

pid$target:libslablist::entry
{
	printf("(%u, %u, %u, %u)", arg0, arg1, arg2, arg3);
}

pid$target:libslablist::return
{
	printf("(%u)", arg1);
}
