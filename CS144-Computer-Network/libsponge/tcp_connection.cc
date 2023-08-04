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

size_t TCPConnection::time_since_last_segment_received() const { return _sender.time_since_last_segment_received(); }

void TCPConnection::segment_received(const TCPSegment &seg) { 
    TCPHeader header = seg.header();
    
    if (header.rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        return;
    }

    _receiver.segment_received(seg);

    if (header.ack) {
        recv_ackno = header.seqno;
        recv_window_size = header.win;
    }

    if (seg.length_in_sequence_space() >= 0) {
        // send empty segment 
        _sender.send_empty_segment();
        queue<TCPSegment> seg_out = _sender.segments_out();
        TCPSegment seg_to_sent = seg_out.front();
    }
    // 判断接收的seqno是否有效
    if ()
 }

bool TCPConnection::active() const {
    return (!_sender.stream_in().input_ended() && !_receiver.stream_out().input_ended());
}

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

    WrappingInt32 ackno = recv_ackno;
    size_t win_size = recv_window_size;

    // receiver_state == LISTEN
    if (ackno.raw_value() == 0) {
        return 0;
    }

    _sender.ack_received(ackno, win_size);
    // 没有创建syn包
    if (_sender.segments_out().empty()) {
        return 0;
    }
    // TCPSender wants sent
    queue<TCPSegment> seg_out = _sender.segments_out();
    TCPSegment seg_to_sent = seg_out.front();

    // need to set ack
    if (_receiver.ackno().has_value()) {
        seg_to_sent.header().ack = true;
        seg_to_sent.header().ackno = _receiver.ackno().value();
        seg_to_sent.header().win = min((uint64_t)0xffff, win_size);
    }

    _segments_out.push(seg_to_sent);


    // 计算seg.payload得出写入了多少
    return seg_to_sent.payload().size();
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    _sender.tick(ms_since_last_tick);

    if (_sender.consecutive_retransmissions() >= TCPConfig::MAX_RETX_ATTEMPTS) {
        _sender.send_empty_segment();
        queue<TCPSegment> seg_out = _sender.segments_out();
        TCPSegment seg_to_sent = seg_out.front();
        seg_to_sent.header().rst = true;
    }
}

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
