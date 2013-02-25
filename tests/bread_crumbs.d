#pragma D option quiet

dtrace:::BEGIN
{
	fail = 0;
}

slablist$target:::test_bread_crumbs
/arg0 == 1/
{
	fail = arg0;
	printf("The elems in the bc_path are not ascending layer-wise.\n");
	exit(0);
}

dtrace:::END
/fail == 0/
{
	printf("All tests passed.");
}
