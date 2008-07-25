#ifndef COMMON_H_
#define COMMON_H_

#include <ostream>

#define SIMPLE_WRAPPED_TYPE(_name, _wrapped_type, _init)                   \
class _name {                                                              \
private:                                                                   \
  _wrapped_type value_;                                                    \
                                                                           \
public:                                                                    \
  typedef _wrapped_type value_type;                                        \
                                                                           \
  _name() : value_(_init) {                                                \
  }                                                                        \
                                                                           \
  _name(const _wrapped_type& _v) : value_(_v) {                            \
  }                                                                        \
                                                                           \
  operator _wrapped_type() const {                                         \
    return value_;                                                         \
  }                                                                        \
                                                                           \
  friend std::ostream& operator<<(std::ostream& _os, const _name& _value); \
};                                                                         \
                                                                           \
inline std::ostream& operator<<(std::ostream& _os, const _name& _v) {      \
  return _os << _v.value_;                                                 \
}


#endif /*COMMON_H_*/
