#include <iostream>
#include <vector>
#include <algorithm>

#include "log/logger.h"
#include "log/context.h"

#include "database.h"
#include "types/datetime.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

namespace Database {
  typedef Factory::Simple<PSQLConnection> ConnectionFactory;
  typedef Manager_<ConnectionFactory>     Manager;

  typedef Manager::connection_type        Connection;
  typedef Manager::transaction_type       Transaction;
  typedef Manager::result_type            Result;
  typedef Manager::sequence_type          Sequence;
  typedef Manager::row_type               Row;
}


struct TestPooler {
public:
  TestPooler(Database::Manager *p, int i) : pool_(p), id_(i) { }
  void operator()() {
    Logging::Context ctx(str(boost::format("threadid-%1%") % id_));

    try {
    std::auto_ptr<Database::Connection> c(pool_->acquire());
    c->exec("SELECT * FROM object_registry");
    }
    catch(...) {
      LOGGER(PACKAGE).error("no free connection");
    }
  }

  Database::Manager *pool_;
  int id_;
};


int main() {
  try {
    using namespace Database;

    Logging::Manager::instance_ref().get(PACKAGE).addHandler(Logging::Log::LT_CONSOLE); 
    Logging::Manager::instance_ref().get(PACKAGE).setLevel(Logging::Log::LL_TRACE);

    Database::Manager *pool = new Database::Manager(new ConnectionFactory("host=localhost dbname=fred user=fred"));

    boost::thread_group tg;
    for (unsigned i = 0; i < 50; ++i) {
      TestPooler tp(pool, i);
      tg.create_thread(tp);
    }
    tg.join_all();

    delete pool;

    //  try {
    //    Connection *c = pool.acquire();
    //    {
    //      Transaction t(*c);
    //      Result r = c->exec("SELECT roid, name, crdate FROM object_registry");
    //
    //      Connection *c2 = pool.acquire();
    //      Result r2 = c2->exec("SELECT * FROM domain");
    //      delete c2;
    //
    //      {
    //        Transaction t(*c);
    //        c->exec("SELECT * FROM files");
    //        c->exec("SELECT * FROM contact");
    //      }
    //
    //
    //      std::string str = r[0][0];
    //      std::cout << r[0][0] << "  " << str << std::endl;
    //    }
    //    delete c;
    //  }
    //  catch (Database::Exception& ex) {
    //    std::cout << ex.what() << std::endl;
    //  }
    //  catch (...) {
    //    std::cout << "ERROR" << std::endl;
    //  }

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
  catch (std::exception &_e) {
    std::cerr << "error - exception occured! (" << _e.what() << ")" << std::endl;
  }
  catch (...) {
    std::cerr << "error - exception occured!" << std::endl;
  }
}

