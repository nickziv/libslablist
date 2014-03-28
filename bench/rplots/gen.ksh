#!/bin/ksh

# arg1 = the /bench dir
# arg2 = the build/bench results dir
# arg3 = input size
# arg4 = machine name

source $1/main/impls


f1names="c(\"ms\", \"elems\", \"heapsz\")"
f2names="c(\"ms\", \"elemsrate\")"

font_sz=50
lnpt_sz=30
# The length of the X axis is the number of elements, usually. But we can
# narrow it down to see more details... Be sure to comment out the xlim lines
# below...
xlima=0
xlimb=10000000


# Taken from Wad's 16 distinct colors
#gray
rcol[0]="#575757"
#black
rcol[1]="#000000"
#red
rcol[2]="#AD2323"
#blue
rcol[3]="#2A4BD7"
#green
rcol[4]="#1D6914"
#brown
rcol[5]="#814A19"
#purple
rcol[6]="#8127C0"
# light gray
rcol[7]="#A0A0A0"
# light green
rcol[8]="#81C57A"
#light blue
rcol[9]="#9DAFFF"
#cyan
rcol[10]="#29D0D0"
#orange
rcol[11]="#FF9233"
#yellow
rcol[12]="FFEE33"
#tan
rcol[13]="E9DEBB"
#pink
rcol[14]="FFCDF3"
#white
rcol[15]="FFFFFF"

pat[0]=rand
pat[1]=seq
geom[0]=point
geom[1]=line
x[0]=ms
x[1]=elems
x[2]=elems
x[3]=ms
y[0]=heapsz
y[1]=ms
y[2]=heapsz
y[3]=elemsrate

FN=0
outdir="rcode"
output=$outdir/$FN.R
rm -r $outdir
mkdir $outdir
rm -r imgs
mkdir imgs

for z in {0..11}; do

output=$outdir/$z.R

print "library(ggplot2)" >> $output
# First we generate the fucking files.
for i in {0..$nimpls}; do;
	struct=${impl[$i]}
	file="$2/"${impl[$i]}"/$4_throughput_plus_heap_rand_intsrt_$3"
	print "rand_df_$struct <- read.table('$file', col.names=$f1names);" >> $output
	file="$2/"${impl[$i]}"/$4_throughput_plus_heap_seqinc_intsrt_$3"
	print "seq_df_$struct <- read.table('$file', col.names=$f1names);" >> $output
done;

done;

jpw="width = 2800"
jph="height = 2800"
prefix="plotvar <- plotvar"

props[0]="size = 6"
props[1]="size = 6"
propsX[0]="size = 6"
propsX[1]="size = 6"

# Here we generate the regular X vs. Y plots.
z=0
for px in {0..1}; do
  for gx in {0..1}; do;
    for cx in {0..2}; do;
      output=$outdir/$z.R   
      print "jpeg('../imgs/${pat[$px]}_${x[$cx]}_${y[$cx]}_${geom[$gx]}.jpeg', $jpw, $jph)" >> $output
      print "plotvar <- ggplot();" >> $output
      print "$prefix + ggtitle(\"Comparison of All Structs\")" >> $output

      # We make the geoms for the current plot.
      #for ix in {0..$nimpls}; do;
      for ix in {0..$nimpls}; do;
	data=${pat[$px]}_df_${impl[$ix]}
	aes="aes(x = ${x[$cx]}, y = ${y[$cx]}, colour = '${rcol[$ix]}')"
	aesX="aes(x = ${x[$cx]}, y = ${y[$cx]})"
	#print "$prefix + geom_${geom[$gx]}(data = $data, $aes, ${props[$gx]}, colour = '${col[$ix]}');" >> $output
	print "$prefix + geom_${geom[$gx]}(data = $data, $aes, ${props[$gx]});" >> $output
      done;

      # And now the legend
      print "$prefix + scale_colour_identity(name = 'data structure', " >> $output
      print "breaks = c(" >> $output
      for ix in {0..$nimpls}; do;
	if [[ $ix -eq $nimpls ]]; then 
	  print "'${rcol[$ix]}')," >> $output
	else
	  print "'${rcol[$ix]}'," >> $output
	fi
      done;
      print "labels = c(" >> $output
      for ix in {0..$nimpls}; do;
	if [[ $ix -eq $nimpls ]]; then 
	  print "'${impl[$ix]}')," >> $output
	else
	  print "'${impl[$ix]}'," >> $output
	fi
      done;
      print "guide = 'legend');" >> $output

      print "$prefix + guides(colour = guide_legend(override.aes = list(size=$lnpt_sz)));" >> $output
      #print "$prefix + xlim($xlima, $xlimb);" >> $output
      print "$prefix + theme(title=element_text(face='bold',size=$font_sz));" >> $output
      print "$prefix + theme(axis.title.x=element_text(face='bold',size=$font_sz));" >> $output
      print "$prefix + theme(axis.title.y=element_text(face='bold',size=$font_sz));" >> $output
      print "$prefix + theme(axis.text.x=element_text(size=$font_sz));" >> $output
      print "$prefix + theme(axis.text.y=element_text(size=$font_sz));" >> $output
      print "$prefix + theme(legend.title=element_text(face='bold',size=$font_sz));" >> $output
      print "$prefix + theme(legend.text=element_text(size=$font_sz));" >> $output

      print "print(plotvar);" >> $output
      print "dev.off();" >> $output
      z=`echo $z + 1 | bc`
    done;
  done;
