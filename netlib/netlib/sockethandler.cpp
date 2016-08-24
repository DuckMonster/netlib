#include "sockethandler.hpp"

#include <iostream>

using namespace net;
using namespace std;

socketworker::socketworker( SOCKET socket, queue<event>& eventQueue, const size_t& id ) :
    socket( socket ),
    eventQueue( eventQueue ),
    connectionID( id ),
    sendThread( ),
    recvThread( ) {

    // Startup worker threads
    sendThread = thread( &socketworker::sendLoop, this );
    recvThread = thread( &socketworker::recvLoop, this );
}

net::socketworker::~socketworker( ) {
    disconnect( );

    sendThread.join( );
    recvThread.join( );
}

void net::socketworker::disconnect( ) {
    if (!connected( ))
        return;

    // Add event
    eventQueue.push( event( event::disconnectData( connectionID ) ) );

    // Shutdown peacefully
    shutdown( socket, SD_BOTH );
    closesocket( socket );

    // Set socket as invalid
    socket = INVALID_SOCKET;
}

void socketworker::send( const packet & pkt ) {
    // Lock
    lock_guard<mutex> lock( sendMtx );

    // Push onto front queue
    frontSendQueue( ).push( pkt );
}

bool socketworker::connected( ) {
    return socket != INVALID_SOCKET;
}

void socketworker::sendLoop( ) {
    //TODO: Use guard_lock somehow?

    while (connected( )) {
        sendMtx.lock( ); // LOCK

        // If the front queue is not empty, it will swap the front and back queue
        // After that it will loop through the back queue to send all packets, while the new front queue will be filled
        if (!frontSendQueue( ).empty( )) {
            sendQueueIndex++;
            sendMtx.unlock( ); // UNLOCK

            sendAll( backSendQueue( ) );
        }
        else
            sendMtx.unlock( ); // UNLOCK

        //TODO: Don't sleep, implement conditional variables instead
        this_thread::sleep_for( chrono::milliseconds( 1 ) );
    }
}

void socketworker::sendAll( queue<packet>& queue ) {
    // Send all packets in queue
    while (!queue.empty( )) {
        packet& pkt = queue.front( );
        ::send( socket, &pkt, pkt.size( ), 0 );

        queue.pop( );
    }
}

void socketworker::recvLoop( ) {
    while (connected( )) {
        char buffer[512];
        std::cout << "starting recv: " << socket << "\n";

        size_t received = ::recv( socket, buffer, 512, 0 );

        std::cout << "recv\n";

        if (received == 0 || received == SOCKET_ERROR) {
            if (received == SOCKET_ERROR)
                eventQueue.push( event( event::errorData( "recv", WSAGetLastError( ) ) ) );

            disconnect( );
            return;
        }
        {
            packet pkt( buffer, received );
            eventQueue.push( event( event::packetData( pkt, connectionID ) ) );
        }
    }
}

queue<packet>& socketworker::frontSendQueue( ) {
    return sendQueues[sendQueueIndex % 2];
}

queue<packet>& socketworker::backSendQueue( ) {
    return sendQueues[(sendQueueIndex + 1) % 2];
}
