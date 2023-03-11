#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn) {
        _syn_received = true;
        _isn = seg.header().seqno;
        _reassembler.push_substring(seg.payload().copy(),0,seg.header().fin);
    }
    if (_syn_received) {
        u_int64_t abs_seqno = unwrap(seg.header().seqno,_isn,stream_out().bytes_written())-1;
        _reassembler.push_substring(seg.payload().copy(),abs_seqno,seg.header().fin);
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if(!_syn_received) {
        return nullopt;
    }
    else {
        if(stream_out().input_ended()) {
            return wrap(stream_out().bytes_written()+2,_isn);
        }
        else {
            return wrap(stream_out().bytes_written()+1,_isn);
        }
    }
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
