#include "socketworker.hpp"
#include <iostream>

#include "lock_break.hpp"

//// SIZE HEADER UTILS
// Convert char* to short
short ctos( char* const val ) {
	return *reinterpret_cast<short*>(val);
}

// Convert short to char*
char* stoc( short& val ) {
	return reinterpret_cast<char*>(&val);
}

using namespace std;
typedef std::chrono::high_resolution_clock::time_point time_point;
using std::chrono::high_resolution_clock;

time_point prev;

net::socketworker::socketworker( const socket_ptr& socket, const size_t socketID, eventmanager& eventMngr ) :
	_socket( socket ),
	_endpoint( socket->remote_endpoint( ) ),
	_id( socketID ),

	_eventMngr( eventMngr ),

	_sendMtx( ),
	_sendCV( ),

	_recvBuffer( ),
	_sendBuffer( ),

	_currentlySending( false ) {

	// Set no-delay option
	asio::ip::tcp::no_delay delay( true );
	_socket->set_option( delay );

	// Start up ASYNC yo
	doRecv( 0 );
}

net::socketworker::~socketworker( ) {
	// Disconnect and join the threads
	disconnect( );
}

bool net::socketworker::connected( ) const {
	// Check if the socket pointer is null
	return (bool)_socket;
}

void net::socketworker::disconnect( ) {
	// Lock send and receive

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
	doSend( pkt );
}

void net::socketworker::doRecv( const size_t index ) {
	// Receive length
	size_t length = 0;

	// Header of 2 bytes
	if (index == 0) length = 2;
	// Otherwise receive length specified by header
	else length = ctos( _recvBuffer );

	asio::async_read( *_socket, asio::buffer( _recvBuffer + index, length ),

		[this, index]( asio::error_code ec, size_t size ) {
		if (ec) {
			disconnect( );
			return;
		}

		// This was a header
		if (index == 0)
			doRecv( index + size );

		// This was data
		else {
			_eventMngr.add( event( event::packetData( packet( _recvBuffer + 2, size ), _id ) ) );
			doRecv( 0 );
		}
	} );
}

void net::socketworker::doSend( const packet & packet ) {
	std::unique_lock<std::mutex> lock( _sendMtx );

	while (_currentlySending)
		_sendCV.wait( lock );

	short sSize = packet.size( );
	char* header = stoc( sSize );

	memcpy( _sendBuffer, header, 2 );
	memcpy( _sendBuffer + 2, packet.begin( ), packet.size( ) );

	_socket->async_write_some( asio::buffer( _sendBuffer, packet.size( ) + 2 ),

		[this]( asio::error_code ec, size_t size ) {
		std::unique_lock<std::mutex> lock( _sendMtx );

		_currentlySending = false;
		_sendCV.notify_one( );

		if (ec) {
			disconnect( );
			return;
		}
	} );
}
