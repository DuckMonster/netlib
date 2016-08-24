#pragma once
#include <WinSock2.h>
#include <thread>
#include <queue>
#include <mutex>

#include "bufferedqueue.hpp"
#include "packet.hpp"
#include "event.hpp"

namespace net {
    typedef void(event_func)( const event& e );

    class socketworker {
    public:
        socketworker( SOCKET socket, const size_t& id );
        ~socketworker( );

        void                    disconnect( );

        void                    send( const packet& pkt );
        bool                    recv( packet& pkt );
        bool                    connected( );

    private:
        void                    sendLoop( );
        void                    sendAll( std::queue<packet>& queue );
        void                    recvLoop( );

        SOCKET                  socket;
        const size_t            connectionID;

        std::thread             sendThread;
        std::thread             recvThread;

        bufferedqueue<packet>   sendQueue;
        std::queue<packet>      recvQueue;

        std::mutex              sendMtx;
        std::mutex              recvMtx;
    };
}