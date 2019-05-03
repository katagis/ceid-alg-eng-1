CXX = g++   # use the g++ compiler

VERB_LEVEL=0

LEDA_PATH = /usr/local/LEDA# directory where LEDA libraries are stored

CPPFILE = main.cpp

INCL_LEDA = -I$(LEDA_PATH)/incl
LINK_LEDAPATH = -L$(LEDA_PATH)
LINK_LEDA = -lleda
DEF_VERBOSITY = -DVERBOSITY=$(VERB_LEVEL)

LEDA_ALL = $(INCL_LEDA) $(LINK_LEDAPATH) $(LINK_LEDA)

all: run

release:
	$(CXX) $(CPPFILE) -o release.out $(DEF_VERBOSITY) $(LEDA_ALL) -O2
debug:
	$(CXX) $(CPPFILE) -o debug.out $(DEF_VERBOSITY) $(LEDA_ALL) -g

run: release
	./release.out
