
#ifndef __PROTO_HH__
#define __PROTO_HH__

#include "core/future-util.hh"
#include "core/temporary_buffer.hh"
#include "core/iostream.hh"

#include <memory>
#include <unistd.h>



struct ReqRspHeader {
    uint32_t size;
    uint32_t seq;
    uint16_t id;
    uint16_t flags;
}; //__attributed__((pack));

struct Request {
    ReqRspHeader ReqHeader;

    int ReqType;

 //   string ReqKey;
 //   vector<pair<string, string> > mReqs;  // key val, key val / field val, ...
};

class Proto {

public:
    Proto(){}

    ~Proto() {}

    future<Request*> Parse(input_stream<char> &in);
};


#endif // __PROTO_HH__

