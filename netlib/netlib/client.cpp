#include "client.hpp"
#include <asio.hpp>
#include <iostream>

net::client::client( ) :
    _service( ),
    _worker( nullptr ),
    _eventMngr( ) {
}

void net::client::connect( asio::ip::tcp::endpoint target ) {
    connect( { target.address( ).to_string( ), std::to_string( target.port( ) ) } );
}

void net::client::connect( const asio::ip::tcp::resolver::query& query ) {
    asio::ip::tcp::resolver resolver( _service );
    asio::error_code ec;

    // Create socket and connect!
    socket_ptr sptr( new asio::ip::tcp::socket( _service ) );
    asio::connect( *sptr, resolver.resolve( query ), ec );

    // Error in connecting
    if (ec)
        std::cerr << "Failed to connect to " << query.host_name( ) << ":" << query.service_name( ) << "\n";

    // ... otherwise, allocate socketworker
    else
        _worker = worker_ptr( new socketworker( sptr, 0, _eventMngr ) );
}

bool net::client::connected( ) {
    return _worker && _worker->connected( );
}

void net::client::send( const net::packet & pkt ) {
    if (!connected( )) {
        std::cerr << "Can't send when not connected to a host\n";
        return;
    }
    _worker->send( pkt );
}

bool net::client::pollEvent( net::event & ref ) {
    return _eventMngr.poll( ref );
}
