# makefile for the Locale Kit add-ons

SUBDIRS = \
	collators \

default .DEFAULT :
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@; \
	done
