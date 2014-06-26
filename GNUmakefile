
SHELL	= /bin/bash
CXXFLAGS += -I. -Wall -g -O0 -std=c++11

rssimple.o:	rssimple.C rs rs.H

rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^
