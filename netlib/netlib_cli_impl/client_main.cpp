#include <iostream>
#include <vector>
#include <client.hpp>

typedef std::chrono::high_resolution_clock::time_point time_point;

net::client client;

time_point send_tp;

void senderThread( ) {
    while (client.connected( )) {
        char* ping = "Ping!";
        client.send( net::packet( ping, strlen( ping ) ) );
        send_tp = std::chrono::high_resolution_clock::now( );

        std::cout << "Sent ping!\n";

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
}

int main( int argc, char** argv ) {
    client.connect( { argv[1], argv[2] } );

    std::thread thr( senderThread );

    while (client.connected( )) {
        net::event e;

        while (client.pollEvent( e )) {
            switch (e.type( )) {
                case net::ePacket: {
                    time_point receive_tp( std::chrono::high_resolution_clock::now( ) );
                    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(receive_tp - send_tp).count( );

                    std::cout << "Server: ";
                    std::cout.write( &e.packet( ).pkt, e.packet( ).pkt.size( ) );
                    std::cout << " ( " << ms << " ms )\n";

                } break;
            }
        }


        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    thr.join( );

    std::cin.get( );
}