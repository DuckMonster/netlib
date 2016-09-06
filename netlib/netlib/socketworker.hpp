#pragma once
#include <asio.hpp>
#include <memory>
#include <thread>
#include <mutex>

#include "eventmanager.hpp"
#include "packet.hpp"
#include "bufferedqueue.hpp"

namespace net {
    typedef std::shared_ptr<asio::ip::tcp::socket> socket_ptr;

    class socketworker {
    public:
        socketworker( const socket_ptr& socket, const size_t socketID, eventmanager& eventMngr );
        ~socketworker( );

        // Returns if this socketworker is connected and active
        bool                    connected( ) const;

        // Close the connection with this socket
        void                    disconnect( );

        // Remote endpoint of this socket
        asio::ip::tcp::endpoint endpoint( );

        // Add a packet to the sending queue
        void                    send( const packet& pkt );

    private:
        void                    recvLoop( );
        void                    sendLoop( );

        //----------------

        socket_ptr              _socket;
        asio::ip::tcp::endpoint _endpoint;
        size_t                  _id;

        eventmanager&           _eventMngr;

        std::thread             _recvThread;
        std::thread             _sendThread;
        
        bufferedqueue<packet>   _sendQueue;

        std::mutex              _recvMutex;
        std::mutex              _sendMutex;

        std::condition_variable _sendCV;
    };

    typedef std::shared_ptr<socketworker> worker_ptr;
}