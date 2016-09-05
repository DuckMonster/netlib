#include <iostream>
#include <socketworker.hpp>
#include <asio.hpp>
#include <vector>

typedef std::chrono::high_resolution_clock::time_point timepoint;

int main( int argc, char** argv ) {
    asio::io_service service;

    net::socket_ptr ptr( new asio::ip::tcp::socket( service ) );
    asio::ip::tcp::resolver resolver( service );
    asio::connect( *ptr, resolver.resolve( { argv[1], argv[2] } ) );

    net::socketworker worker( ptr, 0 );
    
    while (worker.connected( )) {
        char* pingmsg = "PING!";

        net::packet temp( pingmsg, strlen( pingmsg ) );

        std::cout << "Sending... ";

        timepoint t1 = std::chrono::high_resolution_clock::now( );
        worker.send( temp );

        while (!worker.recv( temp ));// { std::this_thread::sleep_for( std::chrono::microseconds( 1 ) ); }
        timepoint t2 = std::chrono::high_resolution_clock::now( );

        std::cout << "Received! ( " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count( ) << " ms )\n";

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
}