#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _output(capacity), 
    _capacity(capacity),
    first_unassembled(0),
    _eof_index(-1),
    _eof(false),
    _buff(),
    _unassembled_bytes(0) {}


size_t StreamReassembler::merge_nodes(const seg_node &a, seg_node &b) {
    size_t duplica = 0;
    if (a.begin < b.begin) {
        b.data = a.data.substr(0, b.begin - a.begin) + b.data;
        duplica += b.begin - a.begin;
        b.begin = a.begin;
    }
    if (a.end > b.end) {
        b.data += a.data.substr(b.end - a.begin, a.end);
        duplica += a.end - b.end;
        b.end = a.end;
    }
    return duplica;
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (eof) {
        _eof_index = index + data.length();
    }
    
    // 超过窗口范围
    if (first_unassembled + _capacity <= index) {
        return;
    }

    // 先裁剪数据块保证大小合适
    size_t begin = max(index, first_unassembled);
    size_t end = min(index + data.length(), _output.bytes_read() + _capacity);
    if (begin > end) {
        // 不存在需要更新的数据
        return;
    }
    string seg_data = data.substr(begin - index, end - begin);
    // cout << "begin: " << begin << " end: " << end << " data: " << seg_data << endl;
    seg_node node(begin, end, seg_data);
    // _unassembled_bytes += seg_data.length();

    if (!_buff.empty()) {
        set<seg_node>::iterator prev = _buff.upper_bound(node);
        if (prev != _buff.begin()) {
            prev--;
        }
        while (prev != _buff.end()) {
            // 判断与当前块是否重叠
            if ((*prev).end >= node.begin && (*prev).begin <= node.end) {
                merge_nodes((*prev), node);
                _unassembled_bytes -= (*prev).data.length();
                prev = _buff.erase(prev);
            } else if ((*prev).begin > node.end) {
                break;
            } else {
                prev++;
            }
        }
    }
    _buff.insert(node);
    _unassembled_bytes += node.data.length();

    // 写入到_output
    if (first_unassembled == node.begin) {
        _output.write(node.data);
        first_unassembled += node.data.length();
        _buff.erase(node);
        _unassembled_bytes -= node.data.length();
    }

    // cout << "buff_size: " << _buff.size() << endl;
    if (first_unassembled == _eof_index) {
        _eof = true;
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes;}

bool StreamReassembler::empty() const { return _unassembled_bytes == 0;}
