SHELL		= /bin/bash
BINDINGS	= ezcod

all:
	@echo "Python Bindings: $(BINDINGS)"
	@echo "  test     -- test bindings"
	@echo "  install  -- install ezpwd_reed_solomon w/ all bindings"

test:			FORCE
	@for binding in $(BINDINGS); do				\
	    echo "Testing binding: $$binding";			\
	    make -C $$binding python-$$binding-test;		\
	done

install:		version.py FORCE
	python setup.py install

version.py:		../../VERSION
	@python -c 'print "__version_info__\t\t= (%s)" % ", ".join( open( "'$<'", "r" ).read().strip().split( "." ))' > $@
	@python -c 'print "__version__\t\t\t= \".\".join( map( str, __version_info__ ))"' >> $@

FORCE:
