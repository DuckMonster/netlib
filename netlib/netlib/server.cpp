#include "server.hpp"

#include <WS2tcpip.h>
#include <string>
#include <iostream>

void startupErr( const char* func, const int& code ) {
    std::cerr << "Error in " << func << ": " << code << "\n";
}

net::server::server( ) :
    acceptSocket( INVALID_SOCKET ),
    clientArray( ),
    acceptMutex( ),
    acceptThread( ) {

}

net::server::~server( ) {
    acceptThread.join( );
}

void net::server::startup( short port ) {
    int iResult;

    WSADATA data;
    iResult = WSAStartup( MAKEWORD( 2, 2 ), &data );
    if (iResult != 0) {
        startupErr( "WSAStartup", iResult );
        return;
    }

    // Resolve hosting address
    addrinfo hints, *result;

    ZeroMemory( &hints, sizeof( hints ) );

    hints.ai_family     = AF_INET;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;

    iResult = getaddrinfo( NULL, std::to_string( port ).c_str( ), &hints, &result );
    if (iResult != 0) {
        startupErr( "getaddrinfo", iResult );
        return;
    }

    // Create and bind socket
    acceptSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
    if (acceptSocket == INVALID_SOCKET) {
        startupErr( "socket", WSAGetLastError( ) );
        freeaddrinfo( result );
        return;
    }

    iResult = bind( acceptSocket, result->ai_addr, result->ai_addrlen );
    if (iResult == SOCKET_ERROR) {
        startupErr( "bind", WSAGetLastError() );
        freeaddrinfo( result );
        return;
    }

    freeaddrinfo( result );

    // Then set as a listening socket
    iResult = listen( acceptSocket, 5 );
    if (iResult == SOCKET_ERROR) {
        startupErr( "listen", WSAGetLastError() );
        return;
    }

    // Start up accepting threads
    acceptThread = std::thread( &server::acceptLoop, this );
}

void net::server::shutdown( ) {
    ::shutdown( acceptSocket, SD_BOTH );
    ::closesocket( acceptSocket );

    acceptSocket = INVALID_SOCKET;
}

bool net::server::active( ) {
    return acceptSocket != INVALID_SOCKET;
}

bool net::server::pollEvent( net::event& e ) {
    std::lock_guard<std::mutex> lock( acceptMutex );

    if (eventQueue.empty( ))
        return false;

    e = eventQueue.front( );
    eventQueue.pop( );

    return true;
}

void net::server::acceptLoop( ) {
    while (active( )) {
        SOCKET      newClient = ::accept( acceptSocket, NULL, NULL );

        if (newClient == INVALID_SOCKET) {
            shutdown( );
            return;
        }

        // Lock and push the new client onto accept queue
        {
            std::lock_guard<std::mutex> lock( acceptMutex );

            // Add to list
            size_t newID = clientArray.insert( newClient );

            // Add a connect event to queue
            eventQueue.push( net::event( net::event::connectData( newID ) ) );
        }
    }
}