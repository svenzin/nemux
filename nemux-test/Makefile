CPPFLAGS += -g -Wall -Wextra -pedantic -std=c++11
CXXFLAGS += -g -Wall -Wextra -pedantic -std=c++11

GTEST_DIR = ./gtest-1.6.0
NEMUX_DIR = ../nemux

all : nemux-tests

clean :
	rm -f *.o

gtest :
	make -C $(GTEST_DIR)/make all
	
nemux :
	make -C $(NEMUX_DIR)

nemux-tests-objects :
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include -I$(NEMUX_DIR) -c *.cpp

nemux-tests :  gtest nemux nemux-tests-objects
	$(CXX) $(CXXFLAGS) -pthread -o nemux-tests *.o $(NEMUX_DIR)/nemux.a $(GTEST_DIR)/make/gtest_main.a
