#!/bin/nawk
# arg1 = the /bench dir
# arg2 = the build/bench results dir
# arg3 = input size
# arg4 = machine name


BEGIN {

f1names = "c(\"ms\", \"elems\", \"heapsz\")";
f2names = "c(\"ms\", \"elemsrate\")";

font_sz = 50;
lnpt_sz = 30;
xlima = 0;
xlimb = 10000000;

impl[0] = "uuavl";
impl[1] = "jmpc_skl_16";
impl[2] = "gnuavl";
impl[3] = "gnupavl";
impl[4] = "gnutavl";
impl[5] = "gnurtavl";
impl[6] = "gnurb";
impl[7] = "gnuprb";
impl[8] = "jmpc_btree_512";
impl[9] = "jmpc_btree_1024";
impl[10] = "jmpc_btree_4096";
impl[11] = "sl";

impl_first = impl[0];
impl_last = impl[11];


# Taken from Wad's 16 distinct colors
#gray
rcol["uuavl"] = "#575757";
rcol["rand"] = "#575757";
#black
rcol["jmpc_skl_16"] = "#000000";
rcol["seq"] = "#000000";
#red
rcol["gnuavl"] = "#AD2323";
#blue
rcol["gnupavl"] = "#2A4BD7";
#green
rcol["gnutavl"] = "#1D6914";
#brown
rcol["gnurtavl"] = "#814A19";
#purple
rcol["gnurb"] = "#8127C0";
# light gray
rcol["gnuprb"] = "#A0A0A0";
# light green
rcol["jmpc_btree_512"] = "#81C57A";
#light blue
rcol["jmpc_btree_1024"] = "#9DAFFF";
#cyan
rcol["jmpc_btree_4096"] = "#29D0D0";
#orange
rcol["sl"] = "#FF9233";
# Rest are unused, for now.
#yellow
rcol[12] = "FFEE33";
#tan
rcol[13] = "E9DEBB";
#pink
rcol[14] = "FFCDF3";
#white
rcol[15] = "FFFFFF";

pattern[0] = "rand";
pattern[1] = "seq";
geometry[0] = "point";
geometry[1] = "line";
x[0] = "ms";
x[1] = "elems";
x[2] = "elems";
x[3] = "ms";
y[0] = "heapsz";
y[1] = "ms";
y[2] = "heapsz";
y[3] = "elemsrate";

file_num = 0;
outdir = "rcode";
output = outdir "/" file_num ".R";
system("rm -r " outdir);
system("mkdir " outdir);
system("rm -r imgs");
system("mkdir imgs");

for (z = 0; z <= 11; z++) {
	output = outdir "/" z ".R";

	print "library(ggplot2)" >> output;
	# First we generate the files.
	for (im = 0; im <= 11; im++) {
		i = impl[im];
		file = arg2 "/" i "/" arg4\
		    "_throughput_plus_heap_rand_intsrt_" arg3;

		print "rand_df_" i " <- read.table('" file "', col.names="\
		     f1names ");" >> output;

		file = arg2 "/" i "/" arg4\
		    "_throughput_plus_heap_seqinc_intsrt_" arg3;

		print "seq_df_" i " <- read.table('" file "', col.names="\
		    f1names ");" >> output;
	}
}

jpw = "width = 2800";
jph = "height = 2800";
prefix = "plotvar <- plotvar";

props["point"] = "size = 6";
props["line"] = "size = 6";

# Here we generate the regular X vs. Y plots.
z = 0;
for (p in pattern) {
	pat = pattern[p];
	for (g in geometry) {
	geom = geometry[g];
		for (coord = 0; coord <= 2; coord++) {
			output =outdir "/" z ".R";
			print "jpeg('../imgs/" pat "_" x[coord] "_" y[coord]\
			    "_" geom ".jpeg', "jpw ", "jph ")" >> output;

			print "plotvar <- ggplot();" >> output;
			print prefix " + ggtitle(\"Comparison of All Structs\")" >> output;

			# We make the geoms for the current plot.
			for (im = 0; im <= 11; im++) {
				i = impl[im];
				data = pat  "_df_" i;
				aes="aes(x = " x[coord] ", y = "y[coord]\
				    ", colour = '"rcol[i] "')";

				print prefix " + geom_" geom "(data = " data\
				    ", "aes", "props[geom]");" >> output;
			}

			# And now the legend
			print prefix " + scale_colour_identity(name = 'data structure', "\
			    >> output;

			print "breaks = c(" >> output;

			for (im = 0; im <= 11; im++) {
				i = impl[im];
				if (i == impl_last) {
					print "'"rcol[i] "')," >> output;
				} else {
					print "'" rcol[i] "'," >> output;
				}
			}

			print "labels = c(" >> output;

			for (im = 0; im <= 11; im++) {
				i = impl[im];
				if (i == impl_last) {
					print "'" i "')," >> output;
				} else {
					print "'" i "'," >> output;
				}
			}

			print "guide = 'legend');" >> output;

			print prefix " + guides(colour = guide_legend("\
			    "override.aes = list(size="lnpt_sz")));" >> output;
			#print prefix" + xlim($xlima, $xlimb);" >> output;
			print prefix " + theme(title=element_text(face='bold',size="\
			    font_sz "));" >> output;

			print prefix " + theme(axis.title.x=element_text("\
			    "face='bold',size=" font_sz"));" >> output;

			print prefix " + theme(axis.title.y=element_text("\
			    "face='bold',size=" font_sz"));" >> output;

			print prefix " + theme(axis.text.x=element_text(size="\
			    font_sz"));" >> output;

			print prefix " + theme(axis.text.y=element_text(size="\
			    font_sz"));" >> output;

			print prefix " + theme(legend.title=element_text(face="\
			    "'bold',size="font_sz"));" >> output;

			print prefix " + theme(legend.text=element_text(size="\
			    font_sz"));" >> output;

			print "print(plotvar);" >> output;
			print "dev.off();" >> output;
			z++;
		}
	}
}

# This is the header for the plotting code that follows.
while (z <= 13) {
	output = outdir "/" z ".R";
	print "library(ggplot2)" >> output;
	# First we generate the files.
	for (im = 0; im <= 11; im++) {
		i = impl[im];
		file = arg2"/" i "/"arg4"_throughput_post_rand_intsrt_"arg3;
		print "rand_df2_" i " <- read.table('"file"', col.names="f2names");" >> output;
		file = arg2"/" i "/"arg4"_throughput_post_seqinc_intsrt_"arg3;
		print "seq_df2_" i " <- read.table('"file"', col.names="f2names");" >> output;
	}
	z++;
}

# XXX STOPPED HERE
props["point"]="size = 3, alpha = 0.1";
z=12;
# Here we generate some Elems vs. Time plots, but we plot the _rate_ of
# insertion, per millisecond.
geom = geometry[0];
coord = 3;
for (p in pattern) {
	pat = pattern[p];
	output = outdir "/" z ".R";
	print "jpeg('../imgs/"pat"_"x[coord]"_"y[coord]"_"geom".jpeg', "jpw", "jph")" >> output;
	print "plotvar <- ggplot();" >> output;
	print prefix" + ggtitle(\"Insertion Rates of All Structs\")" >> output;
	for (im = 0; im <= 11; im++) {
		i = impl[im];
		data = pat"_df2_"i;
		aes = "aes(x = "x[coord]", y = "y[coord]", colour = '"rcol[i]"')";
		print prefix" + geom_"geom"(data = "data", "aes", "props[geom]");" >> output;
	}

	# And now the legend
	print prefix" + scale_colour_identity(name = 'data structure', " >> output;
	print "breaks = c(" >> output;
	for (im = 0; im <= 11; im++) {
		i = impl[im];
		if (i == impl_last) {
			print "'"rcol[i]"')," >> output;
		} else {
			print "'"rcol[i]"'," >> output;
		}
	}
	print "labels = c(" >> output;
	for (im = 0; im <= 11; im++) {
		i = impl[im];
		if (i == impl_last) {
			print "'" i "')," >> output;
		} else {
			print "'" i "'," >> output;
		}
	}

	print "guide = 'legend');" >> output;
	print prefix" + guides(colour = guide_legend(override.aes = list("\
	    "size="lnpt_sz")));" >> output;
	print prefix " + theme(title=element_text(face='bold',size="font_sz\
	    "));" >> output;
	print prefix " + theme(axis.title.x=element_text(face='bold',size="\
	    font_sz"));" >> output;
	print prefix " + theme(axis.title.y=element_text(face='bold',size="\
	    font_sz"));" >> output;
	print prefix " + theme(axis.text.x=element_text(size="\
	    font_sz"));" >> output;
	print prefix " + theme(axis.text.y=element_text(size="\
	    font_sz"));" >> output;
	print prefix " + theme(legend.title=element_text(face='bold',size="\
	    font_sz"));" >> output;
	print prefix " + theme(legend.text=element_text(size="\
	    font_sz"));" >> output;


	print "print(plotvar);" >> output;
	print "dev.off();" >> output;
	z++;
}

# We generate _individual_ plots for insertion rate, per MS, comparing seq and
# rand.

props["point"]="size = 3, alpha = 0.6";
z = 14;
geom = geometry[0];
coord = 3;
for (im = 0; im <= 11; im++) {
	i = impl[im];
	output = outdir "/" z ".R";
	print "library(ggplot2)" >> output;
	file = arg2"/" i "/"arg4"_throughput_post_rand_intsrt_"arg3;
	print "rand_df2_" i " <- read.table('"file"', col.names="f2names");"\
	    >> output;
	file = arg2"/" i "/"arg4"_throughput_post_seqinc_intsrt_"arg3;
	print "seq_df2_" i " <- read.table('"file"', col.names="f2names");"\
	    >> output;
	print "jpeg('../imgs/"i"_"x[coord]"_"y[coord]"_"geom".jpeg', "jpw\
	    ", "jph")" >> output;
	#print "plotvar <- ggplot();" >> output;
	print "plotvar <- ggplot();" >> output;
	print prefix" + ggtitle(\"Insertion Rates of " i "\")" >> output;
	for (p in pattern) {
		pat = pattern[p];
		data = pat"_df2_"i;
		aes="aes(x = "x[coord]", y = "y[coord]", colour = '"rcol[pat]"')";
		print prefix " + geom_"geom"(data = "data", "aes", "\
		    props[geom]");" >> output;
	}

	# And now the legend
	print prefix " + scale_colour_identity(name = 'data structure', "\
	    >> output;
	print "breaks = c(" >> output;
	print "'"rcol["rand"]"'," >> output;
	print "'"rcol["seq"]"')," >> output;
	print "labels = c(" >> output;
	print "'"pattern[0]"'," >> output;
	print "'"pattern[1]"')," >> output;
	print "guide = 'legend');" >> output;
	print prefix" + guides(colour = guide_legend(override.aes ="\
	    " list(size="lnpt_sz")));" >> output;
	print prefix" + theme(title=element_text(face='bold',size="\
	    font_sz"));" >> output;
	print prefix" + theme(axis.title.x=element_text(face='bold',size="\
	    font_sz"));" >> output;
	print prefix" + theme(axis.title.y=element_text(face='bold',size="\
	    font_sz"));" >> output;
	print prefix" + theme(axis.text.x=element_text(size="\
	    font_sz"));" >> output;
	print prefix" + theme(axis.text.y=element_text(size="\
	    font_sz"));" >> output;
	print prefix" + theme(legend.title=element_text(face='bold',size="\
	    font_sz"));" >> output;
	print prefix" + theme(legend.text=element_text(size="\
	    font_sz"));" >> output;


	print "print(plotvar);" >> output;
	print "dev.off();" >> output;
	z++;
}

}
