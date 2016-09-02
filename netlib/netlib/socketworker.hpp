#pragma once
#include <asio.hpp>
#include <memory>
#include <thread>

#include "packet.hpp"
#include "bufferedqueue.hpp"

namespace net {
    typedef std::shared_ptr<asio::ip::tcp::socket> socket_ptr;

    class socketworker {
    public:
        socketworker( const socket_ptr& socket, const size_t socketID );

        // Returns if this socketworker is connected and active
        bool                    connected( ) const;
        void                    disconnect( );

    private:
        void                    recvLoop( );
        void                    sendLoop( );

        //----------------

        socket_ptr              socket;
        size_t                  id;

        std::thread             recvThread;
        std::thread             sendThread;
        
        bufferedqueue<packet>   outQueue;
    };
}