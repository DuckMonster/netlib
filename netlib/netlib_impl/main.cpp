#include <server.hpp>
#include <iostream>
#include <array>

int main( ) {
    // Create server object and listen to port 1520
    net::server serv;
    serv.startup( 1520 );

    while (serv.active( )) {

        // Will update client incoming messages, as well as disconnects
        serv.update( );

        // Poll for events
        net::event e;
        while (serv.pollEvent( e )) {
            size_t clientID;

            switch (e.type) {

                // Accepted connection
                case net::eConnect:
                    clientID = e.dConnect.id;
                    std::cout << "Client " << clientID << " connected!\n";

                    break;

                    // Disconnection
                case net::eDisconnect:
                    clientID = e.dDisconnect.id;
                    std::cout << "Client " << clientID << " disconnected!\n";

                    break;

                    // Packet
                case net::ePacket:
                    if (e.dPacket.pkt.size( ) != 10)
                        std::cout << "BAD SIZE!\n";
                    else
                        std::cout << "GOOD SIZE!\n";

                    break;
            }
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 15 ) );
    }
}