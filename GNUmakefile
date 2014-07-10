
SHELL	= /bin/bash
CXXFLAGS += -I. -Wall -Wno-missing-braces -O3 -std=c++11  -DEZPWD_ARRAY_SAFE # -DEZPWD_ARRAY_TEST

test:	rssimple rscompare rsvalidate
	./rssimple; ./rscompare; ./rsvalidate

clean:
	rm -f rssimple		rssimple.o	\
	      rscompare		rscompare.o	\
	      rsvalidate	rsvalidate.o
	make -C reed-solomon clean

rssimple.o:	rssimple.C rs exercise.H
rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^

rscompare.o:	rscompare.C rs reed-solomon/fec/rs-common.h
rscompare: CXXFLAGS  += -I./reed-solomon
rscompare:	rscompare.o  reed-solomon/librs.a
	$(CXX) $(CXXFLAGS) -o $@ $^

rsvalidate.o:	rsvalidate.C rs reed-solomon/fec/rs-common.h
rsvalidate: CXXFLAGS  += -I./reed-solomon -ftemplate-depth=1000
rsvalidate:	rsvalidate.o reed-solomon/librs.a

	$(CXX) $(CXXFLAGS) -o $@ $^

reed-solomon/fec/rs-common.h	\
reed-solomon/librs.a:
	make -C reed-solomon all
