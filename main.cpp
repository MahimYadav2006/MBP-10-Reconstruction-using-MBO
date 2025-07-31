// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <vector>
// #include <string>
// #include <map>
// #include <unordered_map>
// #include <iomanip>
// #include <algorithm>
// #include <cmath>

// struct LevelInfo {
//     int64_t total_size = 0;
//     uint32_t order_count = 0;
// };

// struct Order {
//     char side;
//     double price;
//     int64_t size;
//     int64_t original_size;
//     int64_t traded_size = 0;
// };

// using BidBook = std::map<double, LevelInfo, std::greater<double>>;
// using AskBook = std::map<double, LevelInfo, std::less<double>>;

// class OrderBook {
// private:
//     BidBook bids;
//     AskBook asks;
//     std::unordered_map<uint64_t, Order> orders;

// public:
//     void clear() {
//         bids.clear();
//         asks.clear();
//         orders.clear();
//     }

//     void add_order(uint64_t order_id, char side, double price, int64_t size) {
//         orders[order_id] = Order{side, price, size, size, 0};

//         if (side == 'B') {
//             auto& level = bids[price];
//             level.total_size += size;
//             level.order_count++;
//         } else if (side == 'A') {
//             auto& level = asks[price];
//             level.total_size += size;
//             level.order_count++;
//         }
//     }

//     void trade_order(uint64_t order_id, int64_t size) {
//         auto it = orders.find(order_id);
//         if (it == orders.end()) return;
        
//         Order& order = it->second;
//         int64_t trade_size = std::min(order.size, size);
//         order.size -= trade_size;
//         order.traded_size += trade_size;

//         if (order.side == 'B') {
//             auto it_bid = bids.find(order.price);
//             if (it_bid != bids.end()) {
//                 it_bid->second.total_size -= trade_size;
//             }
//         } else if (order.side == 'A') {
//             auto it_ask = asks.find(order.price);
//             if (it_ask != asks.end()) {
//                 it_ask->second.total_size -= trade_size;
//             }
//         }
//     }

//     int64_t cancel_order(uint64_t order_id, int64_t size, int& depth_out) {
//         auto it = orders.find(order_id);
//         if (it == orders.end()) return -1;

//         Order order = it->second;
//         int64_t cancel_size = std::min(order.size, size);
//         orders.erase(it);

//         depth_out = -1;
//         if (order.side == 'B') {
//             int depth = 0;
//             for (const auto& level : bids) {
//                 if (std::abs(level.first - order.price) < 1e-9) {
//                     depth_out = depth;
//                     break;
//                 }
//                 depth++;
//             }

//             auto it_bid = bids.find(order.price);
//             if (it_bid != bids.end()) {
//                 LevelInfo& level = it_bid->second;
//                 level.total_size -= cancel_size;
//                 level.order_count--;
//                 if (level.order_count == 0 || level.total_size <= 0) {
//                     bids.erase(it_bid);
//                 }
//             }
//         } else if (order.side == 'A') {
//             int depth = 0;
//             for (const auto& level : asks) {
//                 if (std::abs(level.first - order.price) < 1e-9) {
//                     depth_out = depth;
//                     break;
//                 }
//                 depth++;
//             }

//             auto it_ask = asks.find(order.price);
//             if (it_ask != asks.end()) {
//                 LevelInfo& level = it_ask->second;
//                 level.total_size -= cancel_size;
//                 level.order_count--;
//                 if (level.order_count == 0 || level.total_size <= 0) {
//                     asks.erase(it_ask);
//                 }
//             }
//         }

//         return order.traded_size;
//     }

//     void get_top10(double* bid_px, int64_t* bid_sz, uint32_t* bid_ct,
//                    double* ask_px, int64_t* ask_sz, uint32_t* ask_ct) const {
//         std::fill_n(bid_px, 10, 0.0);
//         std::fill_n(bid_sz, 10, 0);
//         std::fill_n(bid_ct, 10, 0);
//         std::fill_n(ask_px, 10, 0.0);
//         std::fill_n(ask_sz, 10, 0);
//         std::fill_n(ask_ct, 10, 0);

//         int i = 0;
//         for (auto it = bids.begin(); it != bids.end() && i < 10; ++it, ++i) {
//             bid_px[i] = it->first;
//             bid_sz[i] = it->second.total_size;
//             bid_ct[i] = it->second.order_count;
//         }

//         i = 0;
//         for (auto it = asks.begin(); it != asks.end() && i < 10; ++it, ++i) {
//             ask_px[i] = it->first;
//             ask_sz[i] = it->second.total_size;
//             ask_ct[i] = it->second.order_count;
//         }
//     }
// };

// std::string normalize_line(const std::string& line) {
//     std::string result;
//     result.reserve(line.size());
//     for (char c : line) {
//         if (c != '\r') result += c;
//     }
//     if (!result.empty() && result.back() == '\n') {
//         result.pop_back();
//     }
//     return result;
// }

// bool is_header_line(const std::string& line) {
//     return line.find("ts_recv") != std::string::npos && 
//            line.find("ts_event") != std::string::npos;
// }

// int main(int argc, char* argv[]) {
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " mbo.csv" << std::endl;
//         return 1;
//     }

