#pragma once
#include <WinSock2.h>
#include <queue>
#include <thread>
#include <mutex>
#include <idarray.hpp>
#include <memory>

#include "event.hpp"
#include "sockethandler.hpp"


namespace net {
    typedef std::shared_ptr<socketworker> worker_ptr;

    class server {
    public:
        server( );
        ~server( );

        void                    startup( short port );
        void                    shutdown( );
        bool                    active( );

        server&                 send( const size_t& id, const packet& pkt );

        bool                    pollEvent( net::event& e );

    private:
        SOCKET                  acceptSocket;

        //-----------------------

        std::queue<event>       eventQueue;

        idarray<worker_ptr>     clientArray;
        std::mutex              acceptMutex;
        std::thread             acceptThread;

        void                    acceptLoop( );
    };
}