done;

# This is the header for the plotting code the follows.
for z in {12..13}; do

output=$outdir/$z.R

print "library(ggplot2)" >> $output
# First we generate the fucking files.
for i in {0..$nimpls}; do;
	struct=${impl[$i]}
	file="$2/"${impl[$i]}"/$4_throughput_post_rand_intsrt_$3"
	print "rand_df2_$struct <- read.table('$file', col.names=$f2names);" >> $output
	file="$2/"${impl[$i]}"/$4_throughput_post_seqinc_intsrt_$3"
	print "seq_df2_$struct <- read.table('$file', col.names=$f2names);" >> $output
done;

done;

props[0]="size = 3, alpha = 0.1"
z=12
# Here we generate some Elems vs. Time plots, but we plot the _rate_ of
# insertion, per millisecond.
gx=0
cx=3
for px in {0..1}; do
  output=$outdir/$z.R
  print "jpeg('../imgs/${pat[$px]}_${x[$cx]}_${y[$cx]}_${geom[$gx]}.jpeg', $jpw, $jph)" >> $output
  print "plotvar <- ggplot();" >> $output
  print "$prefix + ggtitle(\"Insertion Rates of All Structs\")" >> $output
  for ix in {0..$nimpls}; do;
	data=${pat[$px]}_df2_${impl[$ix]}
	aes="aes(x = ${x[$cx]}, y = ${y[$cx]}, colour = '${rcol[$ix]}')"
	aesX="aes(x = ${x[$cx]}, y = ${y[$cx]})"
	#print "$prefix + geom_${geom[$gx]}(data = $data, $aes, ${props[$gx]}, colour = '${col[$ix]}');" >> $output
	print "$prefix + geom_${geom[$gx]}(data = $data, $aes, ${props[$gx]});" >> $output
  done;
  # And now the legend
  print "$prefix + scale_colour_identity(name = 'data structure', " >> $output
  print "breaks = c(" >> $output
  for ix in {0..$nimpls}; do;
    if [[ $ix -eq $nimpls ]]; then 
      print "'${rcol[$ix]}')," >> $output
    else
      print "'${rcol[$ix]}'," >> $output
    fi
  done;
  print "labels = c(" >> $output
  for ix in {0..$nimpls}; do;
    if [[ $ix -eq $nimpls ]]; then 
      print "'${impl[$ix]}')," >> $output
    else
      print "'${impl[$ix]}'," >> $output
    fi
  done;
  print "guide = 'legend');" >> $output
  print "$prefix + guides(colour = guide_legend(override.aes = list(size=$lnpt_sz)));" >> $output
  #print "$prefix + xlim($xlima, $xlimb);" >> $output
  print "$prefix + theme(title=element_text(face='bold',size=$font_sz));" >> $output
  print "$prefix + theme(axis.title.x=element_text(face='bold',size=$font_sz));" >> $output
  print "$prefix + theme(axis.title.y=element_text(face='bold',size=$font_sz));" >> $output
  print "$prefix + theme(axis.text.x=element_text(size=$font_sz));" >> $output
  print "$prefix + theme(axis.text.y=element_text(size=$font_sz));" >> $output
  print "$prefix + theme(legend.title=element_text(face='bold',size=$font_sz));" >> $output
  print "$prefix + theme(legend.text=element_text(size=$font_sz));" >> $output


  print "print(plotvar);" >> $output
  print "dev.off();" >> $output
  z=`echo $z + 1 | bc`
