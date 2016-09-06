# netlib
Multithreaded asynchronous server and client network library.

#Examples
##Server
```c++
// Create server object and listen to port 1520
net::server serv;
serv.startup( 1520 );

while (serv.active( )) {

    // Poll for events
    net::event e;
    while (serv.pollEvent( e )) {
        switch (e.type) {

            // Accepted connection
            case net::eConnect:
                size_t conID = e.dConnect.id;
                std::cout << "Client " << conID << " connected!\n";

                break;

                // Disconnection
            case net::eDisconnect:
                size_t disID = e.dDisconnect.id;
                std::cout << "Client " << disID << " disconnected!\n";

                break;

                // Packet
            case net::ePacket:
                size_t srcID = e.dPacket.id;
                net::packet& pkt = e.dPacket.pkt;

                std::cout << "Client " << srcID << " sent: ";

                // & operator will return char pointer to beginning of data
                // pkt.size() is number of bytes
                std::cout.write( &pkt, pkt.size( ) );
                std::cout << "\n";

                // Echo packet back to client
                serv.send( srcID, pkt );

                break;
        }
    }
}
```

Output (when running client example)
```
Client 0 connected!
Client 0 sent: Hello from client!
Client 0 disconnected!
```
##Client
```c++
// Create a client object and connect to localhost:1520
net::client cli;
cli.connect( "localhost", 1520 );

// Send a message if connection was successful
if (cli.connected( )) {
    char*           pingMsg = "Hello from client!";
    net::packet     pingPkt( pingMsg, strlen( pingMsg ) );

    cli.send( pingPkt );
}

// Main loop
while (cli.connected( )) {

    // Start polling for events
    net::event e;
    while (cli.pollEvent( e )) {
        switch (e.type) {

            // Packet
            case net::ePacket:
                net::packet& pkt = e.dPacket.pkt;

                std::cout << "Received: ";
                std::cout.write( &pkt, pkt.size( ) );
                std::cout << "\n";

                cli.disconnect( );
                break;
        }
    }
}
```

Output (when server example is running)
```
Received: Hello from client!
```
