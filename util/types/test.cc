#include <iostream>
//#include "conversions.h"
#include "id.h"

int main() {
  #define COUT(v) \
  std::cout << v << std::endl;


//  std::string num1 = Conversion<int>::to_string(-1230000999);
//  COUT(num1)
//  COUT(Conversion<int>::from_string(num1))
//
//  std::string num2 = Conversion<unsigned>::to_string(112);
//  COUT(num2)
//  COUT(Conversion<unsigned>::from_string(num2))
//
//  std::string num3 = Conversion<long long>::to_string(317584931803LL);
//  COUT(num3)
//  COUT(Conversion<long long>::from_string(num3))
//
//  std::string num4 = Conversion<bool>::to_string(true);
//  COUT(num4)
//  COUT(Conversion<bool>::from_string(num4))
//  bool num5 = Conversion<bool>::from_string("t");
//  COUT(num5)
//  COUT(Conversion<bool>::to_string(num5))
 

  ID id = 10;
  COUT(id.to_string())

  return 0;
}
