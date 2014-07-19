
SHELL		= /bin/bash
CXXFLAGS       += -I./c++ -Wall -Wextra -Wpedantic -Wno-missing-braces -O3 -std=c++11 -DEZPWD_ARRAY_SAFE #-DEZPWD_ARRAY_TEST -DDEBUG=2
CXX		= clang++ # g++ 4.9.0 fails rspwd-test at all optimization levels!
#CXX		= g++

EMSDK		= ./emscripten/emsdk_portable
EMSDK_ACTIVATE	= ./emscripten/emsdk_portable/emsdk activate latest

EMSDK_EMXX 	= pushd $(EMSDK) && source ./emsdk_env.sh && popd && PATH=$${PATH}:`pwd`/emscripten && em++
DOCKER_EMXX	= docker run -v \$( shell pwd ):/mnt/test cmfatih/emscripten /srv/var/emscripten/em++ -I/mnt/test

EMXX		= $(EMSDK_EMXX)
EMXX_ACTIVATE	= $(EMSDK_ACTIVATE)

EMXX_EXPORTS	= "['_rspwd_encode_1', '_rspwd_encode_2', '_rspwd_encode_3', '_rspwd_encode_4', '_rspwd_encode_5']"
EMXX_MAIN	= "['_main']"

EMSDK_URL	= https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz


all:	testjs js/ezpwd/rspwd.js

test:	rsexample rssimple rsexercise rscompare rsvalidate rspwd-test rs_5_10
	./rsexample; ./rssimple; ./rsexercise; ./rscompare; ./rsvalidate; ./rspwd-test; ./rs_5_10

testjs:	rsexample.js rssimple.js rsexercise.js rspwd-test.js rs_5_10.js
	node ./rsexample.js; node ./rssimple.js; node ./rsexercise.js; node ./rspwd-test.js; node ./rs_5_10.js

rspwd-test.js:	rspwd-test.C rspwd.C				\
		emscripten
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

js/ezpwd/rspwd.js: rspwd.C c++/ezpwd/rs				\
		emscripten
	$(EMXX) $(CXXFLAGS) -s EXPORTED_FUNCTIONS=$(EMXX_EXPORTS) $< -o $@ 

clean:
	rm -f rssimple		rssimple.o	rssimple.js	\
	      rsexercise	rsexercise.o	rsexercise.js	\
	      rspwd-test	rspwd-test.o	rspwd-test.js	\
	      rscompare		rscompare.o			\
	      rsvalidate	rsvalidate.o			\
	      rs_5_10		rs_5_10.o	rs_5_10.js
	make -C phil-karn clean

rsexample.o:	rsexample.C c++/ezpwd/rs
rsexample:	rsexample.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexample.js:	rsexample.C c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

rssimple.o:	rssimple.C c++/ezpwd/rs
rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rssimple.js:	rssimple.C c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

rsexercise.o:	rsexercise.C exercise.H c++/ezpwd/rs
rsexercise:	rsexercise.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rsexercise.js:	rsexercise.C exercise.H c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

rscompare.o:	rscompare.C c++/ezpwd/rs phil-karn/fec/rs-common.h
rscompare: CXXFLAGS += -I./phil-karn
rscompare:	rscompare.o  phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rsvalidate.o:	rsvalidate.C c++/ezpwd/rs phil-karn/fec/rs-common.h
rsvalidate: CXXFLAGS += -I./phil-karn -ftemplate-depth=1000
rsvalidate:	rsvalidate.o phil-karn/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rspwd-test.o:	rspwd-test.C rspwd.C c++/ezpwd/rs
rspwd-test:	rspwd-test.o
	$(CXX) $(CXXFLAGS) -o $@ $^

rs_5_10.o:	rs_5_10.C c++/ezpwd/rs
rs_5_10:	rs_5_10.o
	$(CXX) $(CXXFLAGS) -o $@ $^
rs_5_10.js:	rs_5_10.C c++/ezpwd/rs
	$(EMXX) $(CXXFLAGS) -s DISABLE_EXCEPTION_CATCHING=0 -s EXPORTED_FUNCTIONS=$(EMXX_MAIN) $< -o $@ 

phil-karn/fec/rs-common.h					\
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