//     std::string input_filename = argv[1];
//     std::string output_filename = "mbp.csv";

//     std::ifstream in_file(input_filename, std::ios::binary);
//     if (!in_file.is_open()) {
//         std::cerr << "Failed to open input file: " << input_filename << std::endl;
//         return 1;
//     }

//     std::ofstream out_file(output_filename, std::ios::binary);
//     if (!out_file.is_open()) {
//         std::cerr << "Failed to open output file: " << output_filename << std::endl;
//         return 1;
//     }

//     // Write output header exactly once
//     out_file << "ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence,";
//     for (int i = 0; i < 10; i++) {
//         out_file << "bid_px_" << std::setfill('0') << std::setw(2) << i << ","
//                  << "bid_sz_" << std::setfill('0') << std::setw(2) << i << ","
//                  << "bid_ct_" << std::setfill('0') << std::setw(2) << i << ","
//                  << "ask_px_" << std::setfill('0') << std::setw(2) << i << ","
//                  << "ask_sz_" << std::setfill('0') << std::setw(2) << i << ","
//                  << "ask_ct_" << std::setfill('0') << std::setw(2) << i << ",";
//     }
//     out_file << "symbol,order_id\n";

//     OrderBook book;
//     std::string line;
//     bool is_first_data_row = true;

//     // Skip header row if present
//     std::getline(in_file, line);
//     line = normalize_line(line);
//     if (is_header_line(line)) {
//         // Header row found and skipped
//         is_first_data_row = true;
//     } else {
//         // First line is data, need to process it
//         std::istringstream first_line_ss(line);
//         // ... (rest of processing for first line) ...
//     }

//     while (std::getline(in_file, line)) {
//         line = normalize_line(line);
//         if (line.empty()) continue;

//         std::istringstream ss(line);
//         std::vector<std::string> fields;
//         std::string field;

//         while (std::getline(ss, field, ',')) {
//             fields.push_back(field);
//         }

//         if (fields.size() < 15) continue;

//         std::string ts_recv = fields[0];
//         std::string ts_event = fields[1];
//         std::string rtype = fields[2];
//         std::string publisher_id = fields[3];
//         std::string instrument_id = fields[4];
//         std::string action_str = fields[5];
//         char action = action_str.empty() ? '\0' : action_str[0];
//         std::string side_str = fields[6];
//         char side = side_str.empty() ? 'N' : side_str[0];
//         std::string price_str = fields[7];
//         std::string size_str = fields[8];
//         std::string order_id_str = fields[10];
//         std::string flags = fields[11];
//         std::string ts_in_delta = fields[12];
//         std::string sequence = fields[13];
//         std::string symbol = fields[14];

//         if (is_first_data_row) {
//             is_first_data_row = false;
//             book.clear();
//             if (action != 'R') {
//                 out_file << ts_recv << "," << ts_event << ",10,2,1108,R,N,0,0,0,8,0,0,";
//                 for (int i = 0; i < 10; i++) {
//                     out_file << "0.00,0,0,0.00,0,0,";
//                 }
//                 out_file << symbol << ",0\n";
//             }
//         }

//         if (action == 'R') {
//             book.clear();
//             out_file << ts_recv << "," << ts_event << ",10,2,1108,R,N,0,0,0,8,0,0,";
//             for (int i = 0; i < 10; i++) {
//                 out_file << "0.00,0,0,0.00,0,0,";
//             }
//             out_file << symbol << ",0\n";
//             continue;
//         }

//         if (side == 'N') {
//             continue;
//         }

//         double price = 0.0;
//         int64_t size_val = 0;
//         uint64_t order_id = 0;

//         try {
//             if (!price_str.empty()) price = std::stod(price_str);
//             if (!size_str.empty()) size_val = std::stoll(size_str);
//             if (!order_id_str.empty()) order_id = std::stoull(order_id_str);
//         } catch (...) {
//             continue;
//         }

//         if (action == 'A') {
//             book.add_order(order_id, side, price, size_val);
//             int depth = -1;
//             double bid_px[10], ask_px[10];
//             int64_t bid_sz[10], ask_sz[10];
//             uint32_t bid_ct[10], ask_ct[10];
//             book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

//             if (side == 'B') {
//                 for (int i = 0; i < 10; i++) {
//                     if (std::abs(bid_px[i] - price) < 1e-9) {
//                         depth = i;
//                         break;
//                     }
//                 }
//             } else if (side == 'A') {
//                 for (int i = 0; i < 10; i++) {
//                     if (std::abs(ask_px[i] - price) < 1e-9) {
//                         depth = i;
//                         break;
//                     }
//                 }
//             }

//             out_file << ts_recv << "," << ts_event << ",10,2,1108,A," << side << "," << depth << ",";
//             out_file << std::fixed << std::setprecision(2) << price << "," << size_val << "," << flags << "," << ts_in_delta << "," << sequence << ",";

