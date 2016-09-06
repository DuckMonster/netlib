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

        void                        startup( const short port );
        void                        shutdown( );
        bool                        active( );

        void                        send( const net::packet& pkt, size_t id );

        bool                        pollEvent( net::event& ref );

    private:
        typedef std::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;

        void                        acceptLoop( const short port);

        void                        doAccept( const short port );
        void                        handleAccept( asio::error_code ec );

        void                        advanceCursor( );

        //-------------


        asio::io_service            _service;
        //acceptor_ptr                _acceptorPtr;
        asio::ip::tcp::acceptor     _acceptor;

        socket_ptr                  _acceptSocket;
        asio::ip::tcp::endpoint     _acceptEP;

        eventmanager                _eventMngr;

        bool                        _active;

        std::array<worker_ptr, 256> _workerArray;
        size_t                      _workerArrayCursor;

        std::thread                 _acceptThread;
    };
}