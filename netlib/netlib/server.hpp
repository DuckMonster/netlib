#pragma once
#include "sdef.h"

#include <asio/io_service.hpp>
#include <array>
#include "socketworker.hpp"
#include "eventmanager.hpp"

namespace net {
    class server {
    public:
        server( );
        ~server( );

        // Start accepting clients and receive data
        void							startup( const short port );
        // Shut down server, releasing all clients
        void							shutdown( );
        // Returns if this server is currently active
        bool							active( );

        // Send a packet to client of ID
        void							send( const net::packet& pkt, size_t id );

        // Poll for an event in the queue, returning if an event was found
        bool							pollEvent( net::event& ref );

    private:
        typedef std::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;

        void							acceptInit( const short port);

        void							doAccept( const short port );
        void							handleAccept( asio::error_code ec, const short port );

        void							advanceCursor( );

        //-------------


        asio::io_service				_service;
        asio::ip::tcp::acceptor			_acceptor;

        socket_ptr						_acceptSocket;
        asio::ip::tcp::endpoint			_acceptEP;

        std::mutex						_acceptMutex;
        std::condition_variable			_acceptCV;

        eventmanager					_eventMngr;

		worker_ptr						_workerArray[2048];
        //std::array<worker_ptr, 2048>	_workerArray;
        size_t							_workerArrayCursor;

        std::thread						_acceptThread;
    };
}