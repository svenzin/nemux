CPPFLAGS += -g -Wall -Wextra -pedantic -std=c++11
CXXFLAGS += -g -Wall -Wextra -pedantic -std=c++11

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all : nemux.a

clean :
	rm -f *.o nemux.a

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

nemux.a : $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	