#!/bin/ksh

cd rcode

print starting...
date
for i in {0..29}; do;
	R CMD BATCH $i.R &
done;

print waiting...
wait
print done
date
