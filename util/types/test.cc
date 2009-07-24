#include <iostream>
#include "id.h"
#include "request_types.h"
#include "money.h"
#include "convert_str_pod.h"
#include "convert_sql_boost_datetime.h"
#include "stringify.h"
#include "sqlize.h"
#include <limits>

#define COUT(v)  std::cout << #v << " = " << v << std::endl;


int main() {

  int maxint = std::numeric_limits<int>::max();
  COUT(maxint);

  int i = unstringify<int>(stringify(maxint + 1));
  COUT(i);

  Database::ID id;
  id = unstringify<Database::ID>("3423");
  COUT(id);

  Database::Money m;
  COUT("22.3")
  m = unstringify<Database::Money>("22.3");
  COUT(m);

  COUT("221.23")
  m = unstringify<Database::Money>("221.23");
  COUT(m);

  COUT("100")
  m = unstringify<Database::Money>("100");
  COUT(m);

  COUT("221.2343")
  m = unstringify<Database::Money>("221.2343");
  COUT(m);


  try {
    Database::Money m;
    COUT("221.29803.43")
    m = unstringify<Database::Money>("221.29803.43");
    COUT(m);
  }
  catch(...) {
    COUT("error");
  }

  try {
    Database::Money m;
    COUT("221.2.9803.43")
    m = unstringify<Database::Money>("221.2.9803.43");
    COUT(m);
  }
  catch(...) {
    COUT("error");
  }

  COUT(12322);
  m = 12322;
  COUT(stringify(m));


  COUT("----");


  COUT(sqlize(ptime(second_clock::local_time())));
  COUT(sqlize(ptime(microsec_clock::local_time())));


  return 0;
}
