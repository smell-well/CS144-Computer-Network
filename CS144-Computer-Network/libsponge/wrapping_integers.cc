#include "wrapping_integers.hh"
#include <iostream>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;




//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint32_t base = n & 0xffffffff;
    return isn + base;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {

    // cout << "n: " << n.raw_value() << endl;

    uint64_t MOD = 1ul << 32;
    uint32_t x = n - isn + MOD;

    // cout << "x: " << x << endl;
    uint64_t base = checkpoint & 0xFFFFFFFF00000000;
    uint32_t y = checkpoint & 0xffffffff;
    if (x < y) {
        uint32_t left = y - x;
        uint32_t right = x - y + MOD;
        return left < right ? base + x : base + MOD + x;
    } else {
        uint32_t left = x - y;
        uint32_t right = MOD - (x - y);
        // cout << "x: " << x << " y: " << y << endl;
        // cout << "left: " << left << " right: " << right << endl;
        // cout << "base: " << base << " MOD: " << MOD <<endl;
        if (left > right && base >= MOD) {
            
            return base - MOD + x;
        }
        return base + x;
    }

}
