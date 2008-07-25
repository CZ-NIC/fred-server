#include <iostream>
#include <vector>
#include <algorithm>

#include "log/logger.h"
#include "manager.h"
#include "types/datetime.h"

int main() {
  using namespace Database;

  Logging::Manager::instance_ref().get("db").addHandler(Logging::Log::LT_CONSOLE); 
  Logging::Manager::instance_ref().get("db").setLevel(Logging::Log::LL_DEBUG);

  Connection conn;
  try {
    conn.open("host=localhost dbname=fred user=fred");
    
    Transaction t(conn);

    Result r = conn.exec("SELECT roid, name, crdate FROM object_registry");
    std::cout << "size: " << r.size() << std::endl;

    std::string str = r[0][0];
    std::cout << r[0][0] << "  " << str << std::endl;
  
    Result::Iterator it1 = r.begin();
    Result::Iterator it2 = r.end();

    for (; it1 != it2; ++it1) {
      for (Row::size_type i = 0; i < (*it1).size(); ++i) {
        std::cout << (*it1)[i] << "   ";
      }
      std::cout << std::endl;
    }
  
  //  it1 = r.begin();
  //  for (; it1 != it2; ++it1) {
  //    for (Row::Iterator it3 = (*it1).begin(); it3 != (*it1).end(); ++it3) {
  //      std::cout << *it3 << "   ";
  //    }
  //    std::cout << std::endl;
  //  }
  
    it1 = r.begin();
    for (; it1 != it2; ++it1) {
      Row::Iterator col = (*it1).begin();

      std::string roid = *col;
      std::string name = *(++col);
      DateTime crdate  = *(++col);
      // std::string roid = row["roid"];
      // std::string name = row["name"];
      // DateTime crdate  = row["crdate"];
  
      std::cout << Value(roid) << std::endl;
      // << "     " << name << "     " << crdate << std::endl;
    }

    //t.commit();
    
  }
  catch (Database::Exception& ex) {
    std::cout << ex.what() << std::endl;
  }
 
//  for (; it1 != r.end(); ++it1) {
//    Value o_roid = (*it1)["roid"];
//    Value o_name = (*it1)["name"];
//
//    std::cout << "roid = " << o_roid << "  name = " << o_name << std::endl;
//  }

  
//  Result::Iterator it1 = r.begin() += 2;
//  Result::Iterator it2 = r.end() - 1;

//  if (it1 == it2) {
//    std::cout << "(it1 == it2) => " << *it1 << " == " << *it2 << std::endl; 
//  }
//  else {
//    std::cout << "(it1 != it2) => " << *it1 << " != " << *it2 << std::endl; 
//  }

//  it1 = r.begin();
//  for (; it1 != r.end(); ++it1) {
//    std::cout << *it1 << std::endl;
//  }


//  std::cout << std::endl;

//  std::copy(r.begin(), r.end(), std::ostream_iterator<std::string>(std::cout, ", "));

//  std::cout << std::endl << std::endl;



//  std::vector<std::string> vec;

//  std::copy(r.begin(), r.end(), back_inserter(vec));
//  std::cout << "copy size: " << vec.size() << std::endl;

//  std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(std::cout, ", "));

//  std::vector<std::string>::iterator it3 = vec.begin();
//  for (; it3 != vec.end(); ++it3) {
//    std::cout << *it3 << std::endl;
//  }

  return 0;
}

