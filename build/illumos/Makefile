include ../Makefile.master

PREFIX=			$(ILLUMOS_PREFIX)
DSLDFLAGS=		$(DSLDF_ILLUMOS)
LIBS+=			-lumem
DSLIBS+=		-lumem -luutil
CFLAGS+=		-D UMEM
DSCFLAGS+=		-D UUTIL -D UMEM
UMEM_CFLAGS=		$(CFLAGS) "-Wno-unused-parameter"
LDFLAGS+=		-h libslablist.so.1

SL_PROV=	slablist_provider.d
DS_PROV=	struc_provider.d

OBJECTS=	$(C_OBJECTS) $(D_OBJECTS)


$(D_HDRS): 
	$(DTRACEH) $(SLDIR)/$(D_SCR) -o $(SLDIR)/$(D_HDRS)

#
# This is used for debugging (like expanding variables), because Make sucks at
# error messages.
#
fuckit: 
	echo $(TEST_RES)
	echo $(TEST_SCR)

.PHONY: $(DS_IMPLS)

.PHONY: $(DS_IMPLS_EXT)

#.PHONY: $(C_SRCS)

$(C_SRCS): %.c: $(D_HDRS)

$(C_OBJECTS): %.o: %.c $(C_HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<
	$(CTFC_POSTPROC)
#$(CTFCONVERT) -i -L VERSION $@

objs: $(OBJECTS)

$(D_OBJECTS): $(C_OBJECTS)
	$(DTRACEG) $(SLDIR)/$(SL_PROV) $(C_OBJECTS)

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
	pfexec rm -r $(PREFIX) 2> /dev/null

install: 
	-pfexec rm -r $(PREFIX) 2> /dev/null
	pfexec mkdir $(PREFIX)
	pfexec mkdir $(PREFIX)/lib/
	pfexec mkdir $(PREFIX)lib/64
	pfexec cp $(SO) $(PREFIX)/lib/64/
	pfexec ln -s $(PREFIX)/lib/64/$(SO) $(PREFIX)/lib/64/libslablist.so
	pfexec mkdir $(PREFIX)/include
	pfexec mkdir $(PREFIX)/include/dtrace
	pfexec cp $(SLDIR)/slablist.h $(PREFIX)/include/
	pfexec cp $(SLDIR)/slablist.d $(PREFIX)/include/dtrace/

$(DS_D_HDRS):
	$(DTRACEH) $(DSDIR)/struc_provider.d -o $(DSDIR)/struc_provider.h

$(DS_SRCS): %.c: $(DS_D_HDRS)

$(DRV_SRCS): %.c: $(DS_D_HDRS)



$(DS_OBJECTS): %.o: %.c
	$(CC) $(DSCFLAGS) -o $@ -c $<


$(DRV_OBJECT): %.o: %.c $(DS_OBJECTS)
	$(CC) $(DSCFLAGS) $(DSCINC) -o $@ -c $(DRV_SRCS)

$(DS_D_OBJECTS): $(DRV_OBJECT)
	$(DTRACEG) $(DSDIR)/$(DS_PROV) $(DRV_OBJECT)

$(DRV): install $(DRV_OBJECT) $(DS_D_OBJECTS) $(DS_OBJECTS)
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

$(DS_BENCHES_R_I_FR): $(R_BENCH)/%/$(DS_RI_FR_SUF): %
	$(DTRACE) -c './$(DRV) $< $(BENCH_SIZE) intsrt rand foldr' -s $(BENCH_FOLDR) -o $@

$(DS_BENCHES_S_I_FR): $(R_BENCH)/%/$(DS_SI_FR_SUF): %
	$(DTRACE) -c './$(DRV) $< $(BENCH_SIZE) intsrt seqinc foldr' -s $(BENCH_FOLDR) -o $@

$(DS_BENCHES_R_I_FL): $(R_BENCH)/%/$(DS_RI_FL_SUF): %
	$(DTRACE) -c './$(DRV) $< $(BENCH_SIZE) intsrt rand foldl' -s $(BENCH_FOLDL) -o $@

$(DS_BENCHES_S_I_FL): $(R_BENCH)/%/$(DS_SI_FL_SUF): %
	$(DTRACE) -c './$(DRV) $< $(BENCH_SIZE) intsrt seqinc foldl' -s $(BENCH_FOLDL) -o $@

$(SL_RI_FR): $(DRV) $(R_BENCH_SD)
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt rand foldr' -s $(BENCH_FOLDR) -o $@

$(SL_SI_FR): $(DRV) $(R_BENCH_SD)
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt seqinc foldr' -s $(BENCH_FOLDR) -o $@

$(SL_RI_FL): $(DRV) $(R_BENCH_SD)
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt rand foldl' -s $(BENCH_FOLDL) -o $@

$(SL_SI_FL): $(DRV) $(R_BENCH_SD)
	$(DTRACE) -c './$(DRV) sl $(BENCH_SIZE) intsrt seqinc foldl' -s $(BENCH_FOLDL) -o $@

bench: $(DRV) $(SL_BENCH) $(DS_BENCHES) $(SL_BENCH_PP) $(DS_BENCHES_PP) $(DS_BENCHES_F) $(SL_BENCH_F)

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
	pfexec rm -r $(FGDIR)

clean_bench:
	pfexec rm -r $(R_BENCH)

clean_plot:
	pfexec rm -r imgs
	pfexec rm -r rcode

check: $(PLISTS)
	echo CHECK DONE

clean:
	rm $(OBJECTS) $(SO)

clean_tests:
	pfexec rm -r $(R_TEST)

clean_check:
	rm $(PLISTS)

clean_drv:
	-rm $(DRV)
	-rm $(DS_D_OBJECTS)
	-rm $(DRV_OBJECT)
	-rm $(DS_OBJECTS)
