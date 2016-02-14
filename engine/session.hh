
#ifndef __SESSION_HH__
#define __SESSION_HH__

#include "proto.hh"

#include "core/future-util.hh"
#include "core/iostream.hh"
#include "net/api.hh"
#include "net/packet-data-source.hh"

#include <unistd.h>


class Session {
private:
    connected_socket _socket;
    socket_address _addr;
    input_stream<char> _in;
    output_stream<char> _out;

    Proto  _proto;

    uint32_t _seqnum;

    uint64_t _reqs;
    uint64_t _rsps;

public:
    Session(connected_socket&& socket, socket_address addr)
            : _socket(std::move(socket))
            , _addr(addr)
            , _in(_socket.input())
            , _out(_socket.output())
    {
        _seqnum = 0;
        _reqs = 0;
        _rsps = 0;
    }

    ~Session() {
    }

    bool Eof() {
        return _in.eof();
    }

    future<> Close(){
        _out.close();

        return make_ready_future<>();
    }

    future<> Handle() {
        _proto.Parse(_in).then([this](Request *req){
//            if(req.seq != _seqnum){
//            }
//            _reqs++;
                
            return make_ready_future<>();
        });

        return make_ready_future<>();
    }
};


#endif // __SESSION_HH__

