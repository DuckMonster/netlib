#include "socketworker.hpp"
#include <iostream>

net::socketworker::socketworker( const socket_ptr& socket, const size_t socketID, eventmanager& eventMngr ) :
    _socket( socket ),
    _endpoint( socket->remote_endpoint( ) ),
    _id( socketID ),

    _eventMngr( eventMngr ),

    _recvThread( ),
    _sendThread( ),

    _sendQueue( ) {

    // Start up the threads
    _recvThread = std::thread( &socketworker::recvLoop, this );
    _sendThread = std::thread( &socketworker::sendLoop, this );
}

net::socketworker::~socketworker( ) {
    // Disconnect and join the threads
    disconnect( );

    // Aquire lock and then notify, to ensure proper thread shutdown
    {
        std::lock_guard<std::mutex> outLock( _sendMutex ), inLock( _recvMutex );
        _sendCV.notify_all( );
    }

    _recvThread.join( );
    _sendThread.join( );
}

bool net::socketworker::connected( ) const {
    // Check if the socket pointer is null
    return (bool)_socket;
}


void net::socketworker::disconnect( ) {
    if (!connected( ))
        return;

    // Shutdown and close
    _socket->shutdown( asio::socket_base::shutdown_both );
    _socket->close( );

    _socket = nullptr;

    // Add disconnect event to the event manager
    _eventMngr.add( event( event::disconnectData( _id ) ) );
}

asio::ip::tcp::endpoint net::socketworker::endpoint( ) {
    return _endpoint;
}

void net::socketworker::send( const packet& pkt ) {
    // Lock!
    std::lock_guard<std::mutex> lock( _sendMutex );

    // Push the packet and notify condition variable so that the send thread can do its thing
    _sendQueue.front( ).push( pkt );
    _sendCV.notify_all( );
}

void net::socketworker::recvLoop( ) {

    while (connected( )) {

        // For disconnect handling
        asio::error_code ec;

        // Receive some data
        char cbuff[1024];
        size_t pckSize = _socket->receive( asio::buffer( cbuff, 1024 ), 0, ec );

        // If there was an error, disconnect and exit
        if (ec) {
            disconnect( );
            return;
        }

        // ... else, lock mutex and add event to the event manager
        else {
            std::lock_guard<std::mutex> lock( _recvMutex );
            _eventMngr.add( event( event::packetData( packet( cbuff, pckSize ), _id ) ) );
        }
    }
}

void net::socketworker::sendLoop( ) {
    while (connected( )) {
        {
            // Wait for condition variable to be notified
            // _outCV is notified when a packet is added to the out-queue, or when the socketworker is disconnected
            {
                std::unique_lock<std::mutex> lock( _sendMutex );
                _sendCV.wait( lock );
            }

            // Manual lock
            // TODO: Change to use guard_lock somehow
            _sendMutex.lock( );

            if (!_sendQueue.front( ).empty( )) {

                // Swap the queue. The mutex is then safe to unlock
                std::queue<packet>& out = _sendQueue.swap( ); //TODO: This method is not exception safe right here
                _sendMutex.unlock( );

                // Go through the queue
                while (!out.empty( )) {

                    // For disconnect checking
                    asio::error_code ec;

                    // Send it!
                    packet& pkt = out.front( );
                    _socket->send( asio::buffer( &pkt, pkt.size( ) ), 0, ec );

                    // Error handling
                    if (ec) {
                        disconnect( );
                        return;
                    }

                    // Pop!
                    out.pop( );
                }
            }

            // Unlock the mutex if there are no items in the queue
            else
                _sendMutex.unlock( );
        }
    }
}
