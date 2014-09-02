
SHELL		= /bin/bash
CXXFLAGS       += -I./c++ -Wall -Wextra -Wpedantic -Wno-missing-braces -O3 -std=c++11 # -DDEBUG=1 -DEZPWD_ARRAY_SAFE #-DEZPWD_ARRAY_TEST -DDEBUG=2
CXX		= clang++ # g++ 4.9.0 fails rspwd-test at all optimization levels!

EMSDK		= ./emscripten/emsdk_portable
EMSDK_ACTIVATE	= ./emscripten/emsdk_portable/emsdk activate latest

EMSDK_EMXX 	= pushd $(EMSDK) && source ./emsdk_env.sh && popd && PATH=$${PATH}:`pwd`/emscripten && em++
DOCKER_EMXX	= docker run -v \$( shell pwd ):/mnt/test cmfatih/emscripten /srv/var/emscripten/em++ -I/mnt/test

EMXX		= $(EMSDK_EMXX)
EMXX_ACTIVATE	= $(EMSDK_ACTIVATE)
EMXXFLAGS	= -s DISABLE_EXCEPTION_CATCHING=0 # -s ASSERTIONS=2

EMXX_EXPORTS_EZCOD = "[	'_ezcod_3_10_encode',			\
			'_ezcod_3_10_decode',			\
			'_ezcod_3_11_encode',			\
			'_ezcod_3_11_decode',			\
			'_ezcod_3_12_encode',			\
			'_ezcod_3_12_decode',			\
			'_malloc', '_free' ]"
EMXX_EXPORTS_RSPWD = "[	'_rspwd_encode_1',			\
			'_rspwd_encode_2',			\
			'_rspwd_encode_3',			\
			'_rspwd_encode_4',			\
			'_rspwd_encode_5' ]"
EMXX_EXPORTS_MAIN  = "[ '_main' ]"

EMSDK_URL	= https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz


all:		js

test:		testbin testjs

js:		jsprod jstest

jsprod:		js/ezpwd/rspwd.js				\
		js/ezpwd/ezcod.js

jstest:		rsexample.js					\
		rssimple.js					\
		rsexercise.js					\
		rspwd-test.js					\
		ezcod_test.js

bintest:	rsexample					\
		rssimple					\
		rsexercise					\
		rscompare					\
		rsvalidate					\
		rspwd-test					\
		ezcod_test

testbin:	bintest
	./rsexample; ./rssimple; ./rsexercise; ./rscompare; ./rsvalidate; ./rspwd-test; ./ezcod_test

testjs:		jstest
	node ./rsexample.js; node ./rssimple.js; node ./rsexercise.js; node ./rspwd-test.js; node ./ezcod_test.js

rspwd-test.js:	rspwd-test.C rspwd.C c++/ezpwd/rs		\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS_MAIN) $< -o $@ 

js/ezpwd/rspwd.js: rspwd.C c++/ezpwd/rs				\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS_RSPWD) $< -o $@ 


ezcod.o:	ezcod.C ezcod.h c++/ezpwd/ezcod c++/ezpwd/rs
ezcod:		ezcod.o
	$(CXX) $(CXXFLAGS) -o $@ $^
js/ezpwd/ezcod.js:	ezcod.C ezcod.h c++/ezpwd/ezcod c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS_EZCOD) $< -o $@ 

clean:
	rm -f rsexample		rsexample.o	rsexample.js	\
	      rssimple		rssimple.o	rssimple.js	\
	      rsexercise	rsexercise.o	rsexercise.js	\
	      rspwd-test	rspwd-test.o	rspwd-test.js	\
	      rscompare		rscompare.o			\
	      rsvalidate	rsvalidate.o			\
	      ezcod		ezcod.o		ezcod.js	\
	      ezcod_test	ezcod_test.o	ezcod_test.jsn
	make -C phil-karn clean

rsexample.o:	rsexample.C c++/ezpwd/rs
rsexample:	rsexample.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexample.js:	rsexample.C c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS_MAIN) $< -o $@ 

rssimple.o:	rssimple.C c++/ezpwd/rs
rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rssimple.js:	rssimple.C c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS_MAIN) $< -o $@ 

rsexercise.o:	rsexercise.C exercise.H c++/ezpwd/rs
rsexercise:	rsexercise.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexercise.js:	rsexercise.C exercise.H c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS_MAIN) $< -o $@ 

rscompare.o:	rscompare.C c++/ezpwd/rs phil-karn/fec/rs-common.h
rscompare: CXXFLAGS += -I./phil-karn
rscompare:	rscompare.o phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rsvalidate.o:	rsvalidate.C c++/ezpwd/rs phil-karn/fec/rs-common.h
rsvalidate: CXXFLAGS += -I./phil-karn -ftemplate-depth=1000
rsvalidate:	rsvalidate.o phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rspwd-test.o:	rspwd-test.C rspwd.C c++/ezpwd/rs
rspwd-test:	rspwd-test.o
	$(CXX) $(CXXFLAGS) -o $@ $^

ezcod_test.o:	ezcod_test.C ezcod.C ezcod.h c++/ezpwd/ezcod c++/ezpwd/rs
ezcod_test.o: CXXFLAGS += -I./phil-karn           # if DEBUG set, include phil-karn/
ezcod_test.js: CXXFLAGS += -I./phil-karn           # if DEBUG set, include phil-karn/
ezcod_test:	ezcod_test.o ezcod.o  phil-karn/librs.a # if DEBUG set, link w/ phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^
ezcod_test.js: ezcod_test.C ezcod.C c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS_MAIN) $< ezcod.C -o $@

# 
# Build Phil Karn's R-S implementation.  Used by some tests.
# 
phil-karn/fec/rs-common.h \
phil-karn/librs.a:
	make -C phil-karn all

# 
# Install and build emscripten SDK, if necessary, and then activate it.
# 
# Presently only works on OS-X as far as I know. Should use a Docker instance.
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
