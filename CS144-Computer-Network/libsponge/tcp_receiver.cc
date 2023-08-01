#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();

    WrappingInt32 data_seqno = header.seqno;
    
    // 监听状态，没有syn则直接丢弃
    if (!_initial_state && !header.syn) {
        return;
    }

    // 需要设置isn
    if (header.syn) {
        ISN = header.seqno;
        _initial_state = true;
        data_seqno = data_seqno + 1;
    }

    // 处理数据
    uint64_t checkpoint = _reassembler.first_unassembled_index() + 1;
    uint64_t index = unwrap(data_seqno, ISN, checkpoint) - 1;
    string data = seg.payload().copy();
    bool eof = header.fin;
    _reassembler.push_substring(data, index, eof);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_initial_state) {
        return optional<WrappingInt32>{};
    }
    
    uint64_t first_unassembled_idx = _reassembler.first_unassembled_index() + 1;
    if (_reassembler.stream_out().input_ended()) {
        first_unassembled_idx += 1;
    }
    return optional<WrappingInt32>{wrap(first_unassembled_idx, ISN)};
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
