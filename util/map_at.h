#ifndef MAP_AT_H_
#define MAP_AT_H_

#include <stdexcept>


template<typename T>
const typename T::mapped_type& map_at(
        const T &_container, const typename T::key_type &_key)
{
    typename T::const_iterator it;
    if ((it = _container.find(_key)) == _container.end()) {
        throw std::out_of_range("map_at: not found");
    }
    return it->second;
}


#endif /*MAP_AT_H_*/
