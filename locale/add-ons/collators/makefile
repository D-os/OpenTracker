# makefile for the Locale Kit Collator add-ons

SUBDIRS = \
	deutsch \

default .DEFAULT :
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@; \
	done
