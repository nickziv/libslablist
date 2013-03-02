BEGIN {
	prev = 0;
	cur = 0;
}

{
	cur = $2;
	print $1" "$2" "($2 - prev);
	prev = cur;
}
