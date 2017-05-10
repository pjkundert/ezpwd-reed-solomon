
SHELL		= /bin/bash

# Compilers
# 
#    Defaults to system C/C++; define CXX to use a specific C++ compiler.  Supported across:
# 
# g++    4.8 - 6.3	-- Recommended; fastest (compilation produces some incorrect [-Wmaybe-uninitialized] warnings)
# clang  3.6+		-- Recommended
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
CFLAGS		= # -O3 already defined

# Emscripten
#   - At -O2 and above, code is minified
# 
#     Instead of 'latest', choose a specific version known to work.  To see
# what's available, use:
# 
#         $ cd emscripten/emsdk_portable   
#         $ ./emsdk list
# 
#     At time of writing, 1.35.0 is "latest", but doesn't work (1.34.1 did, I believe).  The current
# master is 1.36.0, which does appear to work (passes all unit tests, works on ezcod.com).
# 
EMSDK		= ./emscripten/emsdk_portable
EMSDK_VERSION	= latest # sdk-master-64bit # latest
EMSDK_ACTIVATE	= ( cd $(EMSDK); ./emsdk update && ./emsdk install $(EMSDK_VERSION) && ./emsdk activate $(EMSDK_VERSION) )

EMSDK_EMXX 	= pushd $(EMSDK) && source ./emsdk_env.sh && popd && PATH=`pwd`/emscripten:$${PATH} && em++
DOCKER_EMXX	= docker run -v \$( shell pwd ):/mnt/test cmfatih/emscripten /srv/var/emscripten/em++ -I/mnt/test
CHEERP_EMXX	= /opt/cheerp/bin/clang++

EMXX		= $(EMSDK_EMXX)
EMXX_ACTIVATE	= $(EMSDK_ACTIVATE)
EMXXFLAGS	= --memory-init-file 0 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=2

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

EXCOMP =	rsencode rsencode_9 rsencode_16			\
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
		rskey_test					\
		bchsimple					\
		bchclassic					\
		bch_test

# Don't include bch_itron: too dependent on recent boost, etc.
#		bch_itron


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
swig-python-install: c++/ezpwd/ezcod c++/ezpwd/bch
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
	      ezcod.o djelic_bch*.o
	make -C phil-karn clean

rspwd_test.js:	rspwd_test.C rspwd.C						\
		c++/ezpwd/rs c++/ezpwd/serialize c++/ezpwd/corrector		\
		emscripten
	$(EMXX) $(CXXFLAGS) $(EMXXFLAGS) $(EMXX_EXPORTS_MAIN) $< -o $@ 

# rsencode -- correct 8-bit symbols (default to 128 symbol chunks, each w/ 32 parity symbols)
rsencode.o:	rsencode.C c++/ezpwd/rs
rsencode:	rsencode.o
	$(CXX) $(CXXFLAGS) -o $@ $^
	echo "abcd" | ./$@ | perl -pe "s|a|b|" | ./$@ --decode | grep -q "abcd" >/dev/null

# rsencode_9 -- correct lower 9 bits of 16-bit symbols (serialized big-endian)
# Thus, only the lowest bit of the 1st, 3rd, etc. character is corrected
# (remainder passed through unchanged).  So, 'a' (0b01100001) changed to 'b'
# (0b01100010), and is thus corrected to 'c' (0b01100011).
rsencode_9:	CXXFLAGS += -DRSCODEWORD=511 -DRSPARITY=32 -DRSCHUNK=128
rsencode_9.o:	rsencode.C c++/ezpwd/rs
	$(CXX) $(CXXFLAGS) -c -o $@ $<
rsencode_9:	rsencode_9.o
	$(CXX) $(CXXFLAGS) -o $@ $^
	echo "abcde" | ./$@ | perl -pe "s|a|b|" | ./$@ --decode | grep -q "cbcde" >/dev/null

# rsencode_16 -- correct all bits of 16-bit symbols (serialized big-endian)
rsencode_16:	CXXFLAGS += -DRSCODEWORD=65535 -DRSPARITY=32 -DRSCHUNK=128
rsencode_16.o:	rsencode.C c++/ezpwd/rs
	$(CXX) $(CXXFLAGS) -c -o $@ $<
rsencode_16:	rsencode_16.o
	$(CXX) $(CXXFLAGS) -o $@ $^
	echo "abcde" | ./$@ | perl -pe "s|a|b|" | ./$@ --decode | grep -q "abcde" >/dev/null

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

