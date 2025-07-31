#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>

struct LevelInfo {
    int64_t total_size = 0;
    uint32_t order_count = 0;
};

struct Order {
    char side;
    double price;
    int64_t size;
    int64_t original_size;
    int64_t traded_size;

    Order() = default;

    Order(char s, double p, int64_t sz, int64_t orig_sz, int64_t traded = 0)
        : side(s), price(p), size(sz), original_size(orig_sz), traded_size(traded) {}
};


using BidBook = std::map<double, LevelInfo, std::greater<double>>;
using AskBook = std::map<double, LevelInfo, std::less<double>>;

class OrderBook {
private:
    BidBook bids;
    AskBook asks;
    std::unordered_map<uint64_t, Order> orders;

public:
    void clear() {
        bids.clear();
        asks.clear();
        orders.clear();
    }

    void add_order(uint64_t order_id, char side, double price, int64_t size) {
        orders[order_id] = Order{side, price, size, size, 0};
        if (side == 'B') {
            auto& level = bids[price];
            level.total_size += size;
            level.order_count++;
        } else if (side == 'A') {
            auto& level = asks[price];
            level.total_size += size;
            level.order_count++;
        }
    }

    void trade_order(uint64_t order_id, int64_t size) {
        auto it = orders.find(order_id);
        if (it == orders.end()) return;
        
        Order& order = it->second;
        int64_t trade_size = std::min(order.size, size);
        order.size -= trade_size;
        order.traded_size += trade_size;
        if (order.side == 'B') {
            auto it_bid = bids.find(order.price);
            if (it_bid != bids.end()) {
                it_bid->second.total_size -= trade_size;
            }
        } else if (order.side == 'A') {
            auto it_ask = asks.find(order.price);
            if (it_ask != asks.end()) {
                it_ask->second.total_size -= trade_size;
            }
        }
    }

    int64_t cancel_order(uint64_t order_id, int64_t size, int& depth_out) {
        auto it = orders.find(order_id);
        if (it == orders.end()) return -1;

        Order order = it->second;
        int64_t cancel_size = std::min(order.size, size);
        orders.erase(it);
        depth_out = -1;

        if (order.side == 'B') {
            int depth = 0;
            for (const auto& level : bids) {
                if (std::abs(level.first - order.price) < 1e-9) {
                    depth_out = depth;
                    break;
                }
                depth++;
            }
            auto it_bid = bids.find(order.price);
            if (it_bid != bids.end()) {
                LevelInfo& level = it_bid->second;
                level.total_size -= cancel_size;
                level.order_count--;
                if (level.order_count == 0 || level.total_size <= 0) {
                    bids.erase(it_bid);
                }
            }
        } else if (order.side == 'A') {
            int depth = 0;
            for (const auto& level : asks) {
                if (std::abs(level.first - order.price) < 1e-9) {
                    depth_out = depth;
                    break;
                }
                depth++;
            }
            auto it_ask = asks.find(order.price);
            if (it_ask != asks.end()) {
                LevelInfo& level = it_ask->second;
                level.total_size -= cancel_size;
                level.order_count--;
                if (level.order_count == 0 || level.total_size <= 0) {
                    asks.erase(it_ask);
                }
            }
        }
        return order.traded_size;
    }

    void get_top10(double* bid_px, int64_t* bid_sz, uint32_t* bid_ct,
                   double* ask_px, int64_t* ask_sz, uint32_t* ask_ct) const {
        std::fill_n(bid_px, 10, 0.0);
        std::fill_n(bid_sz, 10, 0);
        std::fill_n(bid_ct, 10, 0);
        std::fill_n(ask_px, 10, 0.0);
        std::fill_n(ask_sz, 10, 0);
        std::fill_n(ask_ct, 10, 0);

        int i = 0;
        for (auto it = bids.begin(); it != bids.end() && i < 10; ++it, ++i) {
            bid_px[i] = it->first;
            bid_sz[i] = it->second.total_size;
            bid_ct[i] = it->second.order_count;
        }
        i = 0;
        for (auto it = asks.begin(); it != asks.end() && i < 10; ++it, ++i) {
            ask_px[i] = it->first;
            ask_sz[i] = it->second.total_size;
            ask_ct[i] = it->second.order_count;
        }
    }

    const BidBook& get_bids() const { return bids; }
    const AskBook& get_asks() const { return asks; }
    const std::unordered_map<uint64_t, Order>& get_orders() const { return orders; }
};

#endif