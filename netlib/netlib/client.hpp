#pragma once
#include "sdef.h"

#include <asio/ip/tcp.hpp>

#include "socketworker.hpp"
#include "eventmanager.hpp"

namespace net {
    class client {
    public:
        client( );

        void                connect( asio::ip::tcp::endpoint target );
        void                connect( const asio::ip::tcp::resolver::query& query );
        bool                connected( );

        void                send( const net::packet& pkt );
        bool                pollEvent( net::event& ref );

    private:
        asio::io_service    _service;
        worker_ptr          _worker;

        eventmanager        _eventMngr;
    };
}