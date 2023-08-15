#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity), _window_size(1)
    , _flight_bytes(0), timer(_initial_retransmission_timeout)
    , _syn(true), _fin(false){}

uint64_t TCPSender::bytes_in_flight() const { return _flight_bytes; }

void TCPSender::fill_window() {
    size_t win_size = _window_size == 0 ? 1 : _window_size;

    while (_flight_bytes < win_size) {
        // cout << "win_size " << win_size << endl;
        TCPHeader header;
        header.seqno = wrap(_next_seqno, _isn);
        // cout << "seqno: " << header.seqno << endl;
        size_t maybe_sent = 0;
        if (_syn) {
            header.syn = _syn;
            _syn = false;
            maybe_sent += 1;
        }

        // 计算data
        // 还需要判断添加 fin 后会不会超过win
        size_t data_len = min(_stream.buffer_size(), win_size - _flight_bytes - maybe_sent);
        // cout << "data_len " << data_len << endl;
        

        if (data_len > TCPConfig::MAX_PAYLOAD_SIZE) {
            data_len = TCPConfig::MAX_PAYLOAD_SIZE;
        }

        maybe_sent += data_len;
        // cout << "win_size: " << win_size << " data_len: " << data_len << " maybe_sent: " << maybe_sent <<
        //     " flight_bytes: " << _flight_bytes  << " buffer_size: " << _stream.buffer_size() <<  endl;
        // FIN 包只能发送一次
        if (data_len == _stream.buffer_size() && _stream.input_ended() && !_fin) {
            // cout << "win_size: " << win_size << " data_len: " << data_len << " maybe_sent: " << maybe_sent << endl;
            if (maybe_sent + _flight_bytes + 1 <= win_size) {
                maybe_sent += 1;
                header.fin = true;
                _fin = true;
            }
        
        }

        cout << "_fin: " << (_fin ? 1 : 0) << "\n";
        TCPSegment seg;
        seg.header() = header;
        seg.payload() = Buffer(move(_stream.read(data_len)));
        size_t seg_len = seg.length_in_sequence_space();
        if (seg_len == 0) return; 

        _next_seqno += seg_len;
        _flight_bytes += seg_len;
        // cout << "seqno: " << seg.header().seqno << endl;
        _segments_out.push(seg);
        _segments_in_flight.push(seg);
        if (!timer.running()) timer.start();
        cout << "fill_window seg:" << endl;
        cout << seg.header().summary();
        cout << " with " << seg.payload().size() << " bytes\n";
    }
    
}


//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    cout << "ack raw: " << ackno.raw_value() << endl;
    uint64_t real_ackno = unwrap(ackno, _isn, _next_seqno);
    bool valid = false;
    // invalid ackno
    if (real_ackno > _next_seqno) {
        return;
    }
    cout << "real_ackno: " << real_ackno << endl;
    cout << "_next_seqno: " << _next_seqno << endl; 
    _window_size = window_size;

    // update segment in flights
    while (!_segments_in_flight.empty()) {
        auto seg = _segments_in_flight.front();

        uint64_t seg_end_sqno = unwrap(seg.header().seqno, _isn, _next_seqno) + seg.length_in_sequence_space();
        cout << "seg_end_seqno: " << seg_end_sqno << endl;
        if (real_ackno >= seg_end_sqno) {
            // cout << "seqno: " << seg.header().seqno << endl;
            _segments_in_flight.pop();
            _flight_bytes -= seg.length_in_sequence_space();
            // 只有当真的收到有效ack时，才重置timer
            valid = true;
        } else {
            break;
        }
    }
    if (valid) {
        timer.reset(_initial_retransmission_timeout);
    }

    // cout << "ack sqno:" << ackno.raw_value() << endl;
    fill_window();
    if (_segments_in_flight.empty()) {
        timer.close();
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    bool expire = timer.tick(ms_since_last_tick);
    // 超时
    if (expire) {
        if (_segments_in_flight.empty()) {
            timer.close();
            return;
        }
        _segments_out.push(_segments_in_flight.front());
        if (_window_size == 0) {
            timer.reset(_initial_retransmission_timeout);
            return;
        }
        timer.double_RTO();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return timer.num_of_retransmission; }

void TCPSender::send_empty_segment() {
    TCPHeader header;
    header.seqno = wrap(_next_seqno, _isn);
    TCPSegment seg;
    seg.header() = header;
    _segments_out.push(seg);
}
