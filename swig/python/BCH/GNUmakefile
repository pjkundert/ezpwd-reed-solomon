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

PYTHON_COMP	= -I$(PYTHON_INC) -fPIC $(INCLUDE_BCH)
PYTHON_LINK	= -L$(PYTHON_LIB) -lpython3.10 -shared $(ROOT)/djelic_bch.o

python-BCH:		_BCH.so

python-BCH-test:	runme.py python-BCH
	python3 $<

python-BCH-clean:
	rm -f *.cpp *.o *.so *.pyc

# Forces re-build of Swig-generated code (if desired)
python-BCH-pristine: python-BCH-clean
	rm -f BCH_wrap.cpp BCH.py

BCH_wrap.cpp:		BCH.i
	swig -verbose -c++ $(INCLUDE) $(INCLUDE_BCH) -python -py3 -o $@ $<

BCH_wrap.o:		BCH_wrap.cpp
	$(CXX) $(CXXFLAGS) $(PYTHON_COMP) $(INCLUDE_BCH) -c $<

_BCH.so:		BCH_wrap.o $(ROOT)/djelic_bch.o
	$(CXX) $(CXXFLAGS) $(PYTHON_LINK) -o $@ $<