done;

# We generate _individual_ plots for insertion rate, per MS.

props[0]="size = 3, alpha = 0.6"
z=14
gx=0
cx=3
for ix in {0..$nimpls}; do;
	output=$outdir/$z.R
	struct=${impl[$ix]}
	print "library(ggplot2)" >> $output
	file="$2/"${impl[$ix]}"/$4_throughput_post_rand_intsrt_$3"
	print "rand_df2_$struct <- read.table('$file', col.names=$f2names);" >> $output
	file="$2/"${impl[$ix]}"/$4_throughput_post_seqinc_intsrt_$3"
	print "seq_df2_$struct <- read.table('$file', col.names=$f2names);" >> $output
	print "jpeg('../imgs/${impl[$ix]}_${x[$cx]}_${y[$cx]}_${geom[$gx]}.jpeg', $jpw, $jph)" >> $output
	#print "plotvar <- ggplot();" >> $output
	aesX="aes(x = ${x[$cx]}, y = ${y[$cx]})"
	print "plotvar <- ggplot();" >> $output
	print "$prefix + ggtitle(\"Insertion Rates of $struct\")" >> $output
	for px in {0..1}; do 
		data=${pat[$px]}_df2_${impl[$ix]}
		aes="aes(x = ${x[$cx]}, y = ${y[$cx]}, colour = '${rcol[$px]}')"
		print "$prefix + geom_${geom[$gx]}(data = $data, $aes, ${props[$gx]});" >> $output
	done;
	#print "$prefix + geom_${geom[$gx]}(data = $data, $aes, ${props[$gx]});" >> $output
	#print "$prefix + stat_bin2d(bins = 50) + scale_fill_gradient(low="lightblue", high="red", limits=c(0, 6000));" >> $output
	#print "$prefix + stat_bin2d(bins = 70) + scale_fill_gradient(low='lightblue', high='red');" >> $output
	# And now the legend
	print "$prefix + scale_colour_identity(name = 'data structure', " >> $output
	print "breaks = c(" >> $output
	for ix in {0..1}; do;
		if [[ $ix -eq 1 ]]; then 
			print "'${rcol[$ix]}')," >> $output
		else
			print "'${rcol[$ix]}'," >> $output
		fi
	done;
	print "labels = c(" >> $output
	for ix in {0..1}; do;
		if [[ $ix -eq 1 ]]; then 
			print "'${pat[$ix]}')," >> $output
		else
			print "'${pat[$ix]}'," >> $output
		fi
	done;
	print "guide = 'legend');" >> $output
	print "$prefix + guides(colour = guide_legend(override.aes = list(size=$lnpt_sz)));" >> $output
        #print "$prefix + xlim($xlima, $xlimb);" >> $output
	print "$prefix + theme(title=element_text(face='bold',size=$font_sz));" >> $output
	print "$prefix + theme(axis.title.x=element_text(face='bold',size=$font_sz));" >> $output
	print "$prefix + theme(axis.title.y=element_text(face='bold',size=$font_sz));" >> $output
	print "$prefix + theme(axis.text.x=element_text(size=$font_sz));" >> $output
	print "$prefix + theme(axis.text.y=element_text(size=$font_sz));" >> $output
	print "$prefix + theme(legend.title=element_text(face='bold',size=$font_sz));" >> $output
	print "$prefix + theme(legend.text=element_text(size=$font_sz));" >> $output


	print "print(plotvar);" >> $output
	print "dev.off();" >> $output
	z=`echo $z + 1 | bc`
done;
