
#include "proto.hh"


future<Request*> Proto::Parse(input_stream<char> &in){

    Request *req = new Request;

    return make_ready_future<Request*>(std::move(req));
}



