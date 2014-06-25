
SHELL	= /bin/bash
CXXFLAGS += -I. -Wall -std=c++11

rssimple.o:	rssimple.C rs rs.H

rssimple:	rssimple.o
	$(CXX) $(CXXFLAGS) -o $@ $^
