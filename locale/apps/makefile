# Locale Kit makefile for test apps

LIBS = -lbe -llocale
LIBPATHS = -L../lib
INCPATHS = -I../include

ifeq ($(DEBUG_BUILD), true)
	CFLAGS = -g -DDEBUG=1
else
	CFLAGS = -O1
endif


default: all

all:	collectcatkeys

clean:
	rm -rf initlocale

collectcatkeys:	collectcatkeys.cpp collectcatkeys.rsrc
	gcc collectcatkeys.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)
	xres -o collectcatkeys collectcatkeys.rsrc
	mimeset -f collectcatkeys
