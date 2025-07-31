# Order Book Reconstruction for High-Frequency Trading (HFT)

This project implements a high-performance C++ solution to reconstruct a Market By Price (MBP-10) order book from Market By Order (MBO) data, optimized for high-frequency trading (HFT) environments. It processes an input `mbo.csv` file and generates an `mbp.csv` output, strictly adhering to the provided sample format.

---

## Features

- **High Performance**: Optimized for speed and low latency, critical for HFT.  
- **Correctness**: Handles special operations (e.g., combining T/F/C actions) as specified.  
- **Unit Tests**: Includes optional tests using standard C++ asserts for reliability.  
- **Build System**: Uses a Makefile for easy compilation and execution.

---

## Optimizations

To achieve HFT-level performance, the following optimizations were implemented:

### Efficient Data Structures

- **Vectors for Price Levels**:  
  Uses `std::vector<std::pair<int, LevelInfo>>` for bids and asks, sorted by price (descending for bids, ascending for asks). Vectors provide contiguous memory and better cache locality compared to `std::map`, reducing access times.

- **Integer Price Levels**:  
  Prices are scaled to integers (e.g., price × 100 for 0.01 tick size) to eliminate floating-point operations, enabling faster comparisons and sorting.

- **Unordered Map for Orders**:  
  Employs `std::unordered_map<uint64_t, Order>` for O(1) average-time order lookups and updates.

### Memory Management

- **Pre-allocation**:  
  Vectors are pre-allocated with a capacity of 1000 price levels, and the orders map reserves space for 10,000 entries, minimizing dynamic allocations and hash table resizing.

- **Avoiding Reallocations**:  
  Ensures predictable memory usage, critical for low-latency environments.

### Fast CSV Parsing

- **In-place Processing**:  
  Parses CSV lines using `std::strtoull` and `std::strtod` directly on C-strings, avoiding `std::istringstream` and reducing string copying overhead.

### O(1) Top-10 Access

- Sorted vectors allow direct retrieval of the top 10 bid and ask levels, optimizing MBP-10 snapshot generation.

---

## Usage Instructions

The project includes a Makefile for straightforward compilation and execution.

### Build the Main Program
This generates the executable `reconstruction`.

### Run the Program
```
./reconstruction mbo.csv
```
This processes `mbo.csv` and outputs `mbp.csv` in the current directory.

### Build and Run Unit Tests
```
make test_orderbook
./test_orderbook
```
This compiles and runs unit tests, verifying the `OrderBook` class functionality.

### Clean Build Artifacts
```
make clean
```
This removes the executables (`reconstruction` and `test_orderbook`).

---

## Design Decisions

- **Vectors vs. Maps**:  
  Chose vectors over `std::map` for price levels due to superior cache efficiency, a key factor in HFT where microseconds matter.

- **Integer Prices**:  
  Scaled prices to integers to avoid floating-point precision issues and accelerate operations like sorting and comparisons.

- **Single-Threaded Design**:  
  Optimized for sequential CSV processing as per the task. For live HFT, it can be extended with lock-free structures or multi-threading.

- **Fixed Tick Size**:  
  Assumed a 0.01 tick size (configurable via `PRICE_SCALE`), simplifying price handling while maintaining flexibility.

---

## Special Operations

The code implements the following task-specific requirements:

- **Ignoring Initial 'R' Action**:  
  The first row with action `'R'` (clear) is ignored, initializing an empty order book.

- **Combining 'T', 'F', and 'C' Actions**:  
  Sequences of `'T'` (trade), `'F'` (fill), and `'C'` (cancel) are consolidated into a single `'T'` action in the MBP-10 output, reflecting the book’s state change (e.g., a trade on the ask side updates the bid side).

- **Handling 'T' with Side 'N'**:  
  Skips `'T'` actions with side `'N'`, leaving the order book unchanged.

---

## Potential Further Optimizations

- **Custom Allocator**:  
  Implement a slab or pool allocator to further reduce memory allocation overhead, enhancing performance in ultra-low-latency scenarios.

- **Profiling**:  
  Use tools like `perf` or `gprof` to identify bottlenecks (e.g., vector insertions or hash table lookups) for targeted optimizations.

- **Parallelization**:  
  Extend to multi-threaded or lock-free designs for processing multiple instruments or real-time data streams in a live HFT system.

- **SIMD Instructions**:  
  Leverage SIMD for vectorized price level updates if profiling indicates arithmetic bottlenecks.

---

## Repository Structure
```
.
├── orderbook.h # Header file with OrderBook class
├── main.cpp # Main program for MBP-10 reconstruction
├── test_orderbook.cpp # Unit tests for OrderBook
├── Makefile # Build instructions
└── readme.md # This file
```

---

## Notes

- The code is designed to compile with a standard C++11 compiler (e.g., `g++`).
- Unit tests use only standard C++ libraries (`<cassert>`, `<iostream>`), ensuring no external dependencies.
- Upload the code to a private GitHub repository and include the link in your submission, as per the task requirements.

---

This implementation reflects a deep focus on performance, leveraging HFT principles to deliver a robust and efficient solution.


