# Locale Kit makefile for test apps

LIBS = -lbe -llocale
LIBPATHS = -L../lib
CFLAGS = -I../include


all:	localeTest collatorTest

clean:
	rm -rf localeTest collatorTest

localeTest:	localeTest.cpp
	gcc localeTest.cpp $(CFLAGS) $(LIBPATHS) $(LIBS)

collatorTest:	collatorTest.cpp
	gcc collatorTest.cpp $(CFLAGS) $(LIBPATHS) $(LIBS)

