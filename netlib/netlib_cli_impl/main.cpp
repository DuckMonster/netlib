#include <client.hpp>
#include <iostream>

using namespace std;
using namespace net;

client c;

int main( ) {
    client c;

    c.connect( "localhost", 1520 );

    while (c.connected( )) {

        char msg[512];
        packet pkt( msg, 512 );

        c.send( pkt );

        net::event e;

        c.update( );
        while (c.pollEvent( e ));

        this_thread::sleep_for( chrono::milliseconds( 10 ) );
    }
}