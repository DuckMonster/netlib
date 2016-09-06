#include "server.hpp"
#include <asio.hpp>
#include <iostream>

#include "socketworker.hpp"

net::server::server( ) :
    _service( ),
    _acceptor( _service ),

    _eventMngr( ),

    _active( false ),

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
    //_acceptThread = std::thread( &server::acceptLoop, this, port );

    _acceptThread = std::thread( &server::acceptLoop, this, port );
    _active = true;
}

void net::server::shutdown( ) {
    if (!active( ))
        return;

    _service.stop( );
    //_acceptorPtr->close( );
    _active = false;
}

bool net::server::active( ) {
    return _active;
}

void net::server::send( const net::packet & pkt, size_t id ) {

    // Trying to send to a client which is not connected
    if (_workerArray[id] == nullptr || !_workerArray[id]->connected( ))
        return;

    _workerArray[id]->send( pkt );
}

bool net::server::pollEvent( net::event & ref ) {
    return _eventMngr.poll( ref );
}

void net::server::doAccept( const short port ) {
    using asio::ip::tcp;
    
    _acceptSocket = socket_ptr( new asio::ip::tcp::socket( _service ) );
    _acceptor.async_accept( *_acceptSocket, _acceptEP, std::bind( &server::handleAccept, this, std::placeholders::_1 ) );
}

void net::server::handleAccept( asio::error_code ec ) {
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
}

void net::server::acceptLoop( const short port ) {
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

    // Start service to enable async methods
    _service.run( );
/*
//     while (_active) {
// 
//         asio::error_code ec;
// 
//         // Endpoint and socket
//         tcp::endpoint ep;
//         socket_ptr sockPtr( new tcp::socket( _service ) );
// 
//         // Do the accept!
//         _acceptorPtr->accept( *sockPtr, ep, ec );
// 
//         // Shut down in case of error
//         if (ec) {
//             shutdown( );
//             return;
//         }
// 
//         // Get ID for the new connection
//         size_t id = _workerArrayCursor;
// 
//         // Allocate the new worker at the new ID
//         _workerArray[id] = worker_ptr( new socketworker( sockPtr, id, _eventMngr ) );
// 
//         // Avance the ID cursor
//         advanceCursor( );
// 
//         // Add event to event manager
//         _eventMngr.add( event( event::connectData( id ) ) );
//     }
// 
//     _acceptorPtr = nullptr;
*/
}

void net::server::advanceCursor( ) {

    // Increment the cursor until it finds a nullptr OR the pointer points to a disconnected worker
    do {
        _workerArrayCursor = (_workerArrayCursor + 1) % _workerArray.size( );
    } while (_workerArray[_workerArrayCursor] != nullptr && _workerArray[_workerArrayCursor]->connected( ));
}
