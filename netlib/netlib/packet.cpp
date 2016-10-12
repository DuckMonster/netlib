#include "packet.hpp"

using namespace net;

packet::packet( ) :
	memPtr( nullptr ),
	memSize( 0 ) {
}

packet::packet( const char* ptr, const size_t& size ) :
	packet( ) {
	setMemory( ptr, size );
}

packet::packet( const packet& other ) :
	packet( ) {
	setMemory( &other, other.size( ) );
}

packet::~packet( ) {
	if (memPtr != nullptr)
		delete[] memPtr;
}

const char* packet::begin( ) const {
	return memPtr;
}

packet & packet::operator=( const packet & other ) {
	setMemory( &other, other.size( ) );
	return *this;
}

const char * packet::operator&( ) const {
	return begin( );
}

const size_t packet::size( ) const {
	return memSize;
}

void packet::setMemory( const char * ptr, const size_t & size ) {
	char*   oldPtr = memPtr;

	// Allocate new memory and copy
	memPtr = new char[size];
	memcpy( memPtr, ptr, size );

	// New size
	memSize = size;

	// Delete old memory, if allocated
	if (oldPtr != nullptr)
		delete[] oldPtr;
}
