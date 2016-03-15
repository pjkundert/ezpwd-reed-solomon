
SHELL		= /bin/bash

# Compilers
# 
#    Defaults to system C/C++; define CXX to use a specific C++ compiler.  Supported across:
# 
# g++    4.8 - 5.1	-- Recommended; fastest
# clang  3.6		-- Recommended
# icc			-- Not recommended; much slower than g++ for ezpwd::rs
# 
CC		= cc  # clang   # gcc-4.8   # gcc # gcc-5 gcc-4.9 gcc-4.8 clang
CXX		= c++ # clang++ # g++-4.8   # g++ # g++-5 g++-4.9 g++-4.8 clang++

CXXFLAGS       += -I./c++ -std=c++11								\
		    -Wall -Wextra -pedantic -Wno-missing-braces -Wwrite-strings			\
		    -Wpointer-arith -Wnon-virtual-dtor -Woverloaded-virtual			\
		    -Wsign-promo -Wswitch -Wreturn-type	
CXXFLAGS       += -O3

# Debugging
#
# -D_GLIBCXX_DEBUG	-- Enables the native GNU C++ standard library "debug" containers.
#     This include std::array bounds checking on all accesses (on GNU libstdc++ systems only)
# -fsanitize=undefined	-- Additional compiler-based array bounds checking, etc.
# -DDEBUG=0,1,2,3	-- Additional Reed-Solomon sanity checking and extensive logging
# -DEZPWD_ARRAY_TEST	-- Intentional ERRONEOUS declarations of some R-S array extents.
# -DEZPWD_NO_MOD_TAB	-- Do not use table-based accelerated R-S module implementation.
# 
CXXFLAGS       +=#-D_GLIBCXX_DEBUG # -fsanitize=undefined # -g
CXXFLAGS       +=#-DDEBUG=2 #-DEZPWD_ARRAY_TEST -DEZPWD_NO_MOD_TAB

# C compiler/flags for sub-projects (phil-karn)
# Default to system cc; define CC to use a specific C compiler
#CC		= gcc # clang
CFLAGS		= # -O3 already defined

EMSDK		= ./emscripten/emsdk_portable
EMSDK_ACTIVATE	= ./emscripten/emsdk_portable/emsdk activate latest

EMSDK_EMXX 	= pushd $(EMSDK) && source ./emsdk_env.sh && popd && PATH=`pwd`/emscripten:$${PATH} && em++
DOCKER_EMXX	= docker run -v \$( shell pwd ):/mnt/test cmfatih/emscripten /srv/var/emscripten/em++ -I/mnt/test
CHEERP_EMXX	= /opt/cheerp/bin/clang++

# Emscripten
EMXX		= $(EMSDK_EMXX)
EMXX_ACTIVATE	= $(EMSDK_ACTIVATE)
EMXXFLAGS	= --memory-init-file 0 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 #-s ASSERTIONS=2

EMXX_EXPORTS_EZCOD = -s EXPORTED_FUNCTIONS="[			\
			'_ezcod_3_10_encode',			\
			'_ezcod_3_10_decode',			\
			'_ezcod_3_11_encode',			\
			'_ezcod_3_11_decode',			\
			'_ezcod_3_12_encode',			\
			'_ezcod_3_12_decode',			\
			'_free' ]"
EMXX_EXPORTS_RSPWD = -s EXPORTED_FUNCTIONS="[			\
			'_rspwd_encode_1',			\
			'_rspwd_encode_2',			\
			'_rspwd_encode_3',			\
			'_rspwd_encode_4',			\
			'_rspwd_encode_5',			\
			'_free' ]"
EMXX_EXPORTS_RSKEY = -s EXPORTED_FUNCTIONS="[			\
			'_rskey_2_encode',			\
			'_rskey_2_decode',			\
			'_rskey_3_encode',			\
			'_rskey_3_decode',			\
			'_rskey_4_encode',			\
			'_rskey_4_decode',			\
			'_rskey_5_encode',			\
			'_rskey_5_decode',			\
			'_free' ]"
EMXX_EXPORTS_MAIN  = -s EXPORTED_FUNCTIONS="[ '_main' ]"



EMSDK_URL	= https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz


# # Cheerp
# EMXX		= $(CHEERP_EMXX)
# EMXX_ACTIVATE	=
# EMXXFLAGS	= -fcxx-exceptions -target cheerp -D__cheerp -U__STRICT_ANSI__
# EMXX_EXPORTS_MAIN=
# EMXX_EXPORTS_RSPWD=
# EMXX_EXPORTS_EZCOD=

