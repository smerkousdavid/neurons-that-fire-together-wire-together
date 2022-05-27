CXX             := c++
OPTS            := -Ofast -march=native -DNDEBUG
LANG            := -std=c++17
PICKY           := -Wall
INCLUDES	:= -Isrc
LIBS		+= -L/usr/local/lib -lomp

CXXFLAGS	+= $(DEFS) $(XDEFS) $(OPTS) $(DEBUG) $(PROFILE) $(LANG) $(PICKY) $(INCLUDES) $(DIAG)

all		: $(TARGETS)

%.exe   	: %.o $(LOBJ)
		  $(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

%.o 		: %.cpp
		  $(CXX) -c $(CXXFLAGS) $< -o $@

%.s 		: %.cpp
		  $(CXX) -S $(CXXFLAGS) $<

clean		:
		  /bin/rm -f $(TARGETS) $(TESTS) $(OBJECTS) $(PCHS) Matrix.s a.out *~ Makefile.bak makedep
