#!/bin/awk

{
	if ($1 == "#define" && $2 !~ /\(/) {
		print "inline int", $2, "=", $3";";
	}
}
