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

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }


void TCPConnection::test_end() {
    // clean close
    // 如果出向字节流还没有到EOF的时候，入向stream就关闭了字节流
    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }

    cout << "rule #1-#3: " << (_receiver.stream_out().input_ended() && _sender.stream_in().eof()
        && _sender.bytes_in_flight() == 0) << endl;
    // 一旦前提条件1～3被满足，如果_linger_after_streams_finish为假
    if (_receiver.stream_out().input_ended() && _sender.stream_in().eof()
        && _sender.bytes_in_flight() == 0) {
        cout << "_linger_after_streams_finish: " << _linger_after_streams_finish << endl;
        cout << "time_since_last_segment_received: "<< time_since_last_segment_received() <<endl;
        if (!_linger_after_streams_finish || time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
            cout << "Check TIME WAIT TIMEOUT STATE" << endl;
            _active = false;
        }
    }
}

// 这个函数只负责 ack，ackno，win 和 payload部分
void TCPConnection::send_segment(queue<TCPSegment> &segment_prepare) {
    // TODO:只有当close状态才不能发送包
    cout << "segment to send: " << segment_prepare.size() << endl;
    
    // // 回应一个空ack
    // if (segment_prepare.size() == 0) {
    //     _sender.send_empty_segment();
    // }

    while (!segment_prepare.empty()) {
        TCPSegment seg = segment_prepare.front();
        segment_prepare.pop();
        optional<WrappingInt32> ackno = _receiver.ackno();
        if (ackno.has_value()) {
            seg.header().ack = true;
            seg.header().ackno = ackno.value();
            seg.header().win = max(static_cast<uint64_t>((0xffff)), _receiver.window_size());
        }
        cout << seg.header().summary();
        cout << " with " << seg.payload().size() << " bytes\n";


        _segments_out.push(seg);
    }
    test_end();
}

void TCPConnection::reset() {
    inbound_stream().set_error();
    _sender.stream_in().set_error();
    // TODO：active 立即为false
    _active = false;
}

void TCPConnection::debug_what_state() {
    string sender_state, receiver_state;

    if (!_receiver.ackno().has_value()) {
        receiver_state = "LISTEN";
    } else if (_receiver.ackno().has_value() && !_receiver.stream_out().input_ended()) {
        receiver_state = "SYN_RECV";
    } else if (_receiver.stream_out().input_ended()) {
        receiver_state = "FIN_RECV";
    }

    if (_sender.next_seqno_absolute() == 0) {
        sender_state = "CLOSED";
    } else if (_sender.next_seqno_absolute() > 0 && _sender.next_seqno_absolute() == _sender.bytes_in_flight()) {
        sender_state = "SYN_SENT";
    } else if ((_sender.next_seqno_absolute() > _sender.bytes_in_flight() && !_sender.stream_in().eof())
        || (_sender.stream_in().eof() && _sender.next_seqno_absolute() < _sender.stream_in().bytes_written() + 2)) {
        sender_state = "SYN_ACKED";
    } else if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2
        && _sender.bytes_in_flight() > 0) {
        sender_state = "FIN_SENT";
    } else if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2
        && _sender.bytes_in_flight() == 0) {
        sender_state = "FIN_ACKED";
    }

    cout << "sender_state: " << sender_state << "\treceiver_state: " << receiver_state << endl;
}

void TCPConnection::segment_received(const TCPSegment &seg) { 
    TCPHeader header = seg.header();
    // cout << "func: segment_received" << endl;
    cout << "---------before received segment-------------" << endl;
    // debug_what_state();

    cout << "sender bytes in flight: " << _sender.bytes_in_flight() << endl;

    if (header.rst) {
        reset();
        return;
    }

    _time_since_last_segment_received = 0;
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

        // bool check_fin_sent = _sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2
        //     && _sender.bytes_in_flight() != 0;
        // bool check_syn_recv = _receiver.ackno().has_value() && !_receiver.stream_out().input_ended();

        // cout << "check_fin_sent: " << check_fin_sent << " check_syn_recv: " << check_syn_recv << endl;
        // cout << "---------after received segment------------" << endl;
        // debug_what_state();


        // 结束tcp的过程
        // 处于FIN_WAIT_1, 收到ACK

        
        
        // if (check_fin_acked && check_fin_recv) {
        //     // 转到closed
        //     _active = false;
        //     return;
        // }

        
    }

    // 收到fin
    cout << "--------------------------------" << endl;
    cout << "---------after received segment------------" << endl;
    debug_what_state();
    cout << "sender bytes in flight: " << _sender.bytes_in_flight() << endl;
    if (header.fin) {
        // 不能产生有效包时至少要发一个ack包
        if (_sender.segments_out().empty()) {
            _sender.send_empty_segment();
        }
    }


    // // clean shutdown
    // // 收到 FIN 进入 CLOSE_WAIT 状态
    // if (header.fin) {
    //     end_input_stream();
    //     _get_fin = true;
    // }
    
    if (header.syn) {
        // server端接收到第一次握手
        // 确定是从LISTEN转移过来
        // 运行到这里说明至少 receiver 是 syn_recv
        // 所以根据 sender 的状态决定发送内容
        bool check_sender_closed = _sender.next_seqno_absolute() == 0;
        if (check_sender_closed) {
            // 服务端回复
            _sender.fill_window();
            _active = true;
        } else {
            // SYN_SENT -> received SYN without ack
            _sender.send_empty_segment();
        }

    }


    // 接下来还要处理关闭的情况

    // TCP keep-alive
    if (_receiver.ackno().has_value() && seg.length_in_sequence_space() == 0 && seg.header().seqno == _receiver.ackno().value() - 1) {
        cout << "keep alive TCP segment" << endl;
        _sender.send_empty_segment();
    }

    send_segment(_sender.segments_out());

}

bool TCPConnection::active() const {
    return _active;
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
    _sender.fill_window();
    send_segment(_sender.segments_out());
    return written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    _sender.tick(ms_since_last_tick);
    _time_since_last_segment_received += ms_since_last_tick;

    if (_sender.consecutive_retransmissions() >= TCPConfig::MAX_RETX_ATTEMPTS) {
        _sender.send_empty_segment();
        queue<TCPSegment> seg_out = _sender.segments_out();
        TCPSegment seg_to_sent = seg_out.front();
        seg_to_sent.header().rst = true;
    }
    // 需要重传
    if (!_sender.segments_out().empty()) {
        send_segment(_sender.segments_out());
    }
    test_end();
}

void TCPConnection::end_input_stream() {
    cout << __FUNCTION__ <<endl;
    _sender.stream_in().end_input();
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
    _active = true;
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
