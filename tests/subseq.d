#!/usr/sbin/dtrace

pid$target::do_ops:entry,
pid$target::debug_func:entry,
pid$target::slablist_subseq:entry,
pid$target::slablist_subseq:return,
pid$target::slablist_foldr:entry,
pid$target::slablist_foldr:return,
pid$target::subseq_cb:entry,
pid$target::subseq_cb:return
{
	@[probefunc, probename, arg0, arg1, arg2, arg3] = count();
}
