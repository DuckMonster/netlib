#include "eventmanager.hpp"

void net::eventmanager::add( const net::event & e ) {

    // Always lock when adding events
    std::lock_guard<std::mutex> lock( _mtx );
    _eventQueue.push( e );
}

bool net::eventmanager::poll( net::event & ref ) {

    // Only try to lock when polling events
    std::unique_lock<std::mutex> lock( _mtx, std::try_to_lock );

    // If lock was aquired
    if (lock) {
        // No queues available
        if (_eventQueue.empty( ))
            return false;

        ref = _eventQueue.front( );
        _eventQueue.pop( );

        return true;
    }
    else return false;
}
