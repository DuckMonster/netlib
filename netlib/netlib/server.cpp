#include "server.hpp"

#include <WS2tcpip.h>
#include <string>

net::server::server( ) :
    acceptSocket( INVALID_SOCKET ),
    acceptQueue( ),
    acceptMutex( ),
    acceptThread( ) {

}

net::server::~server( ) {

}

void net::server::startup( short port ) {
    // Resolve hosting address

    addrinfo hints, *result;

    hints.ai_flags      = AF_INET;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;

    getaddrinfo( NULL, std::to_string( port ).c_str( ), &hints, &result );

    // Create and bind socket
    acceptSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
    bind( acceptSocket, result->ai_addr, result->ai_addrlen );

    // Then set as a listening socket
    listen( acceptSocket, 5 );
}

void net::server::shutdown( ) {
    ::shutdown( acceptSocket, SD_BOTH );
    ::closesocket( acceptSocket );

    acceptSocket = INVALID_SOCKET;
}

bool net::server::active( ) {
    return acceptSocket != INVALID_SOCKET;
}

void net::server::acceptLoop( ) {
    while (active( )) {
        sockaddr    sockAddr;
        int         addrLen;
        SOCKET newClient = ::accept( acceptSocket, &sockAddr, &addrLen );

        // Lock and push the new client onto accept queue
        {
            std::lock_guard<std::mutex> lock( acceptMutex );

            acceptQueue.push( newClient );
        }
    }
}