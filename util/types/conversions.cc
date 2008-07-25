#include "conversions.h"

namespace Database {

/*
 * conversion definitions for plain old data types
 */

short Conversion<short>::from_string(const std::string& _value) {
  return atoi(_value.c_str());
}


std::string Conversion<short>::to_string(const short& _value) {
  return signed2string(_value);
}


int Conversion<int>::from_string(const std::string& _value) {
  return atoi(_value.c_str());
}


std::string Conversion<int>::to_string(const int& _value) {
  return signed2string(_value);
}


long Conversion<long>::from_string(const std::string& _value) {
  return atol(_value.c_str());
}


std::string Conversion<long>::to_string(const long& _value) {
  return signed2string(_value);
}


long long Conversion<long long>::from_string(const std::string& _value) {
  return atoll(_value.c_str());
}


std::string Conversion<long long>::to_string(const long long& _value) {
  return signed2string(_value);
}


unsigned int Conversion<unsigned int>::from_string(const std::string& _value) {
  return atoi(_value.c_str());
}


std::string Conversion<unsigned int>::to_string(const unsigned int& _value) {
  return unsigned2string(_value);
}


unsigned long Conversion<unsigned long>::from_string(const std::string& _value) {
  return atoll(_value.c_str());
}


std::string Conversion<unsigned long>::to_string(const unsigned long& _value) {
  return unsigned2string(_value);
}


unsigned long long Conversion<unsigned long long>::from_string(const std::string& _value) {
  return atoll(_value.c_str());
}


std::string Conversion<unsigned long long>::to_string(const unsigned long long& _value) {
  return unsigned2string(_value);
}


bool Conversion<bool>::from_string(const std::string& _value) {
  switch (_value[0]) {
    case 't': return true;
    case 'f': return false;
    default: throw;
  }
}


std::string Conversion<bool>::to_string(const bool& _value) {
  switch (_value) {
    case true: return "t";
    case false: return "f";
    default: throw;
  }
}


CONVERSION_DEFINITION(DateTime)
CONVERSION_DEFINITION(Date)
CONVERSION_DEFINITION(ID)
CONVERSION_DEFINITION(Money)


std::string Conversion<std::string>::from_string(const std::string& _value) {
  return _value;
}


std::string Conversion<std::string>::to_string(const std::string& _value) {
  return _value;
}

}

