SHELL		= bash
BINDINGS	= ezcod		\
		  BCH

all:
	@echo "Python Bindings: $(BINDINGS)"
	@echo "  install	-- install ezpwd_reed_solomon w/ all bindings"
	@echo "  test		-- test ezpwd_reed_solomon w/ all bindings"
	@echo "  binding-	-- Run on all bindings..."
	@echo "    clean	-- "
	@echo "    pristine	-- Force rebuild of binding"
	@echo "    test		-- Internal binding self-test"

binding-%:		FORCE
	@for binding in $(BINDINGS); do				\
	    echo "Running python-binding-$* on $$binding";	\
	    make -C $$binding python-$$binding-$*;		\
	done

clean:			binding-clean
	rm -rf build dist

pristine:		clean binding-pristine

test:			clean binding-test

install:		version.py FORCE
	python3 setup.py install

version.py:		../../VERSION
	@python -c 'print "__version_info__\t\t= (%s)" % ", ".join( open( "'$<'", "r" ).read().strip().split( "." ))' > $@
	@python -c 'print "__version__\t\t\t= \".\".join( map( str, __version_info__ ))"' >> $@

FORCE:
