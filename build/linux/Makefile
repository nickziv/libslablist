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



check: $(PLISTS)
	echo CHECK DONE

clean:
	rm $(OBJECTS) $(SO)

clean_check:
	rm $(PLISTS)

clean_drv:
	-rm $(DRV)
	-rm $(DRV_OBJECT)
	-rm $(DS_OBJECTS)
