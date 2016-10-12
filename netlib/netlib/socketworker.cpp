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

	_recvThread( ),
	_sendThread( ),

	_sendQueue( ) {

	// Set no-delay option
	asio::ip::tcp::no_delay delay( true );
	_socket->set_option( delay );

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
	// Lock send and receive
	std::lock_guard<std::mutex> outLock( _sendMutex ), inLock( _recvMutex );

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
	char cbuff[2 << 16];
	char szBuff[2];

	while (connected( )) {

		// For disconnect handling
		asio::error_code ec;

		// Receive data size
		asio::read( *_socket, asio::buffer( szBuff, 2 ), ec );

		// Convert char* to short
		short dataSize = ctos( szBuff );

		if (dataSize == 0)
			continue;

		// Receive dataSize bytes
		asio::read( *_socket, asio::buffer( cbuff, dataSize ), ec );

		// If there was an error, disconnect and exit
		if (ec) {
			disconnect( );
			return;
		}


		// ... else, lock mutex and add event to the event manager
		else {
			lock_guard<std::mutex> lock( _recvMutex );
			_eventMngr.add( event( event::packetData( packet( cbuff, dataSize ), _id ) ) );
		}
	}
}

void net::socketworker::sendLoop( ) {
	while (connected( )) {
		{
			// Manual lock
			unique_lock<mutex> lock( _sendMutex );

			while (!_sendQueue.front( ).empty( )) {

				// Swap the queue. The mutex is then safe to unlock
				std::queue<packet>& out = _sendQueue.swap( ); //TODO: This method is not exception safe right here

				// Unlock the lock temporarily using lock_break
				lock_break brk( lock );

				while (!out.empty( )) {
					asio::error_code ec;

					packet& pkt = out.front( );

					short sPktSize = pkt.size( );
					char* cPktSize = stoc( sPktSize );

					// Send packet size
					_socket->write_some( asio::buffer( cPktSize, 2 ), ec );
					// Send packet
					_socket->write_some( asio::buffer( pkt.begin( ), pkt.size( ) ), ec );

					// Error handling
					if (ec) {
						disconnect( );
						return;
					}

					out.pop( );
				}
			}

			// Wait for send queue to be populated
			_sendCV.wait( lock );
		}
	}
}
