CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++11 -Wall -O2

all: reconstruction.exe test_orderbook.exe

reconstruction.exe: main.cpp orderbook.h
	$(CXX) $(CXXFLAGS) -o reconstruction.exe main.cpp

test_orderbook.exe: test_orderbook.cpp orderbook.h
	$(CXX) $(CXXFLAGS) -o test_orderbook.exe test_orderbook.cpp

clean:
	rm -f *.exe

.PHONY: all clean
