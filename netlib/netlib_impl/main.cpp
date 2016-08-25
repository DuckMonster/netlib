#include <server.hpp>
#include <iostream>
#include <array>

using namespace std;

class cli {
public:
    cli( ) : id( -1 ) { }
    cli( const size_t& id ) : id( id ) { }
    size_t id;
};

net::server s;
array<cli, 256> clients;

void sendThread( ) {
    while (true) {
        char* ping = "Ping from server!";
        net::packet pkt( ping, strlen( ping ) );

        for (cli c : clients)
            if (c.id != -1)
                s.send( c.id, pkt );

        this_thread::sleep_for( chrono::seconds( 1 ) );
    }
}

int main( ) {
    s.startup( 1520 );

    std::thread thr( &sendThread );

    while (s.active( )) {
        s.update( );
        net::event e;
        while (s.pollEvent( e )) {
            switch (e.type) {
                case net::eConnect:
                    cout << "Client " << e.dConnect.id << " connected!\n";

                    clients[e.dConnect.id] = cli( e.dConnect.id );

                    break;

                case net::eDisconnect:
                    cout << "Client " << e.dDisconnect.id << " disconnected!\n";

                    clients[e.dDisconnect.id] = cli( );

                    break;

                case net::ePacket:
                    net::packet& pkt = e.dPacket.pkt;

                    cout << "Client " << e.dPacket.id << ": ";
                    cout.write( &e.dPacket.pkt, e.dPacket.pkt.size( ) );
                    cout << "\n";

                    break;
            }
        }

        this_thread::sleep_for( chrono::milliseconds( 2 ) );
    }
}