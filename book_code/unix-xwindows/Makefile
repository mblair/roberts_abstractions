# Makefile for cslib/xwindows directory
# Last modified on Sat Oct  1 12:56:29 1994 by eroberts
#****************************************************************

OBJECTS = \
    genlib.o \
    exception.o \
    strlib.o \
    simpio.o \
    random.o \
    graphics.o \
    xmanager.o \
    xdisplay.o \
    xcompat.o \
    glibrary.o

CSLIB = cslib.a

CC = gcc
CFLAGS = -g -I. $(CCFLAGS)
CONFIG = `csh config.csh -DHasPoll $(CC) -E -DHasPoll xcompat.c`

# ***************************************************************
# Entry to bring the package up to date
#    The "make all" entry should be the first real entry

all: $(CSLIB) gccx

# ***************************************************************
# Standard entries to remove files from the directories
#    tidy    -- eliminate unwanted files
#    clean   -- delete derived files in preparation for rebuild
#    scratch -- synonym for clean

tidy:
	rm -f ,* .,* *~ core a.out *.err

clean scratch: tidy
	rm -f *.o *.a gccx

# ***************************************************************
# C compilations

genlib.o: genlib.c genlib.h exception.h gcalloc.h
	$(CC) $(CFLAGS) -c genlib.c

exception.o: exception.c exception.h genlib.h
	$(CC) $(CFLAGS) -c exception.c

strlib.o: strlib.c strlib.h genlib.h
	$(CC) $(CFLAGS) -c strlib.c

simpio.o: simpio.c simpio.h strlib.h genlib.h
	$(CC) $(CFLAGS) -c simpio.c

random.o: random.c random.h genlib.h
	$(CC) $(CFLAGS) -c random.c

graphics.o: graphics.c graphics.h extgraph.h xmanager.h xcompat.h \
            glibrary.h genlib.h gcalloc.h simpio.h strlib.h Makefile
	$(CC) $(CONFIG) $(CFLAGS) -c graphics.c

xmanager.o: xmanager.c xmanager.h xdisplay.h xcompat.h glibrary.h \
	    genlib.h exception.h simpio.h Makefile
	$(CC) $(CONFIG) $(CFLAGS) -c xmanager.c

xdisplay.o: xdisplay.c xdisplay.h xmanager.h glibrary.h genlib.h strlib.h \
            Makefile
	$(CC) $(CONFIG) $(CFLAGS) -c xdisplay.c

xcompat.o: xcompat.c xcompat.h Makefile
	$(CC) $(CONFIG) $(CFLAGS) -c xcompat.c

glibrary.o: glibrary.c glibrary.h genlib.h
	$(CC) $(CFLAGS) -c glibrary.c

# ***************************************************************
# Entry to reconstruct the library archive

$(CSLIB): $(OBJECTS)
	-rm -f $(CSLIB)
	ar cr $(CSLIB) $(OBJECTS)
	ranlib $(CSLIB)

# ***************************************************************
# Entry to reconstruct the gccx script

gccx: Makefile
	@echo '#! /bin/csh -f' > gccx
	@echo 'set INCLUDE =' `pwd` >> gccx
	@echo 'set CSLIB = $$INCLUDE/cslib.a' >> gccx
	@echo 'set LIBRARIES = ($$CSLIB -lX11 -lm)' >> gccx
	@echo 'foreach x ($$*)' >> gccx
	@echo '  if ("x$$x" == "x-c") then' >> gccx
	@echo '    set LIBRARIES = ""' >> gccx
	@echo '    break' >> gccx
	@echo '  endif' >> gccx
	@echo 'end' >> gccx
	@echo 'gcc -g -I$$INCLUDE $$* $$LIBRARIES' >> gccx
	@chmod a+x gccx
	@echo '[gccx script created]'
