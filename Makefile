include config.mk

EXE=main
OBJECTS=main.o



DEBUG=1
DEBUGFLAGS = -g -D_GLIBCXX_DEBUG #-fsanitize=undefined,address
FASTFLAGS = -O2
CXXFLAGS_BASE = -std=c++14
CFLAGS_BASE = -std=c99


COMPILER ?= GCC
ifeq ($(COMPILER),GCC)
	CC=gcc
	CXX=g++
	WARNFLAGS=-Wall -Wextra -pedantic
else
	CC=clang
	CXX=clang++
	WARNFLAGS=-Weverything -pedantic -Wno-c++98-compat -Wno-c++98-c++11-compat -Wno-sign-conversion -Wno-padded -Wno-exit-time-destructors -Wno-global-constructors
endif


DEBUG ?= 1
ifeq ($(DEBUG),1)
	FLAGS += $(DEBUGFLAGS)
else
	FLAGS += $(FASTFLAGS)
endif

FLAGS += $(WARNFLAGS)
CFLAGS = $(CFLAGS_BASE) $(FLAGS)
CXXFLAGS = $(CXXFLAGS_BASE) $(FLAGS)
LINK=$(CXX)
LINKFLAGS=$(CXXFLAGS)
LIBS=


# Pseudotargets
.PHONY: all clean run info

all: $(EXE)

clean:
	rm -f $(EXE) $(OBJECTS) $(OBJECTS:.o=.d) depend

info:
	@echo DEBUGFLAGS = $(DEBUGFLAGS)
	@echo FASTFLAGS = $(FASTFLAGS)
	@echo DEBUG = $(DEBUG)
	@echo CXXFLAGS_BASE = $(CXXFLAGS_BASE)
	@echo CXXFLAGS = $(CXXFLAGS)
	@echo LDFLAGS= $(LDFLAGS)

run: $(EXE)
	./$(EXE)

graph.pdf: $(EXE)
	#./$(EXE) | dot -Tps2 | ps2pdf - >$@
	./$(EXE) | dot -Tpdf  | csplit --quiet --elide-empty-files --prefix=tmpfile - "/%%EOF/+1" "{*}" && pdfunite tmpfile* $@ && rm tmpfile*

# Build system

config.mk:
	echo 'COMPILER=GCC  # possible values: GCC, clang' >> $@

include depend

depend: $(OBJECTS:.o=.d)
	cat $^ > $@

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXE): $(OBJECTS)
	$(LINK) $(LINKFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@

-include .dummy.mk

.dummy.mk: Makefile config.mk
	@echo Makefile changed, cleaning to rebuild
	@echo "# used to let all targets depend on Makefile" > $@
	make -s clean

