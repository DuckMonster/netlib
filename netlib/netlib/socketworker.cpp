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

	_recvBuffer( ) {

	// Set no-delay option
	asio::ip::tcp::no_delay delay( true );
	_socket->set_option( delay );

	// Start up ASYNC yo
	doRecv( );
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

void net::socketworker::doRecv( ) {
	_socket->async_read_some( asio::buffer( _recvBuffer, 2048 ),

		[this]( asio::error_code ec, size_t size ) {
		if (ec) {
			disconnect( );
			return;
		}

		_eventMngr.add( event( event::packetData( packet( _recvBuffer, size ), _id ) ) );
		doRecv( );
	} );
}

void net::socketworker::doSend( const packet & packet ) {
	_socket->async_write_some( asio::buffer( packet.begin( ), packet.size( ) ),

		[this]( asio::error_code ec, size_t size ) {
		if (ec) {
			disconnect( );
			return;
		}
	} );
}
