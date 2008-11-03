#ifndef LOGGING_H_
#define LOGGING_H_

#include <ostream>
#include "config.h"

#ifdef HAVE_BOOST_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#endif


namespace Database {

enum _LogEventType_ { LT_REQUEST, LT_RESPONSE };

enum _LogComponent_ { LC_UNIX_WHOIS, LC_WEB_WHOIS, LC_EPP };

class LogEventType {
private:    
  _LogEventType_ value;

public:
  LogEventType() : value(LT_REQUEST) {
  }
  
  LogEventType(const _LogEventType_& _v) : value(_v) {
  }                                                                        
                                                                           
  operator _LogEventType_() const {                                         
    return value;                                                         
  }                                                                        
                                                                           
  friend bool operator==(const LogEventType& _left, const LogEventType& _right);	   
  friend std::ostream& operator<<(std::ostream& _os, const LogEventType& _value); 
									   
#ifdef HAVE_BOOST_SERIALIZATION						   
  /* boost serialization */						   
  friend class boost::serialization::access;				   
  template<class Archive> void serialize(Archive& _ar,			   
      const unsigned int _version) {					   
    _ar & BOOST_SERIALIZATION_NVP(value);				   
  }									   
#endif									   
									   
};                                                                         
                                                                           
inline std::ostream& operator<<(std::ostream& _os, const LogEventType& _v) {      
  return _os << _v.value;                                                 
}									   
									   
inline bool operator==(const LogEventType& _left, const LogEventType& _right) {	   
  return (_left.value==_right.value);						   
}


class LogComponent {
private:    
  _LogComponent_ value;

public:
  LogComponent() : value(LC_UNIX_WHOIS) {
  }
  
  LogComponent(const _LogComponent_& _v) : value(_v) {
  }                                                                        
                                                                           
  operator _LogComponent_() const {                                         
    return value;                                                         
  }                                                                        
                                                                           
  friend bool operator==(const LogComponent& _left, const LogComponent& _right);	   
  friend std::ostream& operator<<(std::ostream& _os, const LogComponent& _value); 
									   
#ifdef HAVE_BOOST_SERIALIZATION						   
  /* boost serialization */						   
  friend class boost::serialization::access;				   
  template<class Archive> void serialize(Archive& _ar,			   
      const unsigned int _version) {					   
    _ar & BOOST_SERIALIZATION_NVP(value);				   
  }									   
#endif									   
									   
};                                                                         
                                                                           
inline std::ostream& operator<<(std::ostream& _os, const LogComponent& _v) {      
  return _os << _v.value;                                                 
}									   
									   
inline bool operator==(const LogComponent& _left, const LogComponent& _right) {	   
  return (_left.value==_right.value);						   
}
};


#endif /*LOGGING_H_*/
