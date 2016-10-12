#pragma once
#include <mutex>

// A helper class for temporarily unlocking a unique_lock
class lock_break {
public:
	lock_break( std::unique_lock<std::mutex>& lock ) :
		_lock( lock ) {

		_lock.unlock( );
	}

	~lock_break( ) {
		_lock.lock( );
	}

private:
	std::unique_lock<std::mutex>& _lock;
};