//             for (int i = 0; i < 10; i++) {
//                 out_file << (bid_px[i] > 0 ? std::to_string(bid_px[i]) : "") << "," << bid_sz[i] << "," << bid_ct[i] << ",";
//                 out_file << (ask_px[i] > 0 ? std::to_string(ask_px[i]) : "") << "," << ask_sz[i] << "," << ask_ct[i] << ",";
//             }
//             out_file << symbol << "," << order_id << "\n";
//         } else if (action == 'T' || action == 'F') {
//             book.trade_order(order_id, size_val);
//         } else if (action == 'C') {
//             int depth = -1;
//             int64_t traded_size = book.cancel_order(order_id, size_val, depth);
//             double bid_px[10], ask_px[10];
//             int64_t bid_sz[10], ask_sz[10];
//             uint32_t bid_ct[10], ask_ct[10];
//             book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

//             if (traded_size > 0) {
//                 out_file << ts_recv << "," << ts_event << ",10,2,1108,T," << side << "," << depth << ",";
//                 out_file << std::fixed << std::setprecision(2) << price << "," << traded_size << "," << flags << "," << ts_in_delta << "," << sequence << ",";
//             } else {
//                 out_file << ts_recv << "," << ts_event << ",10,2,1108,C," << side << "," << depth << ",";
//                 out_file << std::fixed << std::setprecision(2) << price << "," << size_val << "," << flags << "," << ts_in_delta << "," << sequence << ",";
//             }

//             for (int i = 0; i < 10; i++) {
//                 out_file << (bid_px[i] > 0 ? std::to_string(bid_px[i]) : "") << "," << bid_sz[i] << "," << bid_ct[i] << ",";
//                 out_file << (ask_px[i] > 0 ? std::to_string(ask_px[i]) : "") << "," << ask_sz[i] << "," << ask_ct[i] << ",";
//             }
//             out_file << symbol << "," << order_id << "\n";
//         }
//     }

//     return 0;
// }


// // #include <iostream>
// // #include <fstream>
// // #include <sstream>
// // #include <vector>
// // #include <string>
// // #include <map>
// // #include <unordered_map>
// // #include <iomanip>
// // #include <algorithm>
// // #include <cmath>
// // #include <cassert>
// // #include <chrono>

// // // ================== ORDER BOOK CORE ==================
// // struct LevelInfo {
// //     int64_t total_size = 0;
// //     uint32_t order_count = 0;
// // };

// // struct Order {
// //     char side;
// //     double price;
// //     int64_t size;
// //     int64_t original_size;
// //     int64_t traded_size = 0;
// // };

// // using BidBook = std::map<double, LevelInfo, std::greater<double>>;
// // using AskBook = std::map<double, LevelInfo, std::less<double>>;

// // class OrderBook {
// // public:
// //     void clear() {
// //         bids.clear();
// //         asks.clear();
// //         orders.clear();
// //     }

// //     void add_order(uint64_t order_id, char side, double price, int64_t size) {
// //         orders[order_id] = Order{side, price, size, size, 0};

// //         if (side == 'B') {
// //             auto& level = bids[price];
// //             level.total_size += size;
// //             level.order_count++;
// //         } else if (side == 'A') {
// //             auto& level = asks[price];
// //             level.total_size += size;
// //             level.order_count++;
// //         }
// //     }

// //     void trade_order(uint64_t order_id, int64_t size) {
// //         auto it = orders.find(order_id);
// //         if (it == orders.end()) return;
        
// //         Order& order = it->second;
// //         int64_t trade_size = std::min(order.size, size);
// //         order.size -= trade_size;
// //         order.traded_size += trade_size;

// //         if (order.side == 'B') {
// //             auto it_bid = bids.find(order.price);
// //             if (it_bid != bids.end()) {
// //                 it_bid->second.total_size -= trade_size;
// //             }
// //         } else if (order.side == 'A') {
// //             auto it_ask = asks.find(order.price);
// //             if (it_ask != asks.end()) {
// //                 it_ask->second.total_size -= trade_size;
// //             }
// //         }
// //     }

// //     int64_t cancel_order(uint64_t order_id, int64_t size, int& depth_out) {
// //         auto it = orders.find(order_id);
// //         if (it == orders.end()) return -1;

// //         Order order = it->second;
// //         int64_t cancel_size = std::min(order.size, size);
// //         orders.erase(it);

// //         depth_out = -1;
// //         if (order.side == 'B') {
// //             int depth = 0;
// //             for (const auto& level : bids) {
// //                 if (std::abs(level.first - order.price) < 1e-9) {
// //                     depth_out = depth;
// //                     break;
// //                 }
// //                 depth++;
// //             }

// //             auto it_bid = bids.find(order.price);
// //             if (it_bid != bids.end()) {
// //                 LevelInfo& level = it_bid->second;
// //                 level.total_size -= cancel_size;
// //                 level.order_count--;
// //                 if (level.order_count == 0 || level.total_size <= 0) {
// //                     bids.erase(it_bid);
// //                 }
// //             }
// //         } else if (order.side == 'A') {
// //             int depth = 0;
// //             for (const auto& level : asks) {
// //                 if (std::abs(level.first - order.price) < 1e-9) {
// //                     depth_out = depth;
// //                     break;
// //                 }
// //                 depth++;
// //             }

