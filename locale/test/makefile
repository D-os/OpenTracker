# Locale Kit makefile for test apps

LIBS = -lbe -llocale
LIBPATHS = -L../lib
INCPATHS = -I../include
CFLAGS = # -fcheck-memory-usage -D_NO_INLINE_ASM


all:	localeTest collatorTest

clean:
	rm -rf localeTest collatorTest

localeTest:	localeTest.cpp
	gcc localeTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

collatorTest:	collatorTest.cpp
	gcc collatorTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

