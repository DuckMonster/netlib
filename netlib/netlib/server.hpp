#pragma once
#include <WinSock2.h>
#include <queue>
#include <thread>
#include <mutex>
#include "event.hpp"

namespace net {
    class server {
    public:
        server( );
        ~server( );

        void                startup( short port );
        void                shutdown( );
        bool                active( );

        bool                pollEvent( event& ev );

    private:
        SOCKET              acceptSocket;

        //-----------------------

        std::queue<event>   eventQueue;

        std::queue<SOCKET>  acceptQueue;
        std::mutex          acceptMutex;
        std::thread         acceptThread;

        void                acceptLoop( );
    };
}