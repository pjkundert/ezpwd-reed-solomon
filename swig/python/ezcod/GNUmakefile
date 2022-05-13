# 
# Development build of ezcod Python wrapper
# - Production build is done using setuptools (see: ../setup.py)
# 
ROOT		= ../../..

include $(ROOT)/GNUmakefile

# 
# Swig interfaces.  Requires Swig >= 3.0.5
# 
# Find Python.h, libraries
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
PYTHON_INC	= $(shell python3 -c 'import os; segs=os.__file__.split( os.path.sep ); print( os.path.sep.join( segs[:-3]+["include"]+segs[-2:-1] ))' )
PYTHON_LIB	= $(shell python3 -c 'import os, glob; segs=os.__file__.split( os.path.sep ); print( glob.glob( os.path.sep.join( segs[:-1]+["config*/"] ))[0] )' )
#PYTHON_INC	= $(shell python -c 'import os; segs=os.__file__.split( os.path.sep ); print os.path.sep.join( segs[:-3]+["include"]+segs[-2:-1] )' )
#PYTHON_LIB	= $(shell python -c 'import os; segs=os.__file__.split( os.path.sep ); print os.path.sep.join( segs[:-1]+["config"] )' )

PYTHON_COMP	= -I$(PYTHON_INC) -fPIC $(INCLUDE)
PYTHON_LINK	= -L$(PYTHON_LIB) -lpython3.10 -shared

python-ezcod:		_ezcod.so

python-ezcod-test:	runme.py python-ezcod
	python3 $<

python-ezcod-clean:
	rm -f *.cpp *.o *.so *.pyc

# Forces re-build of Swig-generated code (if desired)
python-ezcod-pristine: python-ezcod-clean
	rm -f ezcod_wrap.cpp ezcod.py

ezcod_wrap.cpp:		ezcod.i
	swig -verbose -c++ $(INCLUDE) -python -py3 -o $@ $<

ezcod_wrap.o:		ezcod_wrap.cpp
	$(CXX) $(CXXFLAGS) $(PYTHON_COMP) -c $<

_ezcod.so:		ezcod_wrap.o
	$(CXX) $(CXXFLAGS) $(PYTHON_LINK) -o $@ $<
