#ifndef MAP_GET_H_
#define MAP_GET_H_

#include <vector>
#include <algorithm>


template<typename T>
struct KeyGetter
{
    typedef typename T::first_type getter_type;

    getter_type operator ()(const T &_pair) const
    {
        return _pair.first;
    }
};



template<typename T>
struct ValueGetter
{
    typedef typename T::second_type getter_type;

    getter_type operator ()(const T &_pair) const
    {
        return _pair.second;
    }
};



template<typename T>
std::vector<typename T::key_type> map_get_keys(const T &_map)
{
    std::vector<typename T::key_type> tmp;
    std::transform(
            _map.begin(),
            _map.end(),
            std::back_inserter(tmp),
            KeyGetter<typename T::value_type>());
    return tmp;
}



template<typename T>
std::vector<typename T::mapped_type> map_get_values(const T &_map)
{
    std::vector<typename T::mapped_type> tmp;
    std::transform(
            _map.begin(),
            _map.end(),
            std::back_inserter(tmp),
            ValueGetter<typename T::value_type>());
    return tmp;
}



#endif /*MAP_GET_H_*/

