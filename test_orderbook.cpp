#include "orderbook.h"
#include <cassert>
#include <iostream>
#include <cmath>

void test_clear() {
    OrderBook book;
    book.add_order(1, 'B', 100.0, 10);
    book.add_order(2, 'A', 101.0, 15);
    book.clear();
    assert(book.get_bids().empty());
    assert(book.get_asks().empty());
    assert(book.get_orders().empty());
    std::cout << "test_clear passed\n";
}

void test_add_order() {
    OrderBook book;
    book.add_order(1, 'B', 100.0, 10);
    book.add_order(2, 'B', 101.0, 20);
    book.add_order(3, 'A', 102.0, 15);

    auto bids = book.get_bids();
    assert(bids.size() == 2);
    auto it = bids.begin();
    assert(std::abs(it->first - 101.0) < 1e-9);
    assert(it->second.total_size == 20);
    assert(it->second.order_count == 1);
    ++it;
    assert(std::abs(it->first - 100.0) < 1e-9);
    assert(it->second.total_size == 10);
    assert(it->second.order_count == 1);

    auto asks = book.get_asks();
    assert(asks.size() == 1);
    it = asks.begin();
    assert(std::abs(it->first - 102.0) < 1e-9);
    assert(it->second.total_size == 15);
    assert(it->second.order_count == 1);

    auto orders = book.get_orders();
    assert(orders.size() == 3);
    assert(orders.at(1).size == 10);
    assert(orders.at(2).size == 20);
    assert(orders.at(3).size == 15);
    std::cout << "test_add_order passed\n";
}

void test_trade_order() {
    OrderBook book;
    book.add_order(1, 'B', 100.0, 10);
    book.trade_order(1, 3);

    auto orders = book.get_orders();
    auto it = orders.find(1);
    assert(it != orders.end());
    assert(it->second.size == 7);
    assert(it->second.traded_size == 3);

    auto bids = book.get_bids();
    auto bid_it = bids.find(100.0);
    assert(bid_it != bids.end());
    assert(bid_it->second.total_size == 7);
    assert(bid_it->second.order_count == 1);
    std::cout << "test_trade_order passed\n";
}

void test_cancel_order() {
    OrderBook book;
    book.add_order(1, 'B', 100.0, 10);
    book.trade_order(1, 3);
    int depth;
    int64_t traded_size = book.cancel_order(1, 7, depth);
    assert(traded_size == 3);
    assert(depth == 0);

    auto orders = book.get_orders();
    assert(orders.find(1) == orders.end());

    auto bids = book.get_bids();
    auto bid_it = bids.find(100.0);
    if (bid_it != bids.end()) {
        assert(bid_it->second.total_size == 0);
        assert(bid_it->second.order_count == 0);
    } else {
        assert(true); 
    }
    std::cout << "test_cancel_order passed\n";
}

void test_get_top10() {
    OrderBook book;
    for (int i = 0; i < 15; i++) {
        book.add_order(100 + i, 'B', 100.0 + i, 10);
    }
    for (int i = 0; i < 15; i++) {
        book.add_order(200 + i, 'A', 115.0 + i, 10);
    }

    double bid_px[10], ask_px[10];
    int64_t bid_sz[10], ask_sz[10];
    uint32_t bid_ct[10], ask_ct[10];
    book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

    for (int i = 0; i < 10; i++) {
        assert(std::abs(bid_px[i] - (114.0 - i)) < 1e-9);
        assert(bid_sz[i] == 10);
        assert(bid_ct[i] == 1);
        assert(std::abs(ask_px[i] - (115.0 + i)) < 1e-9);
        assert(ask_sz[i] == 10);
        assert(ask_ct[i] == 1);
    }

    book.clear();
    for (int i = 0; i < 5; i++) {
        book.add_order(100 + i, 'B', 100.0 + i, 10);
    }
    for (int i = 0; i < 5; i++) {
        book.add_order(200 + i, 'A', 115.0 + i, 10);
    }
    book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

    for (int i = 0; i < 5; i++) {
        assert(std::abs(bid_px[i] - (104.0 - i)) < 1e-9);
        assert(bid_sz[i] == 10);
        assert(bid_ct[i] == 1);
        assert(std::abs(ask_px[i] - (115.0 + i)) < 1e-9);
        assert(ask_sz[i] == 10);
        assert(ask_ct[i] == 1);
    }
    for (int i = 5; i < 10; i++) {
        assert(std::abs(bid_px[i] - 0.0) < 1e-9);
        assert(bid_sz[i] == 0);
        assert(bid_ct[i] == 0);
        assert(std::abs(ask_px[i] - 0.0) < 1e-9);
        assert(ask_sz[i] == 0);
        assert(ask_ct[i] == 0);
    }
    std::cout << "test_get_top10 passed\n";
}

int main() {
    test_clear();
    test_add_order();
    test_trade_order();
    test_cancel_order();
    test_get_top10();
    std::cout << "All tests passed successfully.\n";
    return 0;
}