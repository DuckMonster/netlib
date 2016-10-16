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
		void					doRecv( const size_t index );
		void					doSend( const packet& packet );

        //----------------

        socket_ptr              _socket;
        asio::ip::tcp::endpoint _endpoint;
        size_t                  _id;

        eventmanager&           _eventMngr;

		std::mutex				_sendMtx;
		std::condition_variable	_sendCV;

		char					_recvBuffer[2048];
		char					_sendBuffer[2048];
		bool					_currentlySending;
    };

    typedef std::unique_ptr<socketworker> worker_ptr;
}