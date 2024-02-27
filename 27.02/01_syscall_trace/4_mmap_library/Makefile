# Makefile to compile an application using a library with its corresponding
# source directories.
# Targets 'shared' & 'static' generate both the app (runme_shared) compiled
# using the shared library (libnaive.so) and the one compiled using the static
# library (libnaive.a) called runme_static

BINDIR=./bin
LIBDIR=./lib
INCDIR=./include
SRCLIBDIR=./naivelib

EXEC=runme
SRC=app.c

CFLAGS= -O3 -I $(INCDIR)
LDFLAGS= -L $(LIBDIR) 


.PHONY: shared static build_lib_sh build_lib_st clean # not targets

shared: build_lib_shared $(BINDIR)/$(EXEC)_shared

static: build_lib_static $(BINDIR)/$(EXEC)_static

build_lib_shared:
	make -C $(SRCLIBDIR) shared
	@echo "\nexport LD_LIBRARY_PATH=<LIB_DIR>\n"

build_lib_static:
	make -C $(SRCLIBDIR) static

$(BINDIR)/$(EXEC)_shared: $(SRC) $(LIBDIR)/libnaivelib.so
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -lnaivelib

$(BINDIR)/$(EXEC)_static: $(SRC) $(LIBDIR)/libnaivelib.a
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -lnaivelib -static

# clean executables
clean: 
	rm -f $(BINDIR)/$(EXEC)_*

# clean execs & libraries
vclean: clean
	make -C $(SRCLIBDIR) clean

