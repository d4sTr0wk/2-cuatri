AR=ar

LIBDIR=../lib

CFLAGS= -O3 -fPIC

# not targets
.PHONY: shared static clean

shared: $(LIBDIR)/libnaivelib.so

static: $(LIBDIR)/libnaivelib.a

# compile into a shared library
$(LIBDIR)/libnaivelib.so: naivelib.o
	$(CC) -shared -o $@ $< $(CFLAGS)

# compile into a static library (several object files together into a file)
$(LIBDIR)/libnaivelib.a: naivelib.o
	$(AR) rcs $@ $<

# object file
naivelib.o: naive.c naive.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean: 
	rm -f naivelib.o
	rm -f $(LIBDIR)/libnaivelib.so $(LIBDIR)/libnaivelib.a
