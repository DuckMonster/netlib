#include "socketworker.hpp"
#include <iostream>

net::socketworker::socketworker( const socket_ptr& socket, const size_t socketID ) :
    socket( socket ),
    id( socketID ),

    recvThread(),
    sendThread(),

    outQueue(){

    // Start up the threads
    recvThread = std::thread( &socketworker::recvLoop, this );
    sendThread = std::thread( &socketworker::sendLoop, this );
}

bool net::socketworker::connected( ) const {
    // Check if the socket pointer is null
    return (bool)socket;
}

void net::socketworker::disconnect( ) {
    // Shutdown and close
    socket->shutdown( asio::socket_base::shutdown_both );
    socket->close( );

    socket = nullptr;
}

void net::socketworker::recvLoop( ) {
    while (connected( )) {

        asio::error_code ec;
        char cbuff[1024];
        
        size_t pckSize = socket->receive( asio::buffer( cbuff, 1024 ), 0, ec );

        if (ec) {
            disconnect( );
            return;
        }

        std::cout << "Received " << pckSize << " bytes!\n";
    }
}

void net::socketworker::sendLoop( ) {

}
