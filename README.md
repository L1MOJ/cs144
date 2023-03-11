# Lab0

环境配置

WSL+Clion[教程视频](https://www.youtube.com/watch?v=xno-kohRkQU)视频中更新Cmake版本后与视频中出现的错误不一样，我选择

```shell
sudo ln -sf /home/limoj/cmake-3.25.2-linux-x86_64/bin/* /usr/bin/
```

直接执行cp后cmake --version会报错

```shell
CMake Error: The source directory "/home/limoj/cmake-3.25.2-linux-x86_64/bin/version" does not exist.
Specify --help for usage, or press the help button on the CMake GUI.
```

## ![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676132245882-d654aa97-43cd-46da-ae81-46a3fbd8f75a.png#averageHue=%234fc256&clientId=u85978f1f-ae07-4&from=paste&height=646&id=u8803201c&name=%E5%9B%BE%E7%89%87.png&originHeight=646&originWidth=1803&originalType=binary&ratio=1&rotation=0&showTitle=false&size=101694&status=done&style=none&taskId=ube43401b-b1f2-40be-921e-bd093ecda45&title=&width=1803)

## 使用socket写一个网络程序

```cpp
void get_URL(const string &host, const string &path) {
    // Your code here.
    auto sc = TCPSocket();
    auto addr = Address(host,"http");
    sc.connect(addr);
    sc.write("GET "+path+" HTTP/1.1\r
");
    sc.write("Host: "+host+"\r
");
    sc.write("Connection: close\r
\r
");
    sc.shutdown(SHUT_WR);
    while(!sc.eof()) {
        printf("%s",sc.read().c_str());
    }
    sc.close();
    // You will need to connect to the "http" service on
    // the computer whose name is in the "host" string,
    // then request the URL path given in the "path" string.

    // Then you'll need to print out everything the server sends back,
    // (not just one call to read() -- everything) until you reach
    // the "eof" (end of file).

    cerr << "Function called: get_URL(" << host << ", " << path << ").
";
    cerr << "Warning: get_URL() has not been implemented yet.
";
}

```

:::tips
Socket.shutdown()与Socket.close()<br />shutdown用于优雅的关闭连接，即发送一个连接关闭请求给对方，对方响应后再关闭。<br />close则直接关闭连接
:::

## In-memory reliable byte stream

```cpp
#include "socket.hh"
#include "util.hh"

#include <cstdlib>
#include <iostream>

using namespace std;

void get_URL(const string &host, const string &path) {
    // Your code here.
    auto sc = TCPSocket();
    auto addr = Address(host,"http");
    sc.connect(addr);
    sc.write("GET "+path+" HTTP/1.1\r
");
    sc.write("Host: "+host+"\r
");
    sc.write("Connection: close\r
\r
");
    sc.shutdown(SHUT_WR);
    while(!sc.eof()) {
        printf("%s",sc.read().c_str());
    }
    sc.close();
    // You will need to connect to the "http" service on
    // the computer whose name is in the "host" string,
    // then request the URL path given in the "path" string.

    // Then you'll need to print out everything the server sends back,
    // (not just one call to read() -- everything) until you reach
    // the "eof" (end of file).

    cerr << "Function called: get_URL(" << host << ", " << path << ").
";
    cerr << "Warning: get_URL() has not been implemented yet.
";
}

int main(int argc, char *argv[]) {
    try {
        if (argc <= 0) {
            abort();  // For sticklers: don't try to access argv[0] if argc <= 0.
        }

        // The program takes two command-line arguments: the hostname and "path" part of the URL.
        // Print the usage message unless there are these two arguments (plus the program name
        // itself, so arg count = 3 in total).
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " HOST PATH
";
            cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144
";
            return EXIT_FAILURE;
        }

        // Get the command-line arguments.
        const string host = argv[1];
        const string path = argv[2];

        // Call the student-written function.
        get_URL(host, path);
    } catch (const exception &e) {
        cerr << e.what() << "
";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

```

使用Deque实现数缓冲区据流


# Lab1 reassembler

![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676194692957-6f31c46a-4176-4a0b-b15b-e1dd7acdf0e0.png#averageHue=%23fbfafa&clientId=u20db2054-005e-4&from=paste&height=487&id=u15ff23fa&name=%E5%9B%BE%E7%89%87.png&originHeight=487&originWidth=806&originalType=binary&ratio=1&rotation=0&showTitle=false&size=129741&status=done&style=none&taskId=u5fdb268f-b1b9-463d-90d2-47d9cb43bd3&title=&width=806)<br />自己想了很久没想出来还是参考了别人的代码，看完以后发现是缓冲区的概念没有搞清楚，实际上整个操作都是在_output上进行的，只是从这个ByteStream中划分了后半部分当作缓冲区来看待。Deque储存的是unassembled后面的部分，在Deque flag中红色部分值为true，白色为false代表没有元素

```cpp
#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) :

    _buffer(capacity,'\0'),
    _flag(capacity,false),
    _output(capacity), _capacity(capacity) {}
    //! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //size_t _first_unread = _start_index + _output.bytes_read();
    size_t _first_unassembled = _output.bytes_written();
    size_t _first_unaccept = _output.bytes_read() + _capacity;
    //超出待读取范围的直接扔掉
    if(index>_first_unaccept || index+data.length()<_first_unassembled) {
        return;
    }
    //更新有用部分的起始位置
    size_t begin_index = index;
    size_t end_index = index + data.length();

    if(begin_index < _first_unassembled) {
        begin_index = _first_unassembled;
    }
    if(end_index > _first_unaccept) {
        end_index = _first_unaccept;
    }
	//所有数据都暂时写入缓冲区
    for(size_t i = begin_index;i<end_index;i++) {
        if(!_flag[i-_first_unassembled]) {
            _flag[i-_first_unassembled] = true;
            _buffer[i-_first_unassembled] = data[i-index];
            _unassembled_bytes++;
        }
    }
    //将顺序正确的子串写入，并补充deque使长度不变
    string wait_str="";
    while(_flag.front()) {
        wait_str += _buffer.front();
        _flag.pop_front();
        _buffer.pop_front();
        _flag.push_back(false);
        _buffer.push_back('\0');
    }
    if (wait_str.length() > 0) {
        _output.write(wait_str);
        _unassembled_bytes -= wait_str.length();
    }
    //如果当前传入了eof，但由于数据太长超出acceptable范围，那么还需等待read完后的下次输入
    if(eof) {
        _is_eof = true;
        _eof_index = index+data.length();
    }
    if (_is_eof && _eof_index == _output.bytes_written()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return unassembled_bytes() == 0; }

```

```cpp
class StreamReassembler {
  private:
    // Your code here -- add private members as necessary.
    std::deque < char > _buffer ;
    std::deque < bool > _flag ;
    bool _is_eof = false;
    size_t _eof_index = 0;
    size_t _unassembled_bytes = 0;
    ByteStream _output;  //!< The reassembled in-order byte stream
    size_t _capacity; 
```

# Lab2 TCP Receiver

![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676263697703-72b668a1-2dfc-48af-8d9e-d6c8f7fbfd43.png#averageHue=%23c3d5c9&clientId=udb0e82bc-47fe-4&from=paste&height=267&id=u2310744c&name=%E5%9B%BE%E7%89%87.png&originHeight=267&originWidth=664&originalType=binary&ratio=1&rotation=0&showTitle=false&size=84143&status=done&style=none&taskId=u0a0efce1-0bdd-4cca-9689-15f29f73adb&title=&width=664)TCP需要提供可靠的数据流服务和流量控制

## 实现seqno和absolute seqno的转换

![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676264470676-7b56b6fd-b80a-4c42-9944-d5c198dea4f0.png#averageHue=%23f8f5f0&clientId=udb0e82bc-47fe-4&from=paste&height=664&id=ue807b925&name=%E5%9B%BE%E7%89%87.png&originHeight=664&originWidth=681&originalType=binary&ratio=1&rotation=0&showTitle=false&size=225856&status=done&style=none&taskId=u1f7c3332-741b-4b52-9f44-c53be9472b5&title=&width=681)![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676264543802-c99f4125-6f77-4763-abf2-d241d9cef435.png#averageHue=%23f9f7f5&clientId=udb0e82bc-47fe-4&from=paste&height=271&id=u2408726a&name=%E5%9B%BE%E7%89%87.png&originHeight=271&originWidth=661&originalType=binary&ratio=1&rotation=0&showTitle=false&size=49213&status=done&style=none&taskId=u54cf68bb-3744-4134-bbcc-890232ed53c&title=&width=661)
:::tips
在TCP协议中，序列号用于确定发送和接收数据的顺序，以及检测数据包是否丢失或重复。而32位序列号可以提供足够的空间来处理大多数互联网应用程序中的数据流，同时减少TCP头部的大小，从而降低网络传输的开销。此外，使用32位序列号还可以减少TCP在处理序列号时的计算量，提高传输效率。
:::
In addition to ensuring the receipt of all bytes of data, TCP makes sure that the beginning and ending of the stream are received reliably.
:::tips
所以存在SYN（beginning of the stream）和FIN（end-of-stream）<br />SYN的seqno是ISN（Initial Sequence Number），数据中的第一个字节的seqno是ISN+1
:::

```java
#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    u_int32_t N = n & 0xFFFFFFFF;
    return WrappingInt32{isn+N};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! 
ote Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint32_t offset = n - wrap(checkpoint, isn);
    uint64_t ret = checkpoint + offset;
    // 取距离checkpoint最近的值，因此需要判断是否左移ret
    // 注意位置不够左移的情形！！！
    if (offset >= (1u << 31) && ret >= (1ul << 32)) {
        ret -= (1ul << 32);
    }
    return ret;
}

```

## TCP R

![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676268676181-6fc3e87c-9b13-4e4f-b7a9-8f2b3b6d9dc2.png#averageHue=%23fbfbfb&clientId=udb0e82bc-47fe-4&from=paste&height=320&id=u020e0b4c&name=%E5%9B%BE%E7%89%87.png&originHeight=320&originWidth=653&originalType=binary&ratio=1&rotation=0&showTitle=false&size=39219&status=done&style=none&taskId=u041abf0b-9a1a-42d3-88e6-08da6b954a4&title=&width=653)

```java
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

```

![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676275017752-10c31426-84ca-4930-9dc2-9142650f9697.png#averageHue=%23fbf6f3&clientId=udb0e82bc-47fe-4&from=paste&height=361&id=u5ca870ed&name=%E5%9B%BE%E7%89%87.png&originHeight=361&originWidth=684&originalType=binary&ratio=1&rotation=0&showTitle=false&size=96469&status=done&style=none&taskId=u1e6a643e-845f-4dfd-857a-0456c3a4c5c&title=&width=684)Reassembler接收到的第一个data数据应该是从index为0开始的，所以第一个data第一字节的absolute seqno为1，必须减1后才能push_substring<br />ackno希望接收到的下一个字节，实际上就是stream中已经written的字节数+1.<br />Reassembler中的数据一旦顺序连贯就会立即被写入stream中。我们并不考虑stream.read，所以整个windowsize就是stream中剩余的capacity即capacity-写入但未读取的字节数<br />![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676275538931-9d78b911-e442-4008-b177-1a92a2099eff.png#averageHue=%23fdfcfb&clientId=udb0e82bc-47fe-4&from=paste&height=176&id=uf0b20ee6&name=%E5%9B%BE%E7%89%87.png&originHeight=176&originWidth=581&originalType=binary&ratio=1&rotation=0&showTitle=false&size=38996&status=done&style=none&taskId=u09bccf5e-2bf7-4b4b-860f-dca01c7ec93&title=&width=581)

# Lab3 TCPSender

![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676354613588-d5ac504e-1324-4746-947b-0f203e61e178.png#averageHue=%23ccaa7a&clientId=u8f3c0861-4186-4&from=paste&height=793&id=u4f3aacbe&name=%E5%9B%BE%E7%89%87.png&originHeight=793&originWidth=744&originalType=binary&ratio=1&rotation=0&showTitle=false&size=115531&status=done&style=none&taskId=ua3b15daf-4dff-4aed-aa15-4b4566d712d&title=&width=744)![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676368270681-5a674712-5a98-46a3-bfee-9dfe4df9b144.png#averageHue=%23f9f9f9&clientId=u8f3c0861-4186-4&from=paste&height=540&id=ue4f5c024&name=%E5%9B%BE%E7%89%87.png&originHeight=540&originWidth=606&originalType=binary&ratio=1&rotation=0&showTitle=false&size=106866&status=done&style=none&taskId=u6d727ab4-3278-41e7-879d-34733602805&title=&width=606)

```java
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

```

# Lab4 

![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676437461331-9c1aedfb-4e01-452e-88c7-a6ce91b0e151.png#averageHue=%23eaded7&clientId=u0e6cf215-8825-4&from=paste&height=507&id=u8c176742&name=%E5%9B%BE%E7%89%87.png&originHeight=507&originWidth=689&originalType=binary&ratio=1&rotation=0&showTitle=false&size=133775&status=done&style=none&taskId=u044a5e12-14da-42ef-a612-098a98d2ce8&title=&width=689)<br />本周，你将实现一个能跑的TCP。之前的工作里，你已经实现了sender和receiver。你接下来的任务就是把sender和receiver整合起来，进而与互联网上的任何一台计算机进行连接和通信。<br />回顾：TCP为字节流提供了双向的、具有流量控制功能的可靠传输。在两方建立了TCP连接的这段时间里，它们都即是发送者（从网络字节流里发送数据）又是接收者（从网络字节流里接收数据）。<br />![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676437422945-7d1ffe6a-4db9-49fa-8c54-3a9a1effa2e7.png#averageHue=%23eae9e9&clientId=u0e6cf215-8825-4&from=paste&height=211&id=ufff34835&name=%E5%9B%BE%E7%89%87.png&originHeight=211&originWidth=805&originalType=binary&ratio=1&rotation=0&showTitle=false&size=68157&status=done&style=none&taskId=u1357875d-2d01-490a-beff-81be9899a2d&title=&width=805)我们也将通信的双方称作“终端”，或者“节点”。你的TCPConnection就是建立连接的一个节点。TCPConnection的作用是发送和接收segment，确保发送方和接收方能够从收到的segment中获取到它们所关心的字段。

## 接收segment

如图1所示，当segment_received函数被调用的时候，就意味着TCPConnection从互联网中接收到了TCPSegment。于是TCPConnection需要考察segment的信息并进行如下处理：<br />如果RST（reset）标志位为真，将发送端stream和接受端stream设置成error state并终止连接。<br />把segment传递给TCPReceiver，这样的话，TCPReceiver就能从segment取出它所关心的字段进行处理了：seqno，SYN，payload，FIN。<br />如果ACK标志位为真，通知TCPSender有segment被确认，TCPSender关心的字段有ackno和window_size。<br />如果收到的segment不为空，TCPConnection必须确保至少给这个segment回复一个ACK，以便远端的发送方更新ackno和window_size。<br />在TCPConnection里的segment_received()中，有一个特殊情况需要做额外的处理：给“keep-alive” segment回复消息。对面的终端可能会发送一个不合法的seqno，来探测你的TCP连接是否仍然alive（如果alive，那么你当前的window状态是什么样子的）。即使这些segment不包含任何有效的seqno，你的TCPConnection依然要给这些“keep-avlie”的segment回复消息。代码大概长这个样子：

```
if (_receiver.ackno().has_value() and (seg.length_in_sequence_space() == 0)   
and seg.header().seqno == _receiver.ackno().value() - 1) 
{      _sender.send_empty_segment();      
// 在发送segment之前，      
// TCPConnection会读取TCPReceiver中关于ackno和window_size相关的信息。      
// 如果当前TCPReceiver里有一个合法的ackno，      
// TCPConnection会更改TCPSegment里的ACK标志位，      
// 将其设置为真。 }
```

## 发送Segment

TCPConnection发送segment的规则如下：

1. 当TCPSender把segment放入它的待发送队列中的时候，TCPSender需要将该sengment发送出去。
2. 在发送segment之前，TCPConnection会读取TCPReceiver中关于ackno和window_size相关的信息。如果当前TCPReceiver里有一个合法的ackno，TCPConnection会更改TCPSegment里的ACK标志位，将其设置为真。

随着时间的流逝，操作系统会定期调用TCPConnection的tick方法。当tick方法被调用，TCPConnection需要做如下几件事情：

1. 告诉TCPSender时间正在流逝。
2. 如果同一个segment连续重传的次数超过TCPConfig::MAX_RETX_ATTEMPTS，终止连接，并且给对方发送一个reset segment（一个RST为真的空segment）。
3. 尽可能地干净利落地结束该连接（参考Section5）。

综上，TCPSegmnet的数据结构如下图所示，有些字段是sender写入的，有些字段是receiver写入的，我们用不同的颜色标记了出来。<br />![图片.png](https://cdn.nlark.com/yuque/0/2023/png/5357310/1676437597082-9d424900-3a96-478a-94e8-3859e9076aba.png#averageHue=%23fafafa&clientId=u0e6cf215-8825-4&from=paste&height=357&id=u1696d784&name=%E5%9B%BE%E7%89%87.png&originHeight=357&originWidth=803&originalType=binary&ratio=1&rotation=0&showTitle=false&size=98880&status=done&style=none&taskId=u7750fbbb-70d8-4175-b071-6cfae36ffdd&title=&width=803)

```cpp
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
    // 非启动时不接收
    if (!active_) {
        return;
    }

    // 重置连接时间
    time_since_last_segment_received_ = 0;

    // RST标志，直接关闭连接
    if (seg.header().rst) {
        // 在出站入站流中标记错误，使active返回false
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
        active_ = false;
    }
        // 当前是Closed/Listen状态
    else if (_sender.next_seqno_absolute() == 0) {
        // 收到SYN，说明TCP连接由对方启动，进入Syn-Revd状态
        if (seg.header().syn) {
            // 此时还没有ACK，所以sender不需要ack_received
            _receiver.segment_received(seg);
            // 我们主动发送一个SYN
            connect();
        }
    }
        // 当前是Syn-Sent状态
    else if (_sender.next_seqno_absolute() == _sender.bytes_in_flight() && !_receiver.ackno().has_value()) {
        if (seg.header().syn && seg.header().ack) {
            // 收到SYN和ACK，说明由对方主动开启连接，进入Established状态，通过一个空包来发送ACK
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_data();
        } else if (seg.header().syn && !seg.header().ack) {
            // 只收到了SYN，说明由双方同时开启连接，进入Syn-Rcvd状态，没有接收到对方的ACK，我们主动发一个
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_data();
        }
    }
        // 当前是Syn-Revd状态，并且输入没有结束
    else if (_sender.next_seqno_absolute() == _sender.bytes_in_flight() && _receiver.ackno().has_value() &&
        !_receiver.stream_out().input_ended()) {
        // 接收ACK，进入Established状态
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
    }
        // 当前是Established状态，连接已建立
    else if (_sender.next_seqno_absolute() > _sender.bytes_in_flight() && !_sender.stream_in().eof()) {
        // 发送数据，如果接到数据，则更新ACK
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        if (seg.length_in_sequence_space() > 0) {
            _sender.send_empty_segment();
        }
        _sender.fill_window();
        send_data();
    }
        // 当前是Fin-Wait-1状态
    else if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
        _sender.bytes_in_flight() > 0 && !_receiver.stream_out().input_ended()) {
        if (seg.header().fin) {
        // 收到Fin，则发送新ACK，进入Closing/Time-Wait
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        _sender.send_empty_segment();
        send_data();
        } else if (seg.header().ack) {
        // 收到ACK，进入Fin-Wait-2
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        send_data();
        }
        }
        // 当前是Fin-Wait-2状态
    else if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
        _sender.bytes_in_flight() == 0 && !_receiver.stream_out().input_ended()) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        _sender.send_empty_segment();
        send_data();
        }
        // 当前是Time-Wait状态
    else if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
        _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended()) {
        if (seg.header().fin) {
        // 收到FIN，保持Time-Wait状态
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        _sender.send_empty_segment();
        send_data();
        }
        }
        // 其他状态
    else {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        _sender.fill_window();
        send_data();
        }
        }

        bool TCPConnection::active() const { return active_; }

        size_t TCPConnection::write(const string &data) {
        if (data.empty()) {
        return 0;
        }

        // 在sender中写入数据并发送
        size_t size = _sender.stream_in().write(data);
        _sender.fill_window();
        send_data();
        return size;
        }

        //! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
        void TCPConnection::tick(const size_t ms_since_last_tick) {
        // 非启动时不接收
        if (!active_) {
        return;
        }

        // 保存时间，并通知sender
        time_since_last_segment_received_ += ms_since_last_tick;
        _sender.tick(ms_since_last_tick);

        // 超时需要重置连接
        if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        reset_connection();
        return;
        }
        send_data();
        }

        void TCPConnection::end_input_stream() {
        _sender.stream_in().end_input();
        _sender.fill_window();
        send_data();
        }

        void TCPConnection::connect() {
        // 主动启动，fill_window方法会发送Syn
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
        cerr << "Warning: Unclean shutdown of TCPConnection
";

        // Your code here: need to send a RST segment to the peer
        reset_connection();
        }
        } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
        }
        }
```
