#include "client.hpp"
#include <iostream>
#include <WS2tcpip.h>

using namespace std;
using namespace net;

void connectErr( const char* func, const int& code ) {
    cerr << "Error in " << func << ": " << code << "\n";
}

net::client::client( ) {
}

net::client::~client( ) {
    disconnect( );
}

void net::client::connect( const char * ip, const short & port ) {
    int iResult;

    WSADATA data;
    iResult = WSAStartup( MAKEWORD( 2, 2 ), &data );
    if (iResult != 0) {
        connectErr( "WSAStartup", iResult );
        return;
    }

    // Resolve hosting address
    addrinfo hints, *result;

    ZeroMemory( &hints, sizeof( hints ) );

    hints.ai_family     = AF_INET;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;

    iResult = getaddrinfo( ip, to_string( port ).c_str( ), &hints, &result );
    if (iResult != 0) {
        connectErr( "getaddrinfo", iResult );
        return;
    }

    // Try to connect
    SOCKET connSocket = INVALID_SOCKET;
    addrinfo* ptr = result;

    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        // Create socket
        connSocket = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol );

        if (connSocket == INVALID_SOCKET) {
            connectErr( "socket", WSAGetLastError( ) );
            freeaddrinfo( result );
        }

        iResult = ::connect( connSocket, ptr->ai_addr, ptr->ai_addrlen );

        // Failed to connect
        if (iResult == SOCKET_ERROR) {
            closesocket( connSocket );
            connSocket = INVALID_SOCKET;
            continue;
        }

        // If you got this far you're done!
        break;
    }

    if (connSocket == INVALID_SOCKET) {
        connectErr( "Couldn't connect to host", 0 );
        return;
    }

    //TODO: Add ID receiving

    socketWorker = worker_ptr( new socketworker( connSocket, 0 ) );
}

void net::client::disconnect( ) {
    if (socketWorker) {
        socketWorker->disconnect( );
        socketWorker = nullptr;
    }
}

bool net::client::connected( ) {
    return socketWorker && socketWorker->connected( );
}

client & net::client::send( const packet & pkt ) {
    socketWorker->send( pkt );

    return *this;
}

void net::client::update( ) {
    if (!connected( ))
        return;

    // Disconnect (disabled for now)
    /*
    if (socketWorker && !socketWorker->connected( )) {
        eventQueue.push( event( event::disconnectData( 0 ) ) );
        socketWorker = nullptr;
        return;
    }*/

    // Messages
    packet pkt;
    while (socketWorker->recv( pkt ))
        eventQueue.push( event( event::packetData( pkt, 0 ) ) );
}

bool net::client::pollEvent( net::event & e ) {
    if (eventQueue.empty( ))
        return false;

    e = eventQueue.front( );
    eventQueue.pop( );

    return true;
}