rsembedded_nexc: 	CXXFLAGS += -DEZPWD_NO_EXCEPTS -fno-exceptions
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

rscompare_nexc:		CXXFLAGS += -DEZPWD_NO_EXCEPTS -fno-exceptions
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
# BCH tests.
# 

bchsimple.o:	CXXFLAGS += -I standalone -I djelic/Documentation/bch/standalone -I djelic/include
bchsimple.o:	bchsimple.C c++/ezpwd/bch
bchsimple:	bchsimple.o djelic_bch.o
	$(CXX) $(CXXFLAGS) -o $@ $^

bchclassic.o:	CXXFLAGS += -I standalone -I djelic/Documentation/bch/standalone -I djelic/include
bchclassic.o:	bchclassic.C c++/ezpwd/bch
bchclassic:	bchclassic.o djelic_bch.o
	$(CXX) $(CXXFLAGS) -o $@ $^

bch_test.o:	CXXFLAGS += -I standalone -I djelic/Documentation/bch/standalone -I djelic/include
bch_test.o:	bch_test.C c++/ezpwd/bch
bch_test:	bch_test.o djelic_bch.o
	$(CXX) $(CXXFLAGS) -o $@ $^


bch_itron.o:	CXXFLAGS += -I standalone -I djelic/Documentation/bch/standalone -I djelic/include -I /usr/local/include
bch_itron.o:	bch_itron.C djelic/include
bch_itron: 	CXXFLAGS += -L /usr/local/lib # boost
bch_itron:	bch_itron.o djelic_bch.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_filesystem

.PHONY: itron_test
itron_test:	bch_itron
	./bch_itron			&& exit 0 || exit 1 # expect success

qi_test_1.o:	qi_test_1.C
qi_test_1:	qi_test_1.o
	$(CXX) $(CXXFLAGS) -o $@ $^

qi_test_2.o:	qi_test_2.C
qi_test_2:	qi_test_2.o
	$(CXX) $(CXXFLAGS) -o $@ $^

boost_test:	qi_test_1 qi_test_2
	./qi_test_1 < qi_test_1_good.txt	&& exit 0 || exit 1 # expect success
	./qi_test_1 < qi_test_1_bad.txt		&& exit 1 || exit 0 # expect failure
	./qi_test_2 				&& exit 0 || exit 1 # expect success

# 
# Build Phil Karn's R-S implementation.  Used by some tests.
# 
#     Any tests using Phil Karn's library aren't yet convertible to Javascript
# 
phil-karn/fec/rs-common.h \
phil-karn/librs.a:
	CFLAGS=$(CFLAGS) CC=$(CC) make -C phil-karn all

# 
# Schifra R-S implementation.  Used by some tests.
# 
schifra:
	git clone https://github.com/ArashPartow/schifra.git

# 
# Djelic BCH implementation.  Foundation for EZPWD BCH implementation
# 
#     We provide the original Djelic "BCH" Linux Kernel API implementation, from source.
# This means that we require you to git checkout the Djelic source code, in order to either
# build the djelic_bch.o target (for use in C/C++ projects), or to simply build your "C"
# target directly using the djleic_bch.c source.
#   - Requires Djelic "standalone" shims for building Kernel code in user space
#   - Also requires the additional "standalone" linux/errno.h shim for non-Linux builds
# 
#     djelic_bch.h	 	-- API declarations
#     djelic_bch.c		-- API definitions
# 
#     You can build the djelic_bch.o object files for non-"C" projects to use, or simply compile the
# djelic_bch.c file directly into your "C" application (to avoid needing to pre-compile the object
# files to satisfy your build).
# 
# djelictest: 	build and run Djelic BCH tests once.  Upstream: https://github.com/Parrot-Developers/bch.git
# 
djelic:
	git clone https://github.com/pjkundert/bch.git $@

c++/ezpwd/bch \
djelic/include \
djelic/lib/bch.c: djelic

.PHONY: djelictest
djelictest:	djelic/Documentation/bch/nat_tu_tool

djelic/Documentation/bch/nat_tu_tool: djelic
	cd djelic/Documentation/bch && make && ./nat_tu_short.sh

djelic_bch.c:	CFLAGS += -I standalone -I djelic/Documentation/bch/standalone -I djelic/include
djelic_bch.o:	CFLAGS += -I standalone -I djelic/Documentation/bch/standalone -I djelic/include
djelic_bch.o:	djelic_bch.c		djelic/lib/bch.c
	$(CC) $(CFLAGS) -c -o $@ $<


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
