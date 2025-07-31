all: reconstruction test_orderbook

reconstruction: main.cpp orderbook.h
	g++ -o reconstruction main.cpp -std=c++11

test_orderbook: test_orderbook.cpp orderbook.h
	g++ -o test_orderbook test_orderbook.cpp -std=c++11

clean:
	rm -f reconstruction test_orderbook