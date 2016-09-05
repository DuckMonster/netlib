#pragma once
#include <asio.hpp>
#include <memory>
#include <thread>
#include <mutex>

#include "packet.hpp"
#include "bufferedqueue.hpp"

namespace net {
    typedef std::shared_ptr<asio::ip::tcp::socket> socket_ptr;

    class socketworker {
    public:
        socketworker( const socket_ptr& socket, const size_t socketID );
        ~socketworker( );

        // Returns if this socketworker is connected and active
        bool                    connected( ) const;
        void                    disconnect( );

        asio::ip::tcp::endpoint endpoint( );

        void                    send( const packet& pkt );
        bool                    recv( packet& pkt );

    private:
        void                    recvLoop( );
        void                    sendLoop( );

        //----------------

        socket_ptr              _socket;
        asio::ip::tcp::endpoint _endpoint;
        size_t                  _id;

        std::thread             _recvThread;
        std::thread             _sendThread;
        
        std::queue<packet>      _inQueue;
        bufferedqueue<packet>   _outQueue;

        std::mutex              _inMutex;
        std::mutex              _outMutex;

        std::condition_variable _outCV;
    };
}