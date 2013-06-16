CPPFLAGS += -g -Wall -Wextra -pedantic -std=c++11
CXXFLAGS += -g -Wall -Wextra -pedantic -std=c++11

OBJS = *.o

all : nemux.a

clean :
	rm -f *.o

.cpp.o :
	$(CXX) $(CXXFLAGS) -c $<

nemux.a : $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJS) : *.h
