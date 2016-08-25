#include <server.hpp>
#include <iostream>
#include <array>

using namespace std;
using namespace std::chrono;

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

long long millisecs = 0;
long msgs = 0;
long in = 0;
long out = 0;

int main( ) {
    s.startup( 1520 );

    //std::thread thr( &sendThread );

    high_resolution_clock::time_point t1;

    while (s.active( )) {
        s.update( );
        net::event e;
        while (s.pollEvent( e )) {
            switch (e.type) {
                case net::eConnect:
                    //cout << "Client " << e.dConnect.id << " connected!\n";

                    clients[e.dConnect.id] = cli( e.dConnect.id );

                    break;

                case net::eDisconnect:
                    //cout << "Client " << e.dDisconnect.id << " disconnected!\n";

                    clients[e.dDisconnect.id] = cli( );

                    break;

                case net::ePacket:
                    net::packet& pkt = e.dPacket.pkt;

                    /*cout << "Client " << e.dPacket.id << ": ";
                    cout.write( &e.dPacket.pkt, e.dPacket.pkt.size( ) );
                    cout << "\n";*/

                    msgs++;
                    in += pkt.size( );

                    for (cli c : clients)
                        if (c.id != -1) {
                            s.send( c.id, e.dPacket.pkt );
                            out += pkt.size( );
                        }

                    break;
            }
        }

        high_resolution_clock::time_point t2 = high_resolution_clock::now( );
        auto duration = duration_cast<microseconds>(t2 - t1).count( );

        t1 = high_resolution_clock::now( );

        millisecs += duration;
        if (millisecs >= 1000000) {
            cout << "Handled " << msgs << " messages ( ";
            cout << "in: " << in << " B, out: " << out << " B ) in ";
            cout << (duration / 1000.0) << " ms\n";
            millisecs = 0;

            msgs = 0;
            in = 0;
            out = 0;
        }


        this_thread::sleep_for( chrono::microseconds( 1 ) );
    }
}