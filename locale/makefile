# makefile for the Locale Kit

SUBDIRS = \
	lib \
	add-ons \
	tools \
	languages \
	test

default .DEFAULT :
	-@for f in $(SUBDIRS) ; do \
		$(MAKE) -C $$f -f makefile $@; \
	done

zip backup tar:
	@zip -yr locale-kit-`date +%Y-%m-%d`.zip $(SUBDIRS) include/ languages/ makefile -x obj.x86 -x *.o
