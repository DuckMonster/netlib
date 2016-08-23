#pragma once

#include <string>

namespace net {
    enum EventType {
        eNone,
        eConnect,
        eDisconnect,
        ePacket,
        eError
    };

    class event {
    public:
        struct connectData {
            connectData( ) : id( -1 ) { }
            connectData( size_t id ) : id( id ) { }
            size_t id;
        };

        struct disconnectData {
            disconnectData( ) : id( -1 ) { }
            disconnectData( size_t id ) : id( id ) { }
            size_t id;
        };

        struct packetData {
            packetData( ) { }
        };

        struct errorData {
            errorData( ) : message( ), code( -1 ) { }
            errorData( std::string message, int code = 0 ) : message( message ), code( code ) { }
            std::string message;
            int code;
        };

        event( ) : type( eNone ) { }
        event( connectData dConnect ) :
            type( eConnect ),
            dConnect( dConnect ) {
        }

        event( disconnectData dDisconnect ) :
            type( eDisconnect ),
            dDisconnect( dDisconnect ) {
        }

        event( packetData dPacket ) :
            type( ePacket ),
            dPacket( dPacket ) {
        }

        event( errorData dError ) :
            type( eError ),
            dError( dError ) {
        }

        EventType       type;

        connectData     dConnect;
        disconnectData  dDisconnect;
        packetData      dPacket;
        errorData       dError;
    };
}