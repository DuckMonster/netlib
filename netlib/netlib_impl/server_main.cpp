#include <server.hpp>
#include <iostream>
net::server server;

void shutdownThread( ) {
    std::this_thread::sleep_for( std::chrono::seconds( 15 ) );
    server.shutdown( );
}

int main( ) {
    server.startup( 1520 );

    std::thread thr( shutdownThread );

    while (server.active( )) {
        net::event e;

        while (server.pollEvent( e )) {
            switch (e.type( )) {
                case net::eConnect: {
                    std::cout << "Client " << e.connect( ).id << " connected!\n";

                    char* msg = "Hello from server!";
                    server.send( net::packet( msg, strlen( msg ) ), e.connect( ).id );

                } break;

                case net::eDisconnect: {
                    std::cout << "Client " << e.disconnect( ).id << " disconnected!\n";
                } break;

                case net::ePacket: {
                    std::cout << "Client " << e.packet( ).id << " sent: ";
                    std::cout.write( &e.packet( ).pkt, e.packet( ).pkt.size( ) );
                    std::cout << "\n";

                    // Echo the message
                    server.send( e.packet( ).pkt, e.packet( ).id );

                } break;
            }
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    thr.join( );
}