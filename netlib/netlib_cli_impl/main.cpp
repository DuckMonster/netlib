#include <client.hpp>
#include <iostream>

using namespace std;
using namespace net;

client c;

int main( ) {
    client c;
    c.connect( "localhost", 1520 );

    while (c.connected( )) {
        c.update( );

        event e;
        while (c.pollEvent( e )) {
            switch (e.type) {
                case net::ePacket:
                    net::packet& pkt = e.dPacket.pkt;

                    cout.write( &e.dPacket.pkt, e.dPacket.pkt.size( ) ) << "\n";

                    char* ping = "Ping from client!";
                    net::packet responsePkt( ping, strlen( ping ) );
                    c.send( responsePkt );

                    break;
            }
        }

        this_thread::sleep_for( chrono::milliseconds( 2 ) );
    }
}