slablist$target:::rem_begin
{
	self->sl_sm = args[0]->sli_is_small_list;
}

slablist$target:::rem_end
/arg0 != 0/
{
	@[arg0, self->sl_sm] = count();
}
