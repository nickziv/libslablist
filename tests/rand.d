/*
 * Verifies that the random values we return correspond to values that we have
 * already inserted.
 */
pid$target::slablist_add:entry
{
	elems[arg1] = 1;
}

slablist$target:::rem_end
{
	@s[arg0] = count();
}

slablist$target:::get_rand
/elems[arg1] != 1/
{
	@["error", args[0]->sli_is_small_list] = count();
}

slablist$target:::get_rand
/elems[arg1] == 1/
{
	@["correct", args[0]->sli_is_small_list] = count();
}
