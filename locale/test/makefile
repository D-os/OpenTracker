# Locale Kit makefile for test apps

LIBS = -lbe -llocale
LIBPATHS = -L../lib
INCPATHS = -I../include
CFLAGS = -g # -fcheck-memory-usage -D_NO_INLINE_ASM


default: all

all:	localeTest collatorTest collatorSpeed catalogTest catalogSpeed

clean:
	rm -rf localeTest collatorTest collatorSpeed catalogTest

localeTest:	localeTest.cpp
	gcc localeTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

collatorTest:	collatorTest.cpp
	gcc collatorTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

collatorSpeed:	collatorSpeed.cpp
	gcc collatorSpeed.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

catalogTest:	catalogTest.cpp
	gcc catalogTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

catalogSpeed:	catalogSpeed.cpp
	gcc catalogSpeed.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

