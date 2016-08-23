#include "sockethandler.hpp"

using namespace net;

socketworker::socketworker( SOCKET socket ) :
    socket( socket ),
    sendThread( ),
    recvThread( ) {

    // Startup worker threads
    sendThread = std::thread( &socketworker::sendLoop, this );
    recvThread = std::thread( &socketworker::recvLoop, this );
}

void socketworker::send( const packet & pkt ) {
    // Lock
    std::lock_guard<std::mutex> lock( sendMtx );

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
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
}

void socketworker::sendAll( std::queue<packet>& queue ) {
    // Send all packets in queue
    while (!queue.empty( )) {
        packet& pkt = queue.front( );
        ::send( socket, &pkt, pkt.size( ), 0 );

        queue.pop( );
    }
}

void socketworker::recvLoop( ) {
    while (connected( )) {

    }
}

std::queue<packet>& socketworker::frontSendQueue( ) {
    return sendQueues[sendQueueIndex % 2];
}

std::queue<packet>& socketworker::backSendQueue( ) {
    return sendQueues[(sendQueueIndex + 1) % 2];
}
