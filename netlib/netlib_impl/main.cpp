#include <server.hpp>
#include <iostream>
using namespace std;

int main( ) {
    net::server s;
    s.startup( 1520 );

    while (s.active( )) {
        net::event e;
        while (s.pollEvent( e )) {
            switch (e.type) {
                case net::eConnect:
                    cout << "Client " << e.dConnect.id << " connected!\n";
                    break;

                case net::eDisconnect:
                    cout << "Client " << e.dDisconnect.id << " disconnected!\n";
                    break;
            }
        }
    }
}