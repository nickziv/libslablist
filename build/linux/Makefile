include ../Makefile.master

PREFIX=		$(LINUX_PREFIX)
LIBS+=
DSLIBS+=
CFLAGS+=
UMEM_CFLAGS=	$(CFLAGS) "-Wno-unused-parameter"
DSLDFLAGS=	$(DSLDF_LINUX)

SL_PROV=	slablist_provider.d
DS_PROV=	struc_provider.d

OBJECTS=	$(C_OBJECTS)
DS_D_OBJECTS=
D_OBJECT=


#
# This is used for debugging (like expanding variables), because Make sucks at
# error messages.
#
fuckit: 
	echo $(DTRACE)
	echo $(DTRACE_LIBS)
	echo $(DSLDFLAGS)

#echo $(TEST_RES)
#echo $(TEST_SCR)

.PHONY: $(DS_IMPLS)

.PHONY: $(DS_IMPLS_EXT)

#.PHONY: $(C_SRCS)

$(C_SRCS): %.c:

$(C_OBJECTS): %.o: %.c $(C_HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<
	$(CTFC_POSTPROC)
#$(CTFCONVERT) -i -L VERSION $@

objs: $(OBJECTS)

$(SO): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(SO) $(OBJECTS) $(LIBS)
	$(CTFM_POSTPROC)

all: $(SO) $(DRV)

$(R_TEST):
	-mkdir $@

$(R_TEST_SI): $(R_TEST)
	-mkdir $@

$(R_TEST_SD): $(R_TEST)
	-mkdir $@

$(R_TEST_R): $(R_TEST)
	-mkdir $@

$(TEST_RES_SI): $(R_TEST_SI)/%.out: $(TEST)/%.d $(R_TEST_SI)
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt rem seqinc' -s $< -o $@

$(TEST_RES_SD): $(R_TEST_SD)/%.out: $(TEST)/%.d $(R_TEST_SD)
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt rem seqdec' -s $< -o $@

$(TEST_RES_R): $(R_TEST_R)/%.out: $(TEST)/%.d $(R_TEST_R)
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt rem rand' -s $< -o $@

tests: $(TEST_RES_R) $(TEST_RES_SD) $(TEST_RES_SI)


# $(CTFMERGE) -L VERSION -o $@ $(C_OBJECTS)


uninstall:
	sudo rm -r $(PREFIX) 2> /dev/null

install: 
	-sudo rm -r $(PREFIX) 2> /dev/null
	sudo mkdir $(PREFIX)
	sudo mkdir $(PREFIX)/lib/
	sudo cp $(SO) $(PREFIX)/lib/
	sudo ln -s $(PREFIX)/lib/$(SO) $(PREFIX)/lib/libslablist.so
	sudo mkdir $(PREFIX)/include
	sudo mkdir $(PREFIX)/include/dtrace
	sudo cp $(SLDIR)/slablist.h $(PREFIX)/include/
	sudo cp $(SLDIR)/slablist.d $(PREFIX)/include/dtrace/

$(DS_SRCS): %.c:

$(DRV_SRCS): %.c:


$(DS_OBJECTS): %.o: %.c
	$(CC) $(DSCFLAGS) -o $@ -c $<


$(DRV_OBJECT): %.o: %.c $(DS_OBJECTS)
	$(CC) $(DSCFLAGS) $(DSCINC) -o $@ -c $(DRV_SRCS)

$(DRV): install $(DRV_OBJECT) $(DS_OBJECTS)
	$(CC) $(DSCFLAGS) -o $@ $(DS_OBJECTS) $(DS_D_OBJECTS) $(DRV_OBJECT) $(DSLDFLAGS) $(DSLIBS)

$(PLISTS): %.plist: %.c $(D_HDRS)
	$(CKSTATIC) -D UMEM $< -o $@

$(CSTYLES): %.cstyle: %.c
	$(CSTYLE) $<

.PHONY: cstyle
cstyle: $(CSTYLES)


$(R_BENCH): 
	mkdir $(R_BENCH)
	mkdir $(R_BENCH)/sl
	

$(R_BENCH_SD): $(R_BENCH)/%: %.Z $(R_BENCH)
	mkdir $@

$(SL_BENCH_R_I): $(DRV) $(R_BENCH_SD) 
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt rand' -s $(BENCH_SL_THR_HEAP) -o $@
	$(AWK) -f $(BENCH_PPROC) $(R_BENCH)/sl/$(DS_RI_SUF) > $(R_BENCH)/sl/$(DS_RI_PP_SUF)

$(SL_BENCH_S_I): $(DRV) $(R_BENCH_SD) 
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt seqinc' -s $(BENCH_SL_THR_HEAP) -o $@
	$(AWK) -f $(BENCH_PPROC) $(R_BENCH)/sl/$(DS_SI_SUF) > $(R_BENCH)/sl/$(DS_SI_PP_SUF)

$(DS_BENCHES_R_I): $(R_BENCH)/%/$(DS_RI_SUF): %
	$(DTRACE) -c './$(DRV) $< $(BENCH_SIZE) intsrt rand' -s $(BENCH_GEN_THR_HEAP) -o $@
	$(AWK) -f $(BENCH_PPROC) $(R_BENCH)/$</$(DS_RI_SUF) > $(R_BENCH)/$</$(DS_RI_PP_SUF)

$(DS_BENCHES_S_I): $(R_BENCH)/%/$(MACH_NAME)_$(TPHSI)_$(BENCH_SIZE): %
	$(DTRACE) -c './$(DRV) $< $(BENCH_SIZE) intsrt seqinc' -s $(BENCH_GEN_THR_HEAP) -o $@
	$(AWK) -f $(BENCH_PPROC) $(R_BENCH)/$</$(DS_SI_SUF) > $(R_BENCH)/$</$(DS_SI_PP_SUF)

bench: $(DRV) $(SL_BENCH) $(DS_BENCHES) $(SL_BENCH_PP) $(DS_BENCHES_PP)

bench_plot: 
	$(NAWK) $(NAWK_ARGS) -f $(BENCH_PLOT)/gen.nawk
	$(BENCH_PLOT)/plot.ksh

$(FGDIR):
	mkdir $(FGDIR)
	mkdir $(FGDIR)/time

$(FG_OUT_SIR): $(FGDIR) $(DRV)
	$(DTRACE) $(DTRACE_FRAMES) -c '$(FG_CMD_SIR)' -n $(value FG_TIME_STACKS) -o $@

$(FG_OUT_SII): $(FGDIR) $(DRV)
	$(DTRACE) $(DTRACE_FRAMES) -c '$(FG_CMD_SII)' -n $(value FG_TIME_STACKS) -o $@

$(FG_OUT_SID): $(FGDIR) $(DRV)
	$(DTRACE) $(DTRACE_FRAMES) -c '$(FG_CMD_SID)' -n $(value FG_TIME_STACKS) -o $@

$(FG_OUT_OIR): $(FGDIR) $(DRV)
	$(DTRACE) $(DTRACE_FRAMES) -c '$(FG_CMD_OIR)' -n $(value FG_TIME_STACKS) -o $@

$(FG_OUT_OII): $(FGDIR) $(DRV)
	$(DTRACE) $(DTRACE_FRAMES) -c '$(FG_CMD_OII)' -n $(value FG_TIME_STACKS) -o $@

$(FG_OUT_OID): $(FGDIR) $(DRV)
	$(DTRACE) $(DTRACE_FRAMES) -c '$(FG_CMD_OID)' -n $(value FG_TIME_STACKS) -o $@

$(FG_SVG_SIR): $(FG_OUT_SIR)
	$(STACKCOLLAPSE) $< > temp_folded
	$(FLAMEGRAPH) temp_folded > $@
	rm temp_folded

$(FG_SVG_SII): $(FG_OUT_SII)
	$(STACKCOLLAPSE) $< > temp_folded
	$(FLAMEGRAPH) temp_folded > $@
	rm temp_folded

$(FG_SVG_SID): $(FG_OUT_SID)
	$(STACKCOLLAPSE) $< > temp_folded
	$(FLAMEGRAPH) temp_folded > $@
	rm temp_folded

$(FG_SVG_OIR): $(FG_OUT_OIR)
	$(STACKCOLLAPSE) $< > temp_folded
	$(FLAMEGRAPH) temp_folded > $@
	rm temp_folded

$(FG_SVG_OII): $(FG_OUT_OII)
	$(STACKCOLLAPSE) $< > temp_folded
	$(FLAMEGRAPH) temp_folded > $@
	rm temp_folded

$(FG_SVG_OID): $(FG_OUT_OID)
	$(STACKCOLLAPSE) $< > temp_folded
	$(FLAMEGRAPH) temp_folded > $@
	rm temp_folded



.PHONY: flamegraphs
flamegraphs: $(FG_SVGS)

clean_flamegraphs:
	rm -r $(FGDIR)

clean_bench:
	rm -r $(R_BENCH)

clean_plot:
	rm -r imgs
	rm -r rcode

check: $(PLISTS)
	echo CHECK DONE

clean:
	rm $(OBJECTS) $(SO)

clean_tests:
	rm -r $(R_TEST)

clean_check:
	rm $(PLISTS)

clean_drv:
	-rm $(DRV)
	-rm $(DRV_OBJECT)
	-rm $(DS_OBJECTS)
