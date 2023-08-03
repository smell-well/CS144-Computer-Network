#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _cfg.send_capacity - _sender.bytes_in_flight(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return {}; }

void TCPConnection::segment_received(const TCPSegment &seg) { DUMMY_CODE(seg); }

bool TCPConnection::active() const { return {}; }

size_t TCPConnection::write(const string &data) {
    // 这个就是真实的发送包
    // TODO:如果是 ack 包，需要加上ack标记
    // state != syn_acked
    size_t next_seqno_abso = _sender.next_seqno_absolute();
    ByteStream stream_in = _sender.stream_in();
    bool syn_acked_0 = next_seqno_abso > 0 && !stream_in.eof();
    bool syn_acked_1 = stream_in.eof() && next_seqno_abso < stream_in.bytes_written() + 2;
    if (!syn_acked_0 && !syn_acked_1) {
        return 0;
    }

    // _sender 已经停止写入
    if (stream_in.input_ended()) {
        return 0;
    }
    stream_in.write(data);

    optional<WrappingInt32> ackno = _receiver.ackno();
    size_t win_size = _receiver.window_size();

    // receiver_state == LISTEN
    if (!ackno.has_value()) {
        return 0;
    }

    _sender.ack_received(ackno.value(), win_size);
    // 没有创建syn包
    if (_sender.segments_out().empty()) {
        return 0;
    }
    // TCPSender wants sent
    queue<TCPSegment> seg_out = _sender.segments_out();
    TCPSegment seg_to_sent = seg_out.front();
    _segments_out.push(seg_to_sent);


    // 计算seg.payload得出写入了多少
    return seg_to_sent.payload().size();
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

void TCPConnection::end_input_stream() {
    ByteStream stream_in = _sender.stream_in();
    stream_in.end_input();
}

void TCPConnection::connect() {
    // state != closed
    if (_sender.next_seqno_absolute() != 0) {
        return;
    }
    _sender.fill_window();
    // 没有创建syn包
    if (_sender.segments_out().empty()) {
        return;
    }
    // TCPSender wants sent
    queue<TCPSegment> seg_out = _sender.segments_out();
    TCPSegment start_connect = seg_out.front();
    _segments_out.push(start_connect);
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
