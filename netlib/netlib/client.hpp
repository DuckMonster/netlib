#pragma once
#include <WinSock2.h>
#include <queue>
#include <thread>
#include <mutex>
#include <memory>

#include "event.hpp"
#include "sockethandler.hpp"

namespace net {
    typedef std::shared_ptr<socketworker> worker_ptr;

    class client {
    public:
        client( );
        ~client( );

        void                    connect( const char* ip, const short& port );
        void                    disconnect( );
        bool                    connected( );

        client&                 send( const packet& pkt );

        void                    update( );
        bool                    pollEvent( net::event& e );

    private:

        //-----------------------

        std::queue<event>       eventQueue;
        worker_ptr              socketWorker;
    };
}