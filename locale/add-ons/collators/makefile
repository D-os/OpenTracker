# makefile for the Locale Kit Collator add-ons

LIBS = -lbe -llocale
LIBPATHS = -L../../lib
INCPATHS = -I../../include
CFLAGS = -W -Wall -Wno-multichar -Wno-ctor-dtor-privacy -Woverloaded-virtual # -fcheck-memory-usage -D_NO_INLINE_ASM

COLLATOR_SOURCES = DeutschDIN-2.cpp Français.cpp


define COLLATOR_SOURCES_TO_COLLATORS
	$(addsuffix .collator, $(foreach file, $(COLLATOR_SOURCES), $(basename $(notdir $(file)))))
endef

COLLATORS = $(COLLATOR_SOURCES_TO_COLLATORS)


all:		$(COLLATORS)

clean:
	rm *.collator

install:	all
	copyattr -d $(COLLATORS) /boot/home/config/add-ons/locale/collators/

%.collator : %.cpp %.rsrc
	gcc -o $@ $*.cpp $(CFLAGS) -nostart -Xlinker -soname=$@ $(INCPATHS) $(LIBPATHS) -lbe -llocale
	xres -o $@ $*.rsrc
	mimeset -f $@
