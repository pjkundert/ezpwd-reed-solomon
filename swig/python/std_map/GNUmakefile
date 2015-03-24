
PYTHON_INC	= $(shell python -c 'import os; segs=os.__file__.split( os.path.sep ); print os.path.sep.join( segs[:-3]+["include"]+segs[-2:-1] )' )
PYTHON_LIB	= $(shell python -c 'import os; segs=os.__file__.split( os.path.sep ); print os.path.sep.join( segs[:-1]+["config"] )' )

PYTHON_COMP	= -I$(PYTHON_INC) -fPIC
PYTHON_LINK	= -L$(PYTHON_LIB) -lpython2.7 -dynamiclib

std_map:		_example.so

std_map-test:		runme.py std_map
	python $<

std_map-clean:
	rm -f *.cxx *.o *.so *.pyc example.py

example_wrap.cxx:	example.i
	swig -verbose -c++ -python $<

example_wrap.o:		example_wrap.cxx
	$(CXX) $(CXXFLAGS) $(PYTHON_COMP) -c $<

_example.so:		example_wrap.o
	$(CXX) $(CXXFLAGS) $(PYTHON_LINK) -o $@ $<
