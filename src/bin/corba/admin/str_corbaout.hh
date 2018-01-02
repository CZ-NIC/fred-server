#ifndef STR_CORBAOUT_HH_4471E069481644EAA190BD3C7083BE9D
#define STR_CORBAOUT_HH_4471E069481644EAA190BD3C7083BE9D

#include "src/bin/corba/admin/common.hh"
#include "src/util/types/stringify.hh"


inline std::string str_corbaout(const char *_value) {
  return _value;
}


template<class T>
inline std::string str_corbaout(const T &_value) {
  return stringify(_value);
}


template<>
inline std::string str_corbaout(const std::string &_value) {
  return _value;
}


template<>
inline std::string str_corbaout(const ptime &_value) {
  if (_value.is_special()) {
    return std::string();
  }
  else {
    ptime local = boost::date_time::c_local_adjustor<ptime>::utc_to_local(_value);
    return local.is_special() ? std::string() : stringify(local);
  }
}

template<>
inline std::string str_corbaout(const Decimal &_value) {
  if (_value.is_special()) {
    return std::string();
  }
  else {
    return _value.get_string();
  }
}

template<>
inline std::string str_corbaout(const Database::DateTime &_value) {
  return str_corbaout<ptime>(_value);
}


template<>
inline std::string str_corbaout(const date &_value) {
  return _value.is_special() ? std::string() : stringify(_value);
}


template<>
inline std::string str_corbaout(const Database::Date &_value) {
  return str_corbaout<date>(_value);
}



#endif /*STR_CORBAOUT_H_*/

