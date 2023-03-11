#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

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
    , _stream(capacity)
    ,_retransmisson_timeout(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
   if(_is_fin) {
       return;
   }
   TCPSegment seg;
   if(!_is_syn) {
       _is_syn = true;
       seg.header().syn = true;
       send_segment(seg);
       return ;
   }
   // 窗口大小，未知则假设为1
   uint16_t window_size = (_win_size > 0 ? _win_size : 1);
   //_next-_ack=已发送但尚未确认的data，如果该值小于window，那么就代表这段data已经传输完毕
   if(stream_in().eof() && _ack_seqno + window_size > _next_seqno) {
       seg.header().fin = true;
       _is_fin = true;
       send_segment(seg);
       return ;
   }

    while(!stream_in().buffer_empty()) {
       //window已满，停止发送
       if(_next_seqno > _ack_seqno + window_size-1) {
           break;
       }
        size_t len = min(TCPConfig::MAX_PAYLOAD_SIZE,min(stream_in().buffer_size(),window_size-bytes_in_flight()));
        seg.payload() = stream_in().read(len);
        if(seg.length_in_sequence_space()< window_size && stream_in().eof()) {
            seg.header().fin = true;
            _is_fin = true;
        }
        if(seg.length_in_sequence_space() == 0) {
            return ;
        }
        send_segment(seg);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
   uint64_t abs_ack = unwrap(ackno, _isn, _next_seqno);
   if(abs_ack > _next_seqno) {
       return;
   }
   if(abs_ack >= _ack_seqno) {
       _win_size = window_size;
       _ack_seqno = abs_ack;
   }
   bool pop = false;
   while(!segments_waiting().empty()) {
       TCPSegment seg = segments_waiting().front();
       u_int64_t seq = unwrap(seg.header().seqno,_isn,_next_seqno);
       if(seq +seg.length_in_sequence_space() > abs_ack) {
            return ;
       }
       segments_waiting().pop();
       _bytes_in_flight -= seg.length_in_sequence_space();
       _retransmisson_timeout = _initial_retransmission_timeout;
       _consecutive_retransmission = 0;
       _total_time = 0;
       pop = true;

   }
   //如果有新的空间打开，填充
    if(pop) {
       fill_window();
    }
   if(!segments_waiting().empty()) {
       _is_timer_running = true;
   }
   else {
       _is_timer_running = false;
   }
}
//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
   if(!_is_timer_running) {
       return;
   }
   _total_time += ms_since_last_tick;
   if(_total_time >= _retransmisson_timeout && !segments_waiting().empty()) {
       TCPSegment seg = segments_waiting().front();
       segments_out().push(seg);
       _total_time = 0;
       if(_win_size != 0 || seg.header().syn) {
           _retransmisson_timeout *= 2;
           _consecutive_retransmission++;
       }
   }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    segments_out().push(seg);
}
void TCPSender::send_segment(TCPSegment &tcp_segment) {
    // 设置序号
    tcp_segment.header().seqno = next_seqno();
    // 放入队列中
    segments_waiting().push(tcp_segment);
    segments_out().push(tcp_segment);
    // 本地保存发送的数据大小
    _next_seqno += tcp_segment.length_in_sequence_space();
    _bytes_in_flight += tcp_segment.length_in_sequence_space();

    // 启动重传计时器
    if (!_is_timer_running) {
       _is_timer_running = true;
       _total_time = 0;
    }
}
