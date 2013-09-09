BEGIN {
	print "TIME, DELTA"
	prev = 0;
	cur = 0;
}

{
	cur = $2;
	print $1" "($2 - prev);
	prev = cur;
}
