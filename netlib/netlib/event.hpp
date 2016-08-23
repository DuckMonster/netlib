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
        event( ) : type( eNone ) { }
        event( EventType eType ) : type( eType ) { }

        EventType       type;
    };

    class event_connect : event {
    public:
        event_connect( size_t id ) : event( eConnect ),
            id( id ) {
        }

        size_t          id;
    };

    class event_disconnect : event {
    public:
        event_disconnect( size_t id ) : event( eDisconnect ),
            id( id ) {
        }

        size_t          id;
    };

    class event_packet : event {
    public:
        event_packet( ) : event( ePacket ) {
        }
    };

    class event_error : event {
    public:
        event_error( std::string message, int code ) : event( eError ),
            message( message ),
            code( code ) {
        }

        std::string     message;
        int             code;
    };
}