// //             auto it_ask = asks.find(order.price);
// //             if (it_ask != asks.end()) {
// //                 LevelInfo& level = it_ask->second;
// //                 level.total_size -= cancel_size;
// //                 level.order_count--;
// //                 if (level.order_count == 0 || level.total_size <= 0) {
// //                     asks.erase(it_ask);
// //                 }
// //             }
// //         }

// //         return order.traded_size;
// //     }

// //     void get_top10(double* bid_px, int64_t* bid_sz, uint32_t* bid_ct,
// //                    double* ask_px, int64_t* ask_sz, uint32_t* ask_ct) const {
// //         std::fill_n(bid_px, 10, 0.0);
// //         std::fill_n(bid_sz, 10, 0);
// //         std::fill_n(bid_ct, 10, 0);
// //         std::fill_n(ask_px, 10, 0.0);
// //         std::fill_n(ask_sz, 10, 0);
// //         std::fill_n(ask_ct, 10, 0);

// //         int i = 0;
// //         for (auto it = bids.begin(); it != bids.end() && i < 10; ++it, ++i) {
// //             bid_px[i] = it->first;
// //             bid_sz[i] = it->second.total_size;
// //             bid_ct[i] = it->second.order_count;
// //         }

// //         i = 0;
// //         for (auto it = asks.begin(); it != asks.end() && i < 10; ++it, ++i) {
// //             ask_px[i] = it->first;
// //             ask_sz[i] = it->second.total_size;
// //             ask_ct[i] = it->second.order_count;
// //         }
// //     }

// //     // Test accessors
// //     const BidBook& get_bids() const { return bids; }
// //     const AskBook& get_asks() const { return asks; }
// //     const std::unordered_map<uint64_t, Order>& get_orders() const { return orders; }

// // private:
// //     BidBook bids;
// //     AskBook asks;
// //     std::unordered_map<uint64_t, Order> orders;
// // };

// // // ================== UNIT TEST FRAMEWORK ==================
// // class OrderBookTester {
// // public:
// //     void run_all_tests() {
// //         test_empty_book();
// //         test_single_bid();
// //         test_single_ask();
// //         test_add_multiple_orders();
// //         test_trade_execution();
// //         test_cancel_order();
// //         test_trade_cancel_sequence();
// //         test_price_level_aggregation();
// //         test_top10_calculation();
// //         test_performance();

// //         std::cout << "\n=== TEST RESULTS ==="
// //                   << "\nPassed: " << tests_passed
// //                   << "\nFailed: " << tests_failed
// //                   << "\n====================" << std::endl;
// //     }

// // private:
// //     int tests_passed = 0;
// //     int tests_failed = 0;

// //     void test_empty_book() {
// //         OrderBook book;
// //         double bid_px[10], ask_px[10];
// //         int64_t bid_sz[10], ask_sz[10];
// //         uint32_t bid_ct[10], ask_ct[10];
        
// //         book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);
        
// //         for (int i = 0; i < 10; i++) {
// //             if (bid_px[i] != 0.0 || bid_sz[i] != 0 || bid_ct[i] != 0 ||
// //                 ask_px[i] != 0.0 || ask_sz[i] != 0 || ask_ct[i] != 0) {
// //                 return fail_test("test_empty_book");
// //             }
// //         }
// //         pass_test("test_empty_book");
// //     }

// //     void test_single_bid() {
// //         OrderBook book;
// //         book.add_order(1, 'B', 100.0, 50);
        
// //         double bid_px[10], ask_px[10];
// //         int64_t bid_sz[10], ask_sz[10];
// //         uint32_t bid_ct[10], ask_ct[10];
// //         book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);
        
// //         if (bid_px[0] != 100.0 || bid_sz[0] != 50 || bid_ct[0] != 1) {
// //             return fail_test("test_single_bid");
// //         }
// //         pass_test("test_single_bid");
// //     }

// //     void test_single_ask() {
// //         OrderBook book;
// //         book.add_order(1, 'A', 101.0, 30);
        
// //         double bid_px[10], ask_px[10];
// //         int64_t bid_sz[10], ask_sz[10];
// //         uint32_t bid_ct[10], ask_ct[10];
// //         book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);
        
// //         if (ask_px[0] != 101.0 || ask_sz[0] != 30 || ask_ct[0] != 1) {
// //             return fail_test("test_single_ask");
// //         }
// //         pass_test("test_single_ask");
// //     }

// //     void test_add_multiple_orders() {
// //         OrderBook book;
// //         book.add_order(1, 'B', 100.0, 50);
// //         book.add_order(2, 'B', 99.5, 30);
// //         book.add_order(3, 'A', 101.0, 40);
// //         book.add_order(4, 'A', 102.0, 20);
        
// //         double bid_px[10], ask_px[10];
// //         int64_t bid_sz[10], ask_sz[10];
// //         uint32_t bid_ct[10], ask_ct[10];
// //         book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);
        
// //         if (bid_px[0] != 100.0 || bid_sz[0] != 50 || bid_ct[0] != 1 ||
// //             bid_px[1] != 99.5 || bid_sz[1] != 30 || bid_ct[1] != 1 ||
// //             ask_px[0] != 101.0 || ask_sz[0] != 40 || ask_ct[0] != 1 ||
// //             ask_px[1] != 102.0 || ask_sz[1] != 20 || ask_ct[1] != 1) {
// //             return fail_test("test_add_multiple_orders");
// //         }
// //         pass_test("test_add_multiple_orders");
// //     }

