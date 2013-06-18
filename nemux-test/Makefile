CPPFLAGS += -g -Wall -Wextra -pedantic -std=c++11
CXXFLAGS += -g -Wall -Wextra -pedantic -std=c++11

GTEST_DIR = ./gtest-1.6.0
NEMUX_DIR = ../nemux

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all : nemux-tests

clean :
	rm -f *.o nemux-tests

gtest :
	make -C $(GTEST_DIR)/make all
	
nemux :
	make -C $(NEMUX_DIR)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include -I$(NEMUX_DIR) -c $<

nemux-tests :  gtest nemux $(OBJS)
	$(CXX) $(CXXFLAGS) -pthread -o $@ $(OBJS) $(NEMUX_DIR)/nemux.a $(GTEST_DIR)/make/gtest_main.a