JSPROD =	js/ezpwd/ezcod.js				\
		js/ezpwd/rspwd.js				\
		js/ezpwd/rskey.js


JSCOMP =	rsexample.js					\
		rssimple.js					\
		rsembedded.js					\
		rsexercise.js					\
		rspwd_test.js					\
		ezcod_test.js					\
		rskey_test.js

JSTEST =	$(JSCOMP) rskey_node.js

EXCOMP =	rsencode rsdecode				\
		rsexample					\
		rssimple					\
		rsembedded					\
		rsembedded_nexc					\
		rsexercise					\
		rscompare					\
		rscompare_nexc					\
		rsvalidate					\
		rspwd_test					\
		ezcod_test					\
		rskey_test

EXTEST =	$(EXCOMP)

help:
	@echo  "EZPWD Reed-Solomon GNU 'make' targets"
	@echo  "  all			-- Javascript production targets: js/ezpwd/*.js"
	@echo  "  test			-- C++ (executable) and Javascript (Node.JS) tests"
	@echo  "  testex		-- C++ (executable) tests (with timing, by default)"
	@echo  "     ...-valgrind	-- C++ (executable) tests (with valgrind)"
	@echo  "  testjs		-- Javascript (Node.JS) tests"
	@echo  "  swig-python-install	-- Build and install Python bindings ezpwd_reed_solomon.* (via Swig)"

all:		$(JSPROD)

test:		testex testjs

javascript:	$(JSTEST) $(JSPROD)
executable:	$(EXTEST)

swig-python:	swig-python-install
swig-python-%:
	make -C swig/python $*

valgrind:	testex-valgrind\ -v\ --leak-check=full

testex:		testex-time
testex-%:	$(EXTEST)
	@for t in $^; do 					\
	    echo "$* ./$$t...";					\
	    $* ./$$t </dev/null;				\
	done

testjs:		testjs-node
testjs-%:	$(JSTEST)
	@for t in $^; do 					\
	    echo "$* ./$$t...";					\
	    $* ./$$t </dev/null;				\
	done

COPYRIGHT:	VERSION
	echo "/*! v$$( cat $< ) | (c) 2014 Hard Consulting Corporation | https://github.com/pjkundert/ezpwd-reed-solomon/blob/master/LICENSE */"\
		> $@

# 
# Production Javascript targets
# 
js/ezpwd/rspwd.js: rspwd.C rspwd.h COPYRIGHT rspwd_wrap.js			\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector		\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_RSPWD)			\
		--post-js rspwd_wrap.js $< -o $@				\
	  && cat COPYRIGHT $@ > $@.tmp && mv $@.tmp $@

js/ezpwd/rskey.js: rskey.C rskey.h COPYRIGHT rskey_wrap.js			\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector		\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_RSKEY)			\
		--post-js rskey_wrap.js $< -o $@				\
	  && cat COPYRIGHT $@ > $@.tmp && mv $@.tmp $@

ezcod.o:	ezcod.C ezcod.h c++/ezpwd/ezcod					\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector
js/ezpwd/ezcod.js: ezcod.C ezcod.h COPYRIGHT ezcod_wrap.js c++/ezpwd/ezcod	\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector		\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_EZCOD)			\
		--post-js ezcod_wrap.js $< -o $@				\
	  && cat COPYRIGHT $@ > $@.tmp && mv $@.tmp $@

clean:
	rm -f $(EXCOMP) $(EXCOMP:=.o)						\
	      $(JSCOMP) $(JSCOMP:=.mem)						\
	      ezcod.o
	make -C phil-karn clean

rspwd_test.js:	rspwd_test.C rspwd.C						\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector		\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

rsencode.o:	rsencode.C c++/ezpwd/rs
rsencode:	rsencode.o
	$(CXX) $(CXXFLAGS) -o $@ $^

rsdecode: 	CXXFLAGS += -DRSDECODE
rsdecode.o:	rsencode.C c++/ezpwd/rs
	$(CXX) $(CXXFLAGS) -c -o $@ $<
rsdecode:	rsdecode.o
	$(CXX) $(CXXFLAGS) -o $@ $^


rsexample.o:	rsexample.C c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector
rsexample:	rsexample.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexample.js:	rsexample.C c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

