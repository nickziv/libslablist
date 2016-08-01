slablist$target:::test_foldr_range,
slablist$target:::test_foldl_range
/arg0 > 0/
{
	printf("ERROR: %s %d  %s\n", probename, arg0, sl_e_test_descr[arg0]);
	printf("STACK:");
	ustack();
	exit(0);
}
