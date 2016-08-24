#include <client.hpp>
#include <iostream>

using namespace std;
using namespace net;

client c;

void sendThread( ) {
    while (c.connected( )) {
        packet pkt( "Hello!", 6 );
        c.send( pkt );

        this_thread::sleep_for( chrono::seconds( 1 ) );
    }
}

int main( ) {
    client c;
    c.connect( "localhost", 1520 );

    thread thr( sendThread );

    while (c.connected( )) {
        c.update( );

        event e;
        while (c.pollEvent( e )) {
            switch (e.type) {
                case net::ePacket:
                    net::packet& pkt = e.dPacket.pkt;

                    cout << "Server: ";
                    cout.write( &e.dPacket.pkt, e.dPacket.pkt.size( ) );
                    cout << "\n";

                    break;
            }
        }
    }
}