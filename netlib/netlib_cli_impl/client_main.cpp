#include <iostream>
#include <socketworker.hpp>
#include <asio.hpp>

int main( ) {
    asio::io_service service;

    net::socket_ptr ptr( new asio::ip::tcp::socket( service ) );
    asio::connect( *ptr,   )

    net::socketworker
}