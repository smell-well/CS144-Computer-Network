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


// 这个函数只负责 ack，ackno，win 和 payload部分
void TCPConnection::send_segment(queue<TCPSegment> &segment_prepare) {
    // TODO:只有当close状态才不能发送包


    while (!segment_prepare.empty()) {
        TCPSegment seg = segment_prepare.front();
        segment_prepare.pop();
        optional<WrappingInt32> ackno = _receiver.ackno();
        if (ackno.has_value()) {
            seg.header().ack = true;
            seg.header().ackno = ackno.value();
            seg.header().win = max(static_cast<uint64_t>((0xffff)), _receiver.window_size());
        }
        // cout << seg.header().summary();
        // cout << " with " << seg.payload().size() << " bytes\n";
        _segments_out.push(seg);
    }

}

void TCPConnection::segment_received(const TCPSegment &seg) { 
    TCPHeader header = seg.header();
    // cout << "func: segment_received" << endl;

    if (header.rst) {
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
    }


    _receiver.segment_received(seg);

    
    // 对接收的包做出相应的行为
    // cout << "seg received:" << endl;
    // cout << seg.header().summary();
    // cout << " with " << seg.payload().size() << " bytes\n";
    // cout << "check ack: " << header.ack << endl;
    if (header.ack) {
        WrappingInt32 ackno = header.ackno;
        size_t window_size = header.win;
        _sender.ack_received(ackno, window_size);
    }

    
    if (header.syn) {
        // server端接收到第一次握手
        // 确定是从LISTEN转移过来
        // 运行到这里说明至少 receiver 是 syn_recv
        // 所以根据 sender 的状态决定发送内容
        bool check_sender_closed = _sender.next_seqno_absolute() == 0;
        if (check_sender_closed) {
            // 服务端回复
            _sender.fill_window();
        } else {
            // SYN_SENT -> received SYN without ack
            _sender.send_empty_segment();
        }

    }
    


    // 接下来还要处理关闭的情况

    // TCP keep-alive
    if (_receiver.ackno().has_value() && seg.length_in_sequence_space() == 0 && seg.header().seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }

    send_segment(_sender.segments_out());

}

bool TCPConnection::active() const {
    return (!_sender.stream_in().input_ended() && !_receiver.stream_out().input_ended());
}

size_t TCPConnection::write(const string &data) {
    // 需要确定进入 ESTABLISHED 状态
    // receiver SYN_RECV
    // sender SYN_ACKED
    bool check_estalished = _receiver.ackno().has_value() && !_receiver.stream_out().input_ended()
        && _sender.next_seqno_absolute() > _sender.bytes_in_flight() && !_sender.stream_in().eof();
    if (!check_estalished) {
        return 0;
    }

    size_t written = _sender.stream_in().write(data);
    // _sender.fill_window();
    // send_segment(_sender.segments_out());
    return written;
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
    _sender.fill_window();
    
    send_segment(_sender.segments_out());
}

void TCPConnection::connect() {
    // state != closed
    if (_sender.next_seqno_absolute() != 0) {
        return;
    }

    // SYN_SENT
    _sender.fill_window();
    // 没有创建syn包
    if (_sender.segments_out().empty()) {
        return;
    }
    // TCPSender wants sent
    // cout << "func: connect" << endl;
    send_segment(_sender.segments_out());
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
