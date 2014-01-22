#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>
#include "types/data_types.h"

namespace Util {

//for given instance first call of get() (or first call of get() after reset() ) returns head , all other calls returns separator
//
class HeadSeparator
{
    bool got_head_;
    std::string head_;
    std::string separator_;
public:

    HeadSeparator(const std::string& head, const std::string& separator)
    : got_head_(false)
    , head_(head)
    , separator_(separator)
    {}

    std::string get()
    {
        if(got_head_)
        {
            return separator_;
        }
        else
        {
            got_head_ = true;
            return head_;
        }
    }

    void reset()
    {
        got_head_ = false;
    }
};


//template for initialization of vector
template <typename ELEMENT_TYPE > struct vector_of
    : public std::vector<ELEMENT_TYPE>
{
    //appends one element
    vector_of(const ELEMENT_TYPE& t)
    {
        (*this)(t);
    }
    vector_of& operator()(const ELEMENT_TYPE& t)
    {
        this->push_back(t);
        return *this;
    }
    //appends vector of the same elements
    /*
    vector_of(const std::vector<ELEMENT_TYPE>& v)
    {
        (*this)(v);
    }
    vector_of& operator()(const std::vector<ELEMENT_TYPE>& v)
    {
        this->insert(this->end(), v.begin(), v.end());
        return *this;
    }
    */
};


template<class T>
std::string container2comma_list(const T &_cont)
{
    if (_cont.empty()) {
        return "";
    }

    std::stringstream tmp;
    typename T::const_iterator it = _cont.begin();
    tmp << *it;
    for (++it; it != _cont.end(); ++it) {
        tmp << ", " << *it;
    }
    return tmp.str();
}

inline std::string escape(std::string _input,
                   const std::string _what = "'\\",
                   const std::string _esc_char = "\\") {

  size_t i = 0;
  while ((i = _input.find_first_of(_what, i)) != _input.npos) {
    _input.replace(i, 1, _esc_char + _input[i]);
    i += _esc_char.length() + 1;
  }
  return _input;
}


inline std::string escape2(std::string _input) {
  const std::string _what = "'\\";
  const std::string _esc_char = "\\";
  size_t i = 0;
  while ((i = _input.find_first_of(_what, i)) != _input.npos) {
    _input.replace(i, 1, _esc_char + _input[i]);
    i += _esc_char.length() + 1;
  }
  return _input;
}

std::string make_svtrid(unsigned long long request_id);


/**
 * Makes type from enum value
 * @param VALUE is value of the enum
 */
template <int VALUE>
struct EnumType
{
   enum { value = VALUE };
};

}//namespace Util

#endif /*UTIL_H_*/
