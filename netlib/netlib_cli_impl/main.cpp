#include <iostream>
#include <client.hpp>

int main( ) {
    // Create a client object and connect to localhost:1520
    net::client cli;
    cli.connect( "localhost", 1520 );

    while (cli.connected( )) {
        char data[10];

        net::packet pkt( data, 10 );
        cli.send( pkt );

        //std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }
}