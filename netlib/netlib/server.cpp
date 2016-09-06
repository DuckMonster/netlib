#include "server.hpp"
#include <asio.hpp>
#include <iostream>

#include "socketworker.hpp"

net::server::server( ) :
    _service( ),
    _acceptor( _service ),

    _eventMngr( ),

    _workerArray( ),
    _workerArrayCursor( 0 ),

    _acceptThread( ) {
}

net::server::~server( ) {

    shutdown( );

    if (_acceptThread.joinable( ))
        _acceptThread.join( );
}

void net::server::startup( const short port ) {

    // Lock accept
    std::unique_lock<std::mutex> lock( _acceptMutex );

    // Start up accept init-thread
    _acceptThread = std::thread( &server::acceptInit, this, port );

    // Wait for condition variable to be notified, or 2 seconds if an error occured
    _acceptCV.wait_for( lock, std::chrono::seconds( 2 ) );
}

void net::server::shutdown( ) {
    if (!active( ))
        return;

    _service.stop( );

    // Disconnect all clients and set pointers to null
    for (worker_ptr& ptr : _workerArray)
        if (ptr) {
            ptr->disconnect( );
            ptr = nullptr;
        }
}

bool net::server::active( ) {
    return !_service.stopped( );
}

void net::server::send( const net::packet & pkt, size_t id ) {

    // Trying to send to a client which is not connected
    if (_workerArray[id] == nullptr || !_workerArray[id]->connected( ))
        return;
    //TODO: Throw exception?

    _workerArray[id]->send( pkt );
}

bool net::server::pollEvent( net::event & ref ) {
    return _eventMngr.poll( ref );
}

void net::server::acceptInit( const short port ) {
    using asio::ip::tcp;

    // Listening endpoint
    tcp::endpoint listenEP( tcp::v4( ), port );

    // Set up the acceptor to listen to port
    _acceptor.open( listenEP.protocol( ) );
    _acceptor.set_option( tcp::acceptor::reuse_address( true ) );
    _acceptor.bind( listenEP );
    _acceptor.listen( );

    // Start accepting
    doAccept( port );

    // Service has started, notify CV
    _acceptCV.notify_all( );

    // Start service to enable async methods
    _service.run( );
}

void net::server::doAccept( const short port ) {
    using asio::ip::tcp;

    _acceptSocket = socket_ptr( new asio::ip::tcp::socket( _service ) );
    _acceptor.async_accept( *_acceptSocket, _acceptEP, std::bind( &server::handleAccept, this, std::placeholders::_1, port ) );
}

void net::server::handleAccept( asio::error_code ec, const short port ) {
    if (ec) {
        shutdown( );
        return;
    }

    // Get ID for the new connection
    size_t id = _workerArrayCursor;

    // Allocate the new worker at the new ID
    _workerArray[id] = worker_ptr( new socketworker( _acceptSocket, id, _eventMngr ) );

    // Avance the ID cursor
    advanceCursor( );

    // Add event to event manager
    _eventMngr.add( event( event::connectData( id ) ) );

    // Remove accept socket
    _acceptSocket = nullptr;

    // Accept again
    doAccept( port );
}

void net::server::advanceCursor( ) {

    // Increment the cursor until it finds a nullptr OR the pointer points to a disconnected worker
    do {
        _workerArrayCursor = (_workerArrayCursor + 1) % _workerArray.size( );
    } while (_workerArray[_workerArrayCursor] != nullptr && _workerArray[_workerArrayCursor]->connected( ));
}
