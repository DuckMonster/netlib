#pragma once
#include <queue>

template<class T>
class bufferedqueue {
public:
    bufferedqueue( );

    std::queue<T>&      swap( );
    std::queue<T>&      operator->( );

    std::queue<T>&      front( );
    std::queue<T>&      back( );

private:
    std::queue<T>       queues[2];
    size_t              queueIndex = 0;
};

template<class T>
inline bufferedqueue<T>::bufferedqueue( ) {
}

template<class T>
inline std::queue<T>& bufferedqueue<T>::swap( ) {
    queueIndex++;
    return back( );
}

template<class T>
inline std::queue<T>& bufferedqueue<T>::operator->( ) {
    return front( );
}

template<class T>
inline std::queue<T>& bufferedqueue<T>::front( ) {
    return queues[queueIndex % 2];
}

template<class T>
inline std::queue<T>& bufferedqueue<T>::back( ) {
    return queues[(queueIndex + 1) % 2];
}
