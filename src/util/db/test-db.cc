#include "util/log/logger.hh"
#include "util/log/context.hh"

#include "util/db/database.hh"
#include "util/types/datetime.hh"

#include "src/deprecated/libfred/db_settings.hh"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>

struct TestPooler
{
  TestPooler(int i) : id_(i) { }
  void operator()()
  {
    Logging::Context ctx(str(boost::format("threadid-%1%") % id_));

    try
    {
        Database::Manager::acquire().exec("SELECT * FROM object_registry");
    }
    catch (...)
    {
      LOGGER.error("no free connection");
    }
  }

  int id_;
};

int main()
{
  try
  {
    LOGGER.add_handler_of<Logging::Log::Device::console>(Logging::Log::EventImportance::trace);

    Database::Manager::init(new Database::Factory::Simple<Database::PSQLConnection>(
            std::string("host=localhost dbname=fred user=fred")));

    boost::thread_group tg;
    for (unsigned i = 0; i < 50; ++i)
    {
      TestPooler tp(i);
      tg.create_thread(tp);
    }
    tg.join_all();

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

    return EXIT_SUCCESS;
  }
  catch (const std::exception& e)
  {
    std::cerr << "error - exception occured! (" << e.what() << ")" << std::endl;
    return EXIT_SUCCESS;
  }
  catch (...)
  {
    std::cerr << "error - exception occured!" << std::endl;
    return EXIT_SUCCESS;
  }
}

