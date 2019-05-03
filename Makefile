CXX = g++   # use the g++ compiler

LEDA_PATH = /usr/local/LEDA# directory where LEDA libraries are stored

CPPFILE = main.cpp

INCL_LEDA = -I$(LEDA_PATH)/incl
LINK_LEDAPATH = -L$(LEDA_PATH)
LINK_LEDA = -lleda


LEDA_ALL = $(INCL_LEDA) $(LINK_LEDAPATH) $(LINK_LEDA)

all: run

release:
	$(CXX) $(CPPFILE) -o release.out $(LEDA_ALL) -O2
debug:
	$(CXX) $(CPPFILE) -o debug.out $(LEDA_ALL) -g

run: release
	./release.out