// //     void test_trade_execution() {
// //         OrderBook book;
// //         book.add_order(1, 'B', 100.0, 50);
// //         book.trade_order(1, 30);
        
// //         const auto& orders = book.get_orders();
// //         auto it = orders.find(1);
        
// //         if (it == orders.end() || it->second.size != 20 || it->second.traded_size != 30) {
// //             return fail_test("test_trade_execution");
// //         }
// //         pass_test("test_trade_execution");
// //     }

// //     void test_cancel_order() {
// //         OrderBook book;
// //         book.add_order(1, 'B', 100.0, 50);
// //         int depth = -1;
// //         int64_t traded_size = book.cancel_order(1, 50, depth);
        
// //         if (traded_size != 0 || depth != 0 || !book.get_orders().empty()) {
// //             return fail_test("test_cancel_order");
// //         }
// //         pass_test("test_cancel_order");
// //     }

// //     void test_trade_cancel_sequence() {
// //         OrderBook book;
// //         book.add_order(1, 'B', 100.0, 50);
// //         book.trade_order(1, 30);
// //         int depth = -1;
// //         int64_t traded_size = book.cancel_order(1, 20, depth);
        
// //         if (traded_size != 30 || depth != -1 || !book.get_orders().empty()) {
// //             return fail_test("test_trade_cancel_sequence");
// //         }
// //         pass_test("test_trade_cancel_sequence");
// //     }

// //     void test_price_level_aggregation() {
// //         OrderBook book;
// //         book.add_order(1, 'B', 100.0, 50);
// //         book.add_order(2, 'B', 100.0, 30);
// //         book.add_order(3, 'A', 101.0, 40);
// //         book.add_order(4, 'A', 101.0, 20);
        
// //         double bid_px[10], ask_px[10];
// //         int64_t bid_sz[10], ask_sz[10];
// //         uint32_t bid_ct[10], ask_ct[10];
// //         book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);
        
// //         if (bid_sz[0] != 80 || bid_ct[0] != 2 || 
// //             ask_sz[0] != 60 || ask_ct[0] != 2) {
// //             return fail_test("test_price_level_aggregation");
// //         }
// //         pass_test("test_price_level_aggregation");
// //     }

// //     void test_top10_calculation() {
// //         OrderBook book;
        
// //         // Add 15 bids
// //         for (int i = 1; i <= 15; i++) {
// //             book.add_order(i, 'B', 100.0 - i*0.1, 10);
// //         }
        
// //         // Add 15 asks
// //         for (int i = 16; i <= 30; i++) {
// //             book.add_order(i, 'A', 100.0 + (i-15)*0.1, 10);
// //         }
        
// //         double bid_px[10], ask_px[10];
// //         int64_t bid_sz[10], ask_sz[10];
// //         uint32_t bid_ct[10], ask_ct[10];
// //         book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);
        
// //         // Verify only top 10 are shown
// //         if (bid_px[9] != 99.1 || ask_px[9] != 101.0 || 
// //             bid_px[10] != 0.0 || ask_px[10] != 0.0) {
// //             return fail_test("test_top10_calculation");
// //         }
// //         pass_test("test_top10_calculation");
// //     }

// //     void test_performance() {
// //         OrderBook book;
// //         const int num_orders = 1000000;
        
// //         auto start = std::chrono::high_resolution_clock::now();
        
// //         // Add orders
// //         for (uint64_t i = 1; i <= num_orders; i++) {
// //             double price = 100.0 + (i % 100) * 0.01;
// //             char side = (i % 2 == 0) ? 'B' : 'A';
// //             book.add_order(i, side, price, 100);
// //         }
        
// //         // Process trades
// //         for (uint64_t i = 1; i <= num_orders; i++) {
// //             if (i % 10 == 0) { // Trade 10% of orders
// //                 book.trade_order(i, 50);
// //             }
// //         }
        
// //         // Cancel orders
// //         for (uint64_t i = 1; i <= num_orders; i++) {
// //             if (i % 5 == 0) { // Cancel 20% of orders
// //                 int depth;
// //                 book.cancel_order(i, 100, depth);
// //             }
// //         }
        
// //         auto end = std::chrono::high_resolution_clock::now();
// //         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
// //         double orders_per_sec = num_orders * 3 / (duration / 1000.0);
        
// //         std::cout << "Performance test: " << num_orders << " orders processed in "
// //                   << duration << " ms (" << orders_per_sec << " ops/sec)" << std::endl;
        
// //         if (orders_per_sec > 500000) { // Minimum performance threshold
// //             pass_test("test_performance");
// //         } else {
// //             fail_test("test_performance");
// //         }
// //     }

// //     void pass_test(const std::string& test_name) {
// //         std::cout << "[PASS] " << test_name << std::endl;
// //         tests_passed++;
// //     }

// //     void fail_test(const std::string& test_name) {
// //         std::cerr << "[FAIL] " << test_name << std::endl;
// //         tests_failed++;
// //     }
// // };

