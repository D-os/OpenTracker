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

all:	localeTest collatorTest collatorSpeed catalogTest catalogTestAddOn \
		catalogSpeed

clean:
	rm -rf localeTest collatorTest collatorSpeed catalogTest catalogTestAddOn \
			 catalogSpeed

localeTest:	localeTest.cpp
	gcc localeTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

collatorTest:	collatorTest.cpp
	gcc collatorTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

collatorSpeed:	collatorSpeed.cpp
	gcc collatorSpeed.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

catalogTest:	catalogTest.cpp
	gcc catalogTest.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

catalogTestAddOn:	catalogTestAddOn.cpp
	gcc catalogTestAddOn.cpp $(CFLAGS) -nostart -Xlinker -soname=catalogTestAddOn $(INCPATHS) $(LIBPATHS) $(LIBS)

catalogSpeed:	catalogSpeed.cpp
	gcc catalogSpeed.cpp $(CFLAGS) $(INCPATHS) $(LIBPATHS) $(LIBS)

