#include "packet.hpp"

using namespace net;

packet::packet( const char* ptr, const size_t& size ) :
    memPtr( nullptr ),
    memSize( 0 ) {
    setMemory( ptr, size );
}

packet::packet( const packet& other ) :
    memPtr( nullptr ),
    memSize( 0 ) {

}

packet::~packet( ) {

}

packet & packet::operator=( const packet & other ) {
    setMemory( &other, other.size( ) );
    return *this;
}

const char * packet::operator&( ) const {
    return memPtr;
}

const size_t packet::size( ) const {
    return memSize;
}

void packet::setMemory( const char * ptr, const size_t & size ) {
    char*   oldPtr = memPtr;

    memPtr = new char[size];
    memcpy( memPtr, ptr, size );

    if (oldPtr != nullptr)
        delete[] oldPtr;
}
