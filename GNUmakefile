
SHELL		= /bin/bash
CXXFLAGS       += -I. -Wall -Wno-missing-braces -O3 -std=c++11 -DEZPWD_ARRAY_SAFE # -DEZPWD_ARRAY_TEST -DDEBUG=2
CXX		= clang++ # g++ 4.9.0 fails rspwd-test at all optimization levels!

EMSDK		= ./emscripten/emsdk_portable
EMSDK_ACTIVATE	= ./emscripten/emsdk_portable/emsdk activate latest

EMSDK_EMXX 	= pushd $(EMSDK) && source ./emsdk_env.sh && popd && PATH=$${PATH}:`pwd`/emscripten && em++
DOCKER_EMXX	= docker run -v \$( shell pwd ):/mnt/test cmfatih/emscripten /srv/var/emscripten/em++ -I/mnt/test

EMXX		= $(EMSDK_EMXX)
EMXX_ACTIVATE	= $(EMSDK_ACTIVATE)

EMXX_EXPORTS	= "['_rspwd_encode_1', '_rspwd_encode_2', '_rspwd_encode_3']"
EMXX_MAIN	= "['_main']"

EMSDK_URL	= https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz


all:	testjs rspwd.js

test:	rssimple rsexercise rscompare rsvalidate rspwd-test
	./rssimple; ./rsexercise; ./rscompare; ./rsvalidate; ./rspwd-test

testjs:	rssimple.js rsexercise.js rspwd-test.js
	node ./rssimple.js; node ./rsexercise.js; node ./rspwd-test.js

rspwd-test.js:	rspwd-test.C rspwd.C			\
		emscripten
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

rspwd.js:	rspwd.C rs				\
		emscripten
	$(EMXX) $(CXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS) $< -o $@ 

clean:
	rm -f rssimple		rssimple.o		\
	      rscompare		rscompare.o		\
	      rsvalidate	rsvalidate.o		\
	      rspwd-test	rspwd-test.o
	make -C phil-karn clean

rssimple.o:	rssimple.C rs
rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rssimple.js:	rssimple.C rs
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

rsexercise.o:	rsexercise.C rs exercise.H
rsexercise:	rsexercise.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexercise.js:	rsexercise.C rs exercise.H
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

rscompare.o:	rscompare.C rs phil-karn/fec/rs-common.h
rscompare: CXXFLAGS += -I./phil-karn
rscompare:	rscompare.o  phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rsvalidate.o:	rsvalidate.C rs phil-karn/fec/rs-common.h
rsvalidate: CXXFLAGS += -I./phil-karn -ftemplate-depth=1000
rsvalidate:	rsvalidate.o phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rspwd-test.o:	rspwd-test.C rspwd.C rs
rspwd-test:	rspwd-test.o
	$(CXX) $(CXXFLAGS) -o $@ $^

phil-karn/fec/rs-common.h				\
phil-karn/librs.a:
	make -C phil-karn all

# 
# Install and build emscripten SDK, if necessary, and then activate it.
# 
# Presently only works on OS-X as far as I know. Should use a Docker instance.
# 
emscripten:	emscripten/python2 			\
		emscripten/emsdk_portable/emscripten	\
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
