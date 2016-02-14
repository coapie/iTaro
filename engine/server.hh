
#ifndef __SERVER_HH__
#define __SERVER_HH__

#include "session.hh"
#include "core/future-util.hh"
#include "net/api.hh"

#include <unistd.h>


class CacheServer {

private:
    lw_shared_ptr<server_socket> _listener;
    uint16_t _port;

public:
    CacheServer(uint16_t port = 1080):_port(port)
    {}

    void start() {
        listen_options lo;
        lo.reuse_address = true;
        _listener = engine().listen(make_ipv4_address({_port}), lo);

        keep_doing([this] {
            return _listener->accept().then([this] (connected_socket fd, socket_address addr) mutable {
                auto ses = make_lw_shared<Session>(std::move(fd), addr);

                do_until([ses] { return ses->Eof(); }, [this, ses] {

                    return ses->Handle().then([ses] {
                        // FIXME : dispatch request to other engines
                        return ;
                    });
                }).finally([ses] {
                    return ses->Close().finally([ses]{});
                });
            });
        }).or_terminate();
    }

    future<> stop() { return make_ready_future<>(); }
};


#endif // __SERVER_HH__