// // // ================== CSV PROCESSING ==================
// // std::string normalize_line(const std::string& line) {
// //     std::string result;
// //     result.reserve(line.size());
// //     for (char c : line) {
// //         if (c != '\r') result += c;
// //     }
// //     if (!result.empty() && result.back() == '\n') {
// //         result.pop_back();
// //     }
// //     return result;
// // }

// // bool is_header_line(const std::string& line) {
// //     return line.find("ts_recv") != std::string::npos && 
// //            line.find("ts_event") != std::string::npos;
// // }

// // void process_mbo_file(const std::string& input_filename, const std::string& output_filename) {
// //     std::ifstream in_file(input_filename, std::ios::binary);
// //     if (!in_file.is_open()) {
// //         std::cerr << "Failed to open input file: " << input_filename << std::endl;
// //         return;
// //     }

// //     std::ofstream out_file(output_filename, std::ios::binary);
// //     if (!out_file.is_open()) {
// //         std::cerr << "Failed to open output file: " << output_filename << std::endl;
// //         return;
// //     }

// //     // Write output header
// //     out_file << "ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence,";
// //     for (int i = 0; i < 10; i++) {
// //         out_file << "bid_px_" << std::setfill('0') << std::setw(2) << i << ","
// //                  << "bid_sz_" << std::setfill('0') << std::setw(2) << i << ","
// //                  << "bid_ct_" << std::setfill('0') << std::setw(2) << i << ","
// //                  << "ask_px_" << std::setfill('0') << std::setw(2) << i << ","
// //                  << "ask_sz_" << std::setfill('0') << std::setw(2) << i << ","
// //                  << "ask_ct_" << std::setfill('0') << std::setw(2) << i << ",";
// //     }
// //     out_file << "symbol,order_id\n";

// //     OrderBook book;
// //     std::string line;
// //     bool header_skipped = false;
// //     bool book_cleared = false;

// //     while (std::getline(in_file, line)) {
// //         line = normalize_line(line);
// //         if (line.empty()) continue;

// //         // Skip header row if present
// //         if (!header_skipped && is_header_line(line)) {
// //             header_skipped = true;
// //             continue;
// //         }
// //         header_skipped = true;

// //         std::vector<std::string> fields;
// //         std::istringstream ss(line);
// //         std::string field;

// //         while (std::getline(ss, field, ',')) {
// //             fields.push_back(field);
// //         }

// //         // Ensure we have at least 15 fields
// //         if (fields.size() < 15) {
// //             std::cerr << "Skipping malformed line: " << line << std::endl;
// //             continue;
// //         }

// //         std::string ts_recv = fields[0];
// //         std::string ts_event = fields[1];
// //         std::string rtype = fields[2];
// //         std::string publisher_id = fields[3];
// //         std::string instrument_id = fields[4];
// //         std::string action_str = fields[5];
// //         char action = action_str.empty() ? '\0' : action_str[0];
// //         std::string side_str = fields[6];
// //         char side = side_str.empty() ? 'N' : side_str[0];
// //         std::string price_str = fields[7];
// //         std::string size_str = fields[8];
// //         std::string channel_id = fields[9];  // Not used in output
// //         std::string order_id_str = fields[10];
// //         std::string flags = fields[11];
// //         std::string ts_in_delta = fields[12];
// //         std::string sequence = fields[13];
// //         std::string symbol = fields[14];

// //         // Handle book reset
// //         if (!book_cleared) {
// //             book_cleared = true;
// //             if (action != 'R') {
// //                 out_file << ts_recv << "," << ts_event << ",10,2,1108,R,N,0,0,0,8,0,0,";
// //                 for (int i = 0; i < 10; i++) {
// //                     out_file << "0.00,0,0,0.00,0,0,";
// //                 }
// //                 out_file << symbol << ",0\n";
// //             }
// //         }

// //         if (action == 'R') {
// //             book.clear();
// //             out_file << ts_recv << "," << ts_event << ",10,2,1108,R,N,0,0,0,8,0,0,";
// //             for (int i = 0; i < 10; i++) {
// //                 out_file << "0.00,0,0,0.00,0,0,";
// //             }
// //             out_file << symbol << ",0\n";
// //             continue;
// //         }

// //         if (side == 'N') {
// //             continue;
// //         }

// //         double price = 0.0;
// //         int64_t size_val = 0;
// //         uint64_t order_id = 0;

// //         try {
// //             if (!price_str.empty()) price = std::stod(price_str);
// //             if (!size_str.empty()) size_val = std::stoll(size_str);
// //             if (!order_id_str.empty()) order_id = std::stoull(order_id_str);
// //         } catch (...) {
// //             std::cerr << "Error parsing numeric fields: " << line << std::endl;
// //             continue;
// //         }

// //         if (action == 'A') {
// //             book.add_order(order_id, side, price, size_val);
// //             int depth = -1;
// //             double bid_px[10], ask_px[10];
// //             int64_t bid_sz[10], ask_sz[10];
// //             uint32_t bid_ct[10], ask_ct[10];
// //             book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

