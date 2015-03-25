SHELL		= /bin/bash
BINDINGS	= ezcod

all:			version.py FORCE
	@for binding in $(BINDINGS); do				\
	    echo "Building binding: $$binding";			\
	    make -C $$binding python-$$binding;			\
	done

version.py:		../../VERSION
	@python -c 'print "__version_info__\t\t= (%s)" % ", ".join( open( "'$<'", "r" ).read().strip().split( "." ))' > $@
	@echo "__version__\t\t\t= '.'.join( map( str, __version_info__ ))" >> $@

FORCE:
