# makefile for Tracker/Deskbar

SUBDIRS = \
	tracker \
	deskbar

all:
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile; \
	done

default .DEFAULT :
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@; \
	done