// //             if (side == 'B') {
// //                 for (int i = 0; i < 10; i++) {
// //                     if (std::abs(bid_px[i] - price) < 1e-9) {
// //                         depth = i;
// //                         break;
// //                     }
// //                 }
// //             } else if (side == 'A') {
// //                 for (int i = 0; i < 10; i++) {
// //                     if (std::abs(ask_px[i] - price) < 1e-9) {
// //                         depth = i;
// //                         break;
// //                     }
// //                 }
// //             }

// //             out_file << ts_recv << "," << ts_event << ",10,2,1108,A," << side << "," << depth << ",";
// //             out_file << std::fixed << std::setprecision(2) << price << "," << size_val << ",";
// //             out_file << flags << "," << ts_in_delta << "," << sequence << ",";

// //             for (int i = 0; i < 10; i++) {
// //                 if (bid_px[i] > 0) out_file << std::fixed << std::setprecision(2) << bid_px[i];
// //                 out_file << "," << bid_sz[i] << "," << bid_ct[i] << ",";
                
// //                 if (ask_px[i] > 0) out_file << std::fixed << std::setprecision(2) << ask_px[i];
// //                 out_file << "," << ask_sz[i] << "," << ask_ct[i] << ",";
// //             }
// //             out_file << symbol << "," << order_id << "\n";
// //         } 
// //         else if (action == 'T' || action == 'F') {
// //             book.trade_order(order_id, size_val);
// //         } 
// //         else if (action == 'C') {
// //             int depth = -1;
// //             int64_t traded_size = book.cancel_order(order_id, size_val, depth);
// //             double bid_px[10], ask_px[10];
// //             int64_t bid_sz[10], ask_sz[10];
// //             uint32_t bid_ct[10], ask_ct[10];
// //             book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

// //             if (traded_size > 0) {
// //                 out_file << ts_recv << "," << ts_event << ",10,2,1108,T," << side << "," << depth << ",";
// //                 out_file << std::fixed << std::setprecision(2) << price << "," << traded_size << ",";
// //             } else {
// //                 out_file << ts_recv << "," << ts_event << ",10,2,1108,C," << side << "," << depth << ",";
// //                 out_file << std::fixed << std::setprecision(2) << price << "," << size_val << ",";
// //             }
// //             out_file << flags << "," << ts_in_delta << "," << sequence << ",";

// //             for (int i = 0; i < 10; i++) {
// //                 if (bid_px[i] > 0) out_file << std::fixed << std::setprecision(2) << bid_px[i];
// //                 out_file << "," << bid_sz[i] << "," << bid_ct[i] << ",";
                
// //                 if (ask_px[i] > 0) out_file << std::fixed << std::setprecision(2) << ask_px[i];
// //                 out_file << "," << ask_sz[i] << "," << ask_ct[i] << ",";
// //             }
// //             out_file << symbol << "," << order_id << "\n";
// //         }
// //     }
// // }

// // // ================== MAIN APPLICATION ==================
// // int main(int argc, char* argv[]) {
// //     if (argc == 2 && std::string(argv[1]) == "-test") {
// //         std::cout << "Running unit tests...\n" << std::endl;
// //         OrderBookTester tester;
// //         tester.run_all_tests();
// //         return 0;
// //     }

// //     if (argc < 2) {
// //         std::cerr << "Usage: " << argv[0] << " [-test] | [mbo.csv]" << std::endl;
// //         std::cerr << "Options:" << std::endl;
// //         std::cerr << "  -test    Run unit tests" << std::endl;
// //         std::cerr << "  mbo.csv  Process MBO file and generate MBP output" << std::endl;
// //         return 1;
// //     }

// //     std::string input_filename = argv[1];
// //     std::string output_filename = "mbp.csv";
    
// //     auto start = std::chrono::high_resolution_clock::now();
// //     process_mbo_file(input_filename, output_filename);
// //     auto end = std::chrono::high_resolution_clock::now();
    
// //     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
// //     std::cout << "Processed " << input_filename << " in " << duration << " ms" << std::endl;
    
// //     return 0;
// // }


#include "orderbook.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>

