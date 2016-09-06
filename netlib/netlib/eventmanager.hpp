#pragma once
#include <queue>
#include <mutex>
#include "event.hpp"

namespace net {
    class eventmanager {
    public:
        void                        add( const net::event& e );
        bool                        poll( net::event& ref );

    private:
        std::queue<net::event>      _eventQueue;
        std::mutex                  _mtx;
    };
}