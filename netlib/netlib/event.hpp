#pragma once

#include <string>
#include "packet.hpp"

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
            packetData( ) : pkt( ), id( -1 ) { }
            packetData( const packet& pkt, const size_t& id ) : pkt( pkt ), id( id ) { }

            net::packet pkt;
            size_t id;
        };

        struct errorData {
            errorData( ) : message( ), code( -1 ) { }
            errorData( std::string message, int code = 0 ) : message( message ), code( code ) { }
            std::string message;
            int code;
        };

        event( ) : _type( eNone ) { }
        event( connectData dConnect ) :
            _type( eConnect ),
            _dConnect( dConnect ) {
        }

        event( disconnectData dDisconnect ) :
            _type( eDisconnect ),
            _dDisconnect( dDisconnect ) {
        }

        event( packetData dPacket ) :
            _type( ePacket ),
            _dPacket( dPacket ) {
        }

        event( errorData dError ) :
            _type( eError ),
            _dError( dError ) {
        }

        const EventType&        type( ) { return _type; }
        const connectData&      connect( ) { return _dConnect; }
        const disconnectData&   disconnect( ) { return _dDisconnect; }
        const packetData&       packet( ) { return _dPacket; }
        const errorData&        error( ) { return _dError; }

    private:
        EventType               _type;

        connectData             _dConnect;
        disconnectData          _dDisconnect;
        packetData              _dPacket;
        errorData               _dError;
    };
}