std::string normalize_line(const std::string& line) {
    std::string result;
    result.reserve(line.size());
    for (char c : line) {
        if (c != '\r') result += c;
    }
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

bool is_header_line(const std::string& line) {
    return line.find("ts_recv") != std::string::npos && 
           line.find("ts_event") != std::string::npos;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " mbo.csv" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = "mbp.csv";

    std::ifstream in_file(input_filename, std::ios::binary);
    if (!in_file.is_open()) {
        std::cerr << "Failed to open input file: " << input_filename << std::endl;
        return 1;
    }

    std::ofstream out_file(output_filename, std::ios::binary);
    if (!out_file.is_open()) {
        std::cerr << "Failed to open output file: " << output_filename << std::endl;
        return 1;
    }

    out_file << "ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence,";
    for (int i = 0; i < 10; i++) {
        out_file << "bid_px_" << std::setfill('0') << std::setw(2) << i << ","
                 << "bid_sz_" << std::setfill('0') << std::setw(2) << i << ","
                 << "bid_ct_" << std::setfill('0') << std::setw(2) << i << ","
                 << "ask_px_" << std::setfill('0') << std::setw(2) << i << ","
                 << "ask_sz_" << std::setfill('0') << std::setw(2) << i << ","
                 << "ask_ct_" << std::setfill('0') << std::setw(2) << i << ",";
    }
    out_file << "symbol,order_id\n";

    OrderBook book;
    std::string line;
    bool is_first_data_row = true;

    std::getline(in_file, line);
    line = normalize_line(line);
    if (is_header_line(line)) {
        is_first_data_row = true;
    } else {
        std::istringstream first_line_ss(line);
    }

    while (std::getline(in_file, line)) {
        line = normalize_line(line);
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::vector<std::string> fields;
        std::string field;

        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() < 15) continue;

        std::string ts_recv = fields[0];
        std::string ts_event = fields[1];
        std::string rtype = fields[2];
        std::string publisher_id = fields[3];
        std::string instrument_id = fields[4];
        std::string action_str = fields[5];
        char action = action_str.empty() ? '\0' : action_str[0];
        std::string side_str = fields[6];
        char side = side_str.empty() ? 'N' : side_str[0];
        std::string price_str = fields[7];
        std::string size_str = fields[8];
        std::string order_id_str = fields[10];
        std::string flags = fields[11];
        std::string ts_in_delta = fields[12];
        std::string sequence = fields[13];
        std::string symbol = fields[14];

        if (is_first_data_row) {
            is_first_data_row = false;
            book.clear();
            if (action != 'R') {
                out_file << ts_recv << "," << ts_event << ",10,2,1108,R,N,0,0,0,8,0,0,";
                for (int i = 0; i < 10; i++) {
                    out_file << "0.00,0,0,0.00,0,0,";
                }
                out_file << symbol << ",0\n";
            }
        }

        if (action == 'R') {
            book.clear();
            out_file << ts_recv << "," << ts_event << ",10,2,1108,R,N,0,0,0,8,0,0,";
            for (int i = 0; i < 10; i++) {
                out_file << "0.00,0,0,0.00,0,0,";
            }
            out_file << symbol << ",0\n";
            continue;
        }

        if (side == 'N') {
            continue;
        }

        double price = 0.0;
        int64_t size_val = 0;
        uint64_t order_id = 0;

        try {
            if (!price_str.empty()) price = std::stod(price_str);
            if (!size_str.empty()) size_val = std::stoll(size_str);
            if (!order_id_str.empty()) order_id = std::stoull(order_id_str);
        } catch (...) {
            continue;
        }

        if (action == 'A') {
            book.add_order(order_id, side, price, size_val);
            int depth = -1;
            double bid_px[10], ask_px[10];
            int64_t bid_sz[10], ask_sz[10];
            uint32_t bid_ct[10], ask_ct[10];
            book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

            if (side == 'B') {
                for (int i = 0; i < 10; i++) {
                    if (std::abs(bid_px[i] - price) < 1e-9) {
                        depth = i;
                        break;
                    }
                }
            } else if (side == 'A') {
                for (int i = 0; i < 10; i++) {
                    if (std::abs(ask_px[i] - price) < 1e-9) {
                        depth = i;
                        break;
                    }
                }
            }

            out_file << ts_recv << "," << ts_event << ",10,2,1108,A," << side << "," << depth << ",";
            out_file << std::fixed << std::setprecision(2) << price << "," << size_val << "," << flags << "," << ts_in_delta << "," << sequence << ",";

            for (int i = 0; i < 10; i++) {
                out_file << (bid_px[i] > 0 ? std::to_string(bid_px[i]) : "") << "," << bid_sz[i] << "," << bid_ct[i] << ",";
                out_file << (ask_px[i] > 0 ? std::to_string(ask_px[i]) : "") << "," << ask_sz[i] << "," << ask_ct[i] << ",";
            }
            out_file << symbol << "," << order_id << "\n";
        } else if (action == 'T' || action == 'F') {
            book.trade_order(order_id, size_val);
        } else if (action == 'C') {
            int depth = -1;
            int64_t traded_size = book.cancel_order(order_id, size_val, depth);
            double bid_px[10], ask_px[10];
            int64_t bid_sz[10], ask_sz[10];
            uint32_t bid_ct[10], ask_ct[10];
            book.get_top10(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);

            if (traded_size > 0) {
                out_file << ts_recv << "," << ts_event << ",10,2,1108,T," << side << "," << depth << ",";
                out_file << std::fixed << std::setprecision(2) << price << "," << traded_size << "," << flags << "," << ts_in_delta << "," << sequence << ",";
            } else {
                out_file << ts_recv << "," << ts_event << ",10,2,1108,C," << side << "," << depth << ",";
                out_file << std::fixed << std::setprecision(2) << price << "," << size_val << "," << flags << "," << ts_in_delta << "," << sequence << ",";
            }

            for (int i = 0; i < 10; i++) {
                out_file << (bid_px[i] > 0 ? std::to_string(bid_px[i]) : "") << "," << bid_sz[i] << "," << bid_ct[i] << ",";
                out_file << (ask_px[i] > 0 ? std::to_string(ask_px[i]) : "") << "," << ask_sz[i] << "," << ask_ct[i] << ",";
            }
            out_file << symbol << "," << order_id << "\n";
        }
    }

    return 0;
}