#include "socketworker.hpp"
#include <iostream>

net::socketworker::socketworker( const socket_ptr& socket, const size_t socketID ) :
    _socket( socket ),
    _endpoint( socket->remote_endpoint( ) ),
    _id( socketID ),

    _recvThread( ),
    _sendThread( ),

    _outQueue( ) {

    // Start up the threads
    _recvThread = std::thread( &socketworker::recvLoop, this );
    _sendThread = std::thread( &socketworker::sendLoop, this );
}

net::socketworker::~socketworker( ) {
    // Disconnect and join the threads
    disconnect( );

    // Aquire lock and then notify, to ensure proper thread shutdown
    {
        std::lock_guard<std::mutex> outLock( _outMutex ), inLock( _inMutex );
        _outCV.notify_all( );
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
}

asio::ip::tcp::endpoint net::socketworker::endpoint( ) {
    return _endpoint;
}

void net::socketworker::send( const packet& pkt ) {
    std::lock_guard<std::mutex> lock( _outMutex );

    _outQueue.front( ).push( pkt );
    _outCV.notify_all( );
}

bool net::socketworker::recv( packet& pkt ) {
    std::unique_lock<std::mutex> lock( _inMutex, std::try_to_lock );

    if (lock.owns_lock( )) {
        if (_inQueue.empty( ))
            return false;

        pkt = _inQueue.front( );
        _inQueue.pop( );

        return true;
    }
    else
        return false;
}

void net::socketworker::recvLoop( ) {
    while (connected( )) {

        asio::error_code ec;
        char cbuff[1024];

        size_t pckSize = _socket->receive( asio::buffer( cbuff, 1024 ), 0, ec );

        if (ec) {
            disconnect( );
            return;
        }
        else {
            std::lock_guard<std::mutex> lock( _inMutex );
            _inQueue.push( packet( cbuff, pckSize ) );
        }
    }
}

void net::socketworker::sendLoop( ) {
    while (connected( )) {
        {
            {
                std::unique_lock<std::mutex> lock( _outMutex );
                _outCV.wait( lock );
            }

            _outMutex.lock( );

            if (!_outQueue.front( ).empty( )) {

                std::queue<packet>& out = _outQueue.swap( );
                _outMutex.unlock( );

                while (!out.empty( )) {

                    packet& pkt = out.front( );

                    asio::error_code ec;
                    _socket->send( asio::buffer( &pkt, pkt.size( ) ), 0, ec );

                    // Error handling
                    if (ec) {
                        disconnect( );
                        return;
                    }

                    out.pop( );
                }
            }
            else
                _outMutex.unlock( );
        }
    }
}
