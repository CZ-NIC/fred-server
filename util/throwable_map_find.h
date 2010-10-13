#ifndef THROWABLE_MAP_FIND_H_
#define THROWABLE_MAP_FIND_H_

#include <stdexcept>


template<typename T>
const typename T::mapped_type& throwable_map_find(
        const T &_container, const typename T::key_type &_key)
{
    typename T::const_iterator it;
    if ((it = _container.find(_key)) == _container.end()) {
        throw std::out_of_range("throwable_map_find: not found");
    }
    return it->second;
}


#endif /*THROWABLE_MAP_FIND_H_*/
