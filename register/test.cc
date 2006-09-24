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
  Register::Registrar::Manager *rm = m->getRegistrarManager();
  Register::Registrar::RegistrarList *rl = rm->getList();
  rl->reload();
  for (unsigned i=0; i<rl->size(); i++)
    std::cout << "id:" << rl->get(i)->getId() 
              << " handle: " << rl->get(i)->getHandle() << std::endl;
  std::string input;
  while (1) {
    std::cout << "Domain: ";
    std::cin >> input;
    Register::CheckHandle ch;
    m->checkHandle(input,ch);
    std::cout << "Result of checkHandle: " << ch.handleClass 
              << " NewHandle: " << ch.newHandle << std::endl;
    Register::Domain::CheckAvailType ca = dm->checkAvail(input);
    std::cout << "Result of checkAvail: " << ca << std::endl;
  }
}
