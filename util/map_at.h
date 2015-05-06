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
/**
 * if value is found in map by the key, return set optional map value, if not return unset optional map value
 * ReturnOptionalTypeTemplate might be Optional or Nullable template
 */
template<template <typename> class ReturnOptionalTypeTemplate, typename T>
ReturnOptionalTypeTemplate<typename T::mapped_type> optional_map_at(
        const T &_container, const typename T::key_type &_key)
{
    typename T::const_iterator it;
    if ((it = _container.find(_key)) == _container.end()) {
        return ReturnOptionalTypeTemplate<typename T::mapped_type>();
    }
    return ReturnOptionalTypeTemplate<typename T::mapped_type>(it->second);
}

template<typename CONTAINER, class EXCEPTION>
const typename CONTAINER::mapped_type& map_at_ex(
        const CONTAINER &_container
        , const typename CONTAINER::key_type &_key
        ,  EXCEPTION ex)//exception to throw when _key not fond in _container
{
    typename CONTAINER::const_iterator it;
    if ((it = _container.find(_key)) == _container.end()) {
        throw ex;
    }
    return it->second;
}


#endif /*MAP_AT_H_*/
