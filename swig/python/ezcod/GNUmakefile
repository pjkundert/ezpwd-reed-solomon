ROOT		= ../../..

include $(ROOT)/GNUmakefile

# 
# Swig interfaces
# 
# Find Python.h
# 
# On Mac:
#   /usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/include/python2.7/Python.h
#   $ python -e "import os; print os.__file__"
#   /usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/python2.7/os.pyc
#  - Library:
#   /usr/local//Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config/libpython2.7.a
# On Linux:
#   /usr/include/python2.7/Python.h
#   $ python -e "import os; print os.__file__"
#   /usr/lib/python2.7/os.pyc
# 
PYTHON_INC	= $(shell python -c 'import os; segs=os.__file__.split( os.path.sep ); print os.path.sep.join( segs[:-3]+["include"]+segs[-2:-1] )' )
PYTHON_LIB	= $(shell python -c 'import os; segs=os.__file__.split( os.path.sep ); print os.path.sep.join( segs[:-1]+["config"] )' )

PYTHON_COMP	= -I$(PYTHON_INC) -I$(ROOT)/c++ -fPIC
PYTHON_LINK	= -L$(PYTHON_LIB) -lpython2.7 -dynamiclib

ezcod-test:		_ezcod.so

ezcod-test:		runme.py ezcod-test ezcod_python
	./ezcod_python
	python $<

ezcod-clean:
	rm -f *.cxx *.o *.so *.pyc ezcod.py ezcod_python

ezcod_wrap.cxx:		ezcod.i
	swig -verbose -c++ -I$(ROOT)/c++ -python $<

ezcod_wrap.o:		ezcod_wrap.cxx
	$(CXX) $(CXXFLAGS) $(PYTHON_COMP) -c $<

ezcod_python:		ezcod_python.C ezcod_python.h
	$(CXX) $(CXXFLAGS) -I$(ROOT)/c++ $< -o $@

_ezcod.so:		ezcod_wrap.o
	$(CXX) $(CXXFLAGS) $(PYTHON_LINK) -o $@ $<
