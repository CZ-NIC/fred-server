#include <memory>
#include <iostream>
#include <string>
#include "register.h"
#include "dbsql.h"

int main()
{
  DB db;
  db.OpenDatabase("dbname=ccreg user=ccreg");
  std::auto_ptr<Register::Manager> m(Register::Manager::create(&db));
  Register::Domain::Manager *dm = m->getDomainManager();
  std::string input;
  while (1) {
    std::cout << "Domain: ";
    std::cin >> input;
    Register::Domain::CheckAvailType ca = dm->checkAvail(input);
    std::cout << "Result: " << ca << std::endl;
  }
}
