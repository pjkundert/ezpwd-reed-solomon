
SHELL	= /bin/bash
CXXFLAGS += -I. -Wall -Wno-missing-braces -O1 -std=c++11 -DEZPWD_ARRAY_SAFE # -DEZPWD_ARRAY_TEST -DDEBUG=2
CXX = clang++ # g++ 4.9.0 fails rspwd-test at all optimization levels!

EMCC_DOCKER = docker run -v `pwd`:/mnt/test cmfatih/emscripten /srv/var/emscripten/emcc -I/mnt/test $(CXXFLAGS)
EMCC_EXPORT = "['_rspwd_encode_1', '_rspwd_encode_2', '_rspwd_encode_3']"

test:   rssimple rscompare rsvalidate rspwd-test
	./rssimple; ./rscompare; ./rsvalidate; ./rspwd-test

rspwd-test.js:	rspwd-test.C rspwd.C
	$(EMCC_DOCKER) -s EXPORTED_FUNCTIONS=$(EMCC_EXPORT) /mnt/test/$< -o /mnt/test/$@ 
rspwd.js:	rspwd.C
	$(EMCC_DOCKER) -s EXPORTED_FUNCTIONS=$(EMCC_EXPORT) /mnt/test/$< -o /mnt/test/$@ 

clean:
	rm -f rssimple		rssimple.o	\
	      rscompare		rscompare.o	\
	      rsvalidate	rsvalidate.o	\
	      rspwd-test	rspwd-test.o
	make -C phil-karn clean

rssimple.o:	rssimple.C rs exercise.H
rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^

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

phil-karn/fec/rs-common.h	\
phil-karn/librs.a:
	make -C phil-karn all
