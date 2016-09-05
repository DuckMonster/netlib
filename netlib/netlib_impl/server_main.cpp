#include <socketworker.hpp>
#include <iostream>

using namespace std;
using asio::ip::tcp;

void handleSocket( net::socket_ptr sock ) {
    net::socketworker worker( sock, 0 );

    cout << worker.endpoint().address().to_string() << ":" << worker.endpoint( ).port( ) << " connected!\n";

    while (worker.connected( )) {
        net::packet pkt;

        if (worker.recv( pkt )) {
            cout << "Received " << pkt.size( ) << " bytes!\n";
            worker.send( pkt );
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    cout << worker.endpoint( ).address( ).to_string( ) << ":" << worker.endpoint( ).port( ) << " disconnected!\n";
}

int main( ) {
    asio::io_service service;

    tcp::acceptor acceptor( service, tcp::endpoint( tcp::v4( ), 1520 ) );

    while (true) {
        tcp::endpoint ep;
        net::socket_ptr sock( new tcp::socket( service ) );
        acceptor.accept( *sock, ep );

        std::thread thr( handleSocket, sock );
        thr.detach( );
    }
}