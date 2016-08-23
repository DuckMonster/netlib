#pragma once
#include <WinSock2.h>
#include <thread>
#include <queue>
#include <mutex>

#include "packet.hpp"

namespace net {
    class socketworker {
    public:
        socketworker( SOCKET socket );

        void                    send( const packet& pkt );
        bool                    connected( );

    private:
        void                    sendLoop( );
        void                    sendAll( std::queue<packet>& queue );
        void                    recvLoop( );

        std::queue<packet>&     frontSendQueue( );
        std::queue<packet>&     backSendQueue( );

        SOCKET                  socket;

        std::thread             sendThread;
        std::thread             recvThread;

        std::queue<packet>      sendQueues[2];
        size_t                  sendQueueIndex = 0;

        std::mutex              sendMtx;
    };
}