
#include "server.hh"

#include <boost/intrusive/unordered_set.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <iomanip>
#include <sstream>
#include "core/app-template.hh"
#include "core/future-util.hh"
#include "core/timer-set.hh"
#include "core/shared_ptr.hh"
#include "core/stream.hh"
#include "core/memory.hh"
#include "core/units.hh"
#include "core/distributed.hh"
#include "core/vector-data-sink.hh"
#include "core/bitops.hh"
#include "core/slab.hh"
#include "core/align.hh"
#include "net/api.hh"
#include "net/packet-data-source.hh"
#include <unistd.h>

#if 0
class meson {
private:
    server _sv;
    cache  _ca;
//  leveldb _ldb;
  
public:
    meson() { }
    ~meson(){ }

    void start(){
    }

    void stop(){
    }
};
#endif


int main(int ac, char** av) {
    distributed<CacheServer>  cache;

    app_template app;

    return app.run_deprecated(ac, av, [&] {
        engine().at_exit([&] { return cache.stop(); });

        return cache.start(1080).then([&cache] {
            return cache.invoke_on_all(&CacheServer::start);
        });
    });

}



