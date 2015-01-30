
SHELL		= /bin/bash
CXXFLAGS       += -I./c++ -std=c++11 -O3							\
		    -Wall -Wextra -pedantic -Wno-missing-braces -Wwrite-strings -Wcast-align	\
		    -Wpointer-arith -Wcast-qual -Wnon-virtual-dtor -Woverloaded-virtual		\
		    -Wsign-promo -Wswitch -Wreturn-type	
CXXFLAGS       +=#-DDEBUG=2 #-DEZPWD_ARRAY_SAFE #-DEZPWD_ARRAY_TEST -DEZPWD_NO_MOD_TAB
#CXX		= clang++
CXX		= g++

EMSDK		= ./emscripten/emsdk_portable
EMSDK_ACTIVATE	= ./emscripten/emsdk_portable/emsdk activate latest

EMSDK_EMXX 	= pushd $(EMSDK) && source ./emsdk_env.sh && popd && PATH=`pwd`/emscripten:$${PATH} && em++
DOCKER_EMXX	= docker run -v \$( shell pwd ):/mnt/test cmfatih/emscripten /srv/var/emscripten/em++ -I/mnt/test
CHEERP_EMXX	= /opt/cheerp/bin/clang++

# Emscripten
EMXX		= $(EMSDK_EMXX)
EMXX_ACTIVATE	= $(EMSDK_ACTIVATE)
EMXXFLAGS	= --memory-init-file 0 -s DISABLE_EXCEPTION_CATCHING=0 # -s ASSERTIONS=2

EMXX_EXPORTS_EZCOD = -s EXPORTED_FUNCTIONS="[			\
			'_ezcod_3_10_encode',			\
			'_ezcod_3_10_decode',			\
			'_ezcod_3_11_encode',			\
			'_ezcod_3_11_decode',			\
			'_ezcod_3_12_encode',			\
			'_ezcod_3_12_decode',			\
			'_malloc', '_free' ]"
EMXX_EXPORTS_RSPWD = -s EXPORTED_FUNCTIONS="[			\
			'_rspwd_encode_1',			\
			'_rspwd_encode_2',			\
			'_rspwd_encode_3',			\
			'_rspwd_encode_4',			\
			'_rspwd_encode_5' ]"
EMXX_EXPORTS_MAIN  = -s EXPORTED_FUNCTIONS="[ '_main' ]"



EMSDK_URL	= https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz


# # Cheerp
# EMXX		= $(CHEERP_EMXX)
# EMXX_ACTIVATE	=
# EMXXFLAGS	= -fcxx-exceptions -target cheerp -D__cheerp -U__STRICT_ANSI__
# EMXX_EXPORTS_MAIN=
# EMXX_EXPORTS_RSPWD=
# EMXX_EXPORTS_EZCOD=

all:		js

test:		testbin testjs

js:		jsprod jstest

jsprod:		js/ezpwd/rspwd.js				\
		js/ezpwd/ezcod.js

jstest:		rsexample.js					\
		rssimple.js					\
		rsexercise.js					\
		rspwd_test.js					\
		ezcod_test.js					\
		rskey_test.js

bintest:	rsexample					\
		rssimple					\
		rsexercise					\
		rscompare					\
		rsvalidate					\
		rspwd_test					\
		ezcod_test					\
		rskey_test

testbin:	bintest
	./rsexample; ./rssimple; ./rsexercise; ./rscompare; ./rsvalidate; ./rspwd_test; ./ezcod_test; ./rskey_test

testjs:		jstest
	node ./rsexample.js
	node ./rssimple.js
	node ./rsexercise.js
	node ./rspwd_test.js
	node ./ezcod_test.js
	node ./rskey_test.js

rspwd_test.js:	rspwd_test.C rspwd.C				\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector \
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

js/ezpwd/rspwd.js: rspwd.C					\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector \
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_RSPWD) $< -o $@ 


ezcod.o:	ezcod.C ezcod.h c++/ezpwd/ezcod c++/ezpwd/rs
ezcod:		ezcod.o
	$(CXX) $(CXXFLAGS) -o $@ $^
js/ezpwd/ezcod.js: ezcod.C ezcod.h c++/ezpwd/ezcod		\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector \
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_EZCOD) $< -o $@ 

clean:
	rm -f rsexample		rsexample.o	rsexample.js	rsexample.js.mem	\
	      rssimple		rssimple.o	rssimple.js	rssimple.js.mem		\
	      rsexercise	rsexercise.o	rsexercise.js	rsexercise.js.mem	\
	      rspwd_test	rspwd_test.o	rspwd_test.js	rspwd_test.js.mem	\
	      rskey_test	rskey_test.o	rskey_test.js	rskey_test.js.mem	\
	      rscompare		rscompare.o						\
	      rsvalidate	rsvalidate.o						\
	      ezcod		ezcod.o		ezcod.js	ezcod.js.mem		\
	      ezcod_test	ezcod_test.o	ezcod_test.js	ezcod_test.js.mem
	make -C phil-karn clean

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

rsexercise.o:	rsexercise.C exercise.H c++/ezpwd/rs
rsexercise:	rsexercise.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexercise.js:	rsexercise.C exercise.H c++/ezpwd/rs				\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

rscompare.o:	rscompare.C c++/ezpwd/rs phil-karn/fec/rs-common.h
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

ezcod_test.o:	ezcod_test.C ezcod.C ezcod.h c++/ezpwd/ezcod c++/ezpwd/rs
ezcod_test.o: CXXFLAGS += -I./phil-karn           # if DEBUG set, include phil-karn/
ezcod_test:	ezcod_test.o ezcod.o  phil-karn/librs.a # if DEBUG set, link w/ phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^
ezcod_test.js: CXXFLAGS += -I./phil-karn           # if DEBUG set, include phil-karn/
ezcod_test.js: ezcod_test.C ezcod.C c++/ezpwd/rs				\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< ezcod.C -o $@

rskey_test.o:	rskey_test.C c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector
rskey_test:	rskey_test.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rskey_test.js:	rskey_test.C c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector \
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 
# 
# Build Phil Karn's R-S implementation.  Used by some tests.
# 
#     Any tests using Phil Karn's library aren't yet convertible to Javascript
# 
phil-karn/fec/rs-common.h \
phil-karn/librs.a:
	make -C phil-karn all

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
