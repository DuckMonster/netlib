#pragma once
#include <WinSock2.h>
#include <queue>
#include <thread>
#include <mutex>
#include <idarray.hpp>

#include "event.hpp"

namespace net {
    class server {
    public:
        server( );
        ~server( );

        void                startup( short port );
        void                shutdown( );
        bool                active( );

        bool                pollEvent( net::event& e );

    private:
        SOCKET              acceptSocket;

        //-----------------------

        std::queue<event>   eventQueue;

        idarray<SOCKET>     clientArray;
        std::mutex          acceptMutex;
        std::thread         acceptThread;

        void                acceptLoop( );
    };
}