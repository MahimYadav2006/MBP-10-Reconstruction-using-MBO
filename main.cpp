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