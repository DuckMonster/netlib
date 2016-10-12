#pragma once
#include <string>

namespace net {
    class packet {
    public:
        packet( );
        packet( const char* ptr, const size_t& size );
        packet( const packet& other );
        ~packet( );

        const size_t    size( ) const;
		const char*		begin( ) const;

        packet&         operator=( const packet& other );
        const char*     operator&( ) const;

    private:
        char*           memPtr;
        size_t          memSize;

        void            setMemory( const char* ptr, const size_t& size );
    };
}