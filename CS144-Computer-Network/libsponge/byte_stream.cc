#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) { 
    _capacity = capacity;
}

size_t ByteStream::write(const string &data) {
    size_t len = data.length();
    if (_input_ending) return 0;
    if (len > _capacity - _buffer.size()) {
        len = _capacity - _buffer.size();
    }
    _writeCnt += len;
    for (size_t i = 0; i < len; i++) {
        _buffer.push_back(data[i]);
    }
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t dataLen = len;
    if (dataLen > _buffer.size()) {
        dataLen = _buffer.size();
    }
    string res = string().assign(_buffer.begin(), _buffer.begin() + dataLen);

    return res;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t dataLen = len;
    if (dataLen > _buffer.size()) {
        dataLen = _buffer.size();
    }
    _readCnt += dataLen;
    while (dataLen--) {
        _buffer.pop_front();
    }
    return;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string res = peek_output(len);
    pop_output(len);
    return res;
}

void ByteStream::end_input() { _input_ending = true;}

bool ByteStream::input_ended() const { return _input_ending; }

size_t ByteStream::buffer_size() const { return _buffer.size();}

bool ByteStream::buffer_empty() const { return _buffer.empty(); }

bool ByteStream::eof() const { return _buffer.size() == 0 && _input_ending; }

size_t ByteStream::bytes_written() const { return _writeCnt; }

size_t ByteStream::bytes_read() const { return _readCnt; }

size_t ByteStream::remaining_capacity() const { return _capacity - _buffer.size(); }
