BEGIN {
	prev = 0;
	cur = 0;
}

{
	cur = $2;
	if ($1 != "") {
		print $1" "($2 - prev);
	}
	prev = cur;
}
