#include "sockethandler.hpp"

#include <iostream>

using namespace net;
using namespace std;

// Short to char array
void stoc( const unsigned short& value, char arr[2] ) {
    memcpy( arr, &value, 2 );
}

// Char array to short
unsigned short ctos( const char arr[2] ) {
    unsigned short value;
    memcpy( &value, arr, 2 );

    return value;
}

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
            queue<packet>& q = sendQueue.swap( );
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

        // Each message has a 2 byte unsigned short with the length of the packet
        char pktSizeBuf[2];
        stoc( pkt.size( ), pktSizeBuf );    // unsigned short to char[2]
        
        // Send header
        ::send( socket, pktSizeBuf, 2, 0 );
        // Send actual package
        ::send( socket, &pkt, pkt.size( ), 0 );

        queue.pop( );
    }
}

void socketworker::recvLoop( ) {
    while (connected( )) {

        // Wait for packet size header (2 bytes, unsigned short)
        char header[2];
        size_t received = ::recv( socket, header, 2, 0 );

        if (received != 2) {
            disconnect( );
            return;
        }
        
        unsigned short dataSize = ctos( header );

        // Allocate buffer for actual packet
        char* buffer = new char[dataSize];
        received = ::recv( socket, buffer, dataSize, MSG_WAITALL ); // MSG_WAITALL tells the socket to completely fill the buffer

        // Error checking
        if (received == 0 || received == SOCKET_ERROR) {
            delete[] buffer; // Deallocate buffer
            disconnect( );
            return;
        }

        // Add packet to queue
        packet pkt( buffer, received );
        delete[] buffer;    // Deallocate buffer

        {
            lock_guard<mutex> lock( recvMtx );
            recvQueue.push( pkt );
        }
    }
}
