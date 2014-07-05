
SHELL	= /bin/bash
CXXFLAGS += -I. -Wall -g -O3 -std=c++11

rssimple.o:	rssimple.C rs array_safe exercise.H
rssimple: CXXFLAGS += -DARRAY_SAFE # -DARRAY_TEST
rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^

rscompare.o:	rscompare.C rs array_safe
rscompare: CXXFLAGS  += -I./reed-solomon -DARRAY_SAFE # -DARRAY_TEST
rscompare:	rscompare.o
	$(CXX) $(CXXFLAGS) -o $@ $^ \
	    reed-solomon/init_rs_char.o \
	    reed-solomon/encode_rs_char.o \
	    reed-solomon/decode_rs_char.o
