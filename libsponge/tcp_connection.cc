#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return time_since_last_segment_received_; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if(!active_) {
        return ;
    }
    time_since_last_segment_received_ = 0;
    if(seg.header().rst) {
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
        active_ = false;
    }
    else if(_sender.next_seqno_absolute() == 0) {
        if(seg.header().syn) {
            _receiver.segment_received(seg);
            connect();
        }
    }
    else if(_sender.bytes_in_flight() == _sender.next_seqno_absolute() && !_receiver.ackno().has_value()) {
        if(seg.header().syn && seg.header().ack) {
            _sender.ack_received(seg.header().ackno,seg.header().win);
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_data();
        }
        else if(seg.header().syn && !seg.header().ack) {
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_data();
        }
    }
}

bool TCPConnection::active() const {return active_;}

size_t TCPConnection::write(const string &data) {
    DUMMY_CODE(data);
    return {};
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if(!active_) {
       return;
    }
    time_since_last_segment_received_ += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
       reset_connection();
       return ;
    }
    send_data();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_data();
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_data();
}

void TCPConnection::send_data() {
    // 将sender中的数据保存到connection中
    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        // 尽量设置ackno和window_size
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = _receiver.window_size();
        }
        _segments_out.push(seg);
    }
    // 如果发送完毕则结束连接
    if (_receiver.stream_out().input_ended()) {
        if (!_sender.stream_in().eof()) {
            _linger_after_streams_finish = false;
        }
        else if (_sender.bytes_in_flight() == 0) {
            if (!_linger_after_streams_finish || time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
                active_ = false;
            }
        }
    }
}

void TCPConnection::reset_connection() {
    // 发送RST标志
    TCPSegment seg;
    seg.header().rst = true;
    _segments_out.push(seg);

    // 在出站入站流中标记错误，使active返回false
    _receiver.stream_out().set_error();
    _sender.stream_in().set_error();
    active_ = false;
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
