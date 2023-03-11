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
    _nwrite = 0;
    _nread = 0;
    _buf.resize(capacity);
    _buf.clear();
}

size_t ByteStream::write(const string &data) {
    size_t len = data.length();
    if(len > remaining_capacity()) {
        len = remaining_capacity();
    }
    for(size_t i =0;i<len;i++) {
        _buf.push_back(data[i]);
    }
    _nwrite += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t length = len;
    if(len > buffer_size()) {
        length = buffer_size();
    }
    return string().assign(_buf.begin(),_buf.begin()+length);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t length = len;
    if(len > buffer_size()) {
        length = buffer_size();
    }
    while(length--) {
        _buf.pop_front();
    }
    _nread += len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string s = peek_output(len);
    pop_output(s.length());
    return s;
}

void ByteStream::end_input() {_isend= true;}

bool ByteStream::input_ended() const { return _isend; }

size_t ByteStream::buffer_size() const { return _buf.size(); }

bool ByteStream::buffer_empty() const { return _buf.empty(); }

bool ByteStream::eof() const { return _isend && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _nwrite; }

size_t ByteStream::bytes_read() const { return _nread; }

size_t ByteStream::remaining_capacity() const { return _capacity-_buf.size(); }
