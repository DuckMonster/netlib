#include "sockethandler.hpp"

#include <iostream>

using namespace net;
using namespace std;

socketworker::socketworker( SOCKET socket, const size_t& id ) :
    socket( socket ),
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

    cout << "Socket " << connectionID << " destroyed.\n";
}

void net::socketworker::disconnect( ) {
    if (!connected( ))
        return;

    // Shutdown peacefully
    shutdown( socket, SD_BOTH );
    closesocket( socket );

    // Set socket as invalid
    socket = INVALID_SOCKET;
}

void socketworker::send( const packet & pkt ) {
    // Not connected
    if (!connected( ))
        return;

    // Lock (try)
    unique_lock<mutex> lock( sendMtx, try_to_lock );

    // Push onto front queue
    if (lock.owns_lock( ))
        sendQueue.front( ).push( pkt );
}

bool net::socketworker::recv( packet & pkt ) {
    // Not connected
    if (!connected( ))
        return false;

    // Lock
    lock_guard<mutex> lock( recvMtx );

    // If there is packets in the queue, copy, remove it and return true
    if (!recvQueue.empty( )) {
        pkt = recvQueue.front( );
        recvQueue.pop( );

        return true;
    }

    // No messages
    return false;
}

bool socketworker::connected( ) {
    return socket != INVALID_SOCKET;
}

void socketworker::sendLoop( ) {
    //TODO: Use guard_lock somehow?

    while (connected( )) {
        sendMtx.lock( ); // LOCK

        // If the front queue is not empty, it will swap the front and back queue
        if (!sendQueue.front( ).empty( )) {
            queue<packet> q = sendQueue.swap( );
            sendMtx.unlock( ); // UNLOCK

            sendAll( q );
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
        size_t received = ::recv( socket, buffer, 512, 0 );

        if (received == 0 || received == SOCKET_ERROR) {
            disconnect( );
            return;
        }

        // Add packet to queue
        packet pkt( buffer, received );
        {
            lock_guard<mutex> lock( recvMtx );
            recvQueue.push( pkt );
        }
    }
}