rssimple.o:	rssimple.C c++/ezpwd/rs
rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rssimple.js:	rssimple.C c++/ezpwd/rs						\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

rsembedded_nexc: 	CXXFLAGS += -DEZPWD_NO_EXCEPTS -fno-exceptions -Os
rsembedded_nexc.o:	rsembedded.C c++/ezpwd/rs
	$(CXX) $(CXXFLAGS) -c -o $@ $<
rsembedded_nexc:	rsembedded_nexc.o
	$(CXX) $(CXXFLAGS) -o $@ $^

rsembedded.o:	rsembedded.C c++/ezpwd/rs
rsembedded:	rsembedded.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsembedded.js:	rsembedded.C c++/ezpwd/rs					\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

rsexercise.o:	rsexercise.C exercise.H c++/ezpwd/rs
rsexercise:	rsexercise.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexercise.js:	rsexercise.C exercise.H c++/ezpwd/rs				\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

rscompare_nexc:		CXXFLAGS += -DEZPWD_NO_EXCEPTS -fno-exceptions -Os
rscompare_nexc.o:	rscompare.C c++/ezpwd/rs phil-karn/fec/rs-common.h schifra
	$(CXX) $(CXXFLAGS) -c -o $@ $<
rscompare_nexc: 	CXXFLAGS += -I./phil-karn
rscompare_nexc:	rscompare_nexc.o phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rscompare.o:	rscompare.C c++/ezpwd/rs phil-karn/fec/rs-common.h schifra
rscompare: CXXFLAGS += -I./phil-karn
rscompare:	rscompare.o phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rsvalidate.o:	rsvalidate.C c++/ezpwd/rs phil-karn/fec/rs-common.h
rsvalidate: CXXFLAGS += -I./phil-karn -ftemplate-depth=1000
rsvalidate:	rsvalidate.o phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rspwd_test.o:	rspwd_test.C rspwd.C c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector
rspwd_test:	rspwd_test.o
	$(CXX) $(CXXFLAGS) -o $@ $^

ezcod_test.o:	ezcod_test.C ezcod.C ezcod.h						\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector c++/ezpwd/ezcod
ezcod_test.o: CXXFLAGS += -I./phil-karn           # if DEBUG set, include phil-karn/
ezcod_test:	ezcod_test.o ezcod.o  phil-karn/librs.a # if DEBUG set, link w/ phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^
ezcod_test.js: CXXFLAGS += -I./phil-karn           # if DEBUG set, include phil-karn/
ezcod_test.js: ezcod_test.C ezcod.C ezcod.h						\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector c++/ezpwd/ezcod	\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< ezcod.C -o $@

rskey_test.o:	rskey_test.C rskey.C rskey.h c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector
rskey_test:	rskey_test.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rskey_test.js:	rskey_test.C rskey.C rskey.h c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector \
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

# 
# Build Phil Karn's R-S implementation.  Used by some tests.
# 
#     Any tests using Phil Karn's library aren't yet convertible to Javascript
# 
phil-karn/fec/rs-common.h \
phil-karn/librs.a:
	CFLAGS=$(CFLAGS) CC=$(CC) make -C phil-karn all

# 
# Build Schifra R-S implementation.  Used by some tests.
# 
# schifra:	schifra.tgz
#	tar xzf $<
#	make -C schifra all
#
# schifra.tgz:
#	wget -O $@ http://www.schifra.com/downloads/schifra.tgz
#

schifra:
	git clone https://github.com/ArashPartow/schifra.git

# 
# Install and build emscripten SDK, if necessary, and then activate it.
# 
#    Presently only works on OS-X as far as I know. Should use a Docker instance.
# 
emscripten:	emscripten/python2 				\
		emscripten/emsdk_portable/emscripten		\
		FORCE
	$(EMXX_ACTIVATE)

emscripten/python2:
	mkdir -p emscripten
	ln -fs $$( which python2.7 ) emscripten/python2

emscripten/emsdk_portable/emscripten: emscripten/emsdk_portable
	cd $< && ./emsdk update && ./emsdk install latest
	touch $@

emscripten/emsdk_portable: emscripten/emsdk-portable.tar.gz
	mkdir -p emscripten
	tar -C emscripten -xzf $<
	touch $@

emscripten/emsdk-portable.tar.gz:
	mkdir -p emscripten
	wget -O $@ $(EMSDK_URL)

FORCE:
