# makefile for the Locale Kit tools

SUBDIRS = \
	genprops \

default .DEFAULT :
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@; \
	done
