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
  for (unsigned i=0; i<rl->size(); i++) {
    const Register::Registrar::Registrar *r = rl->get(i);
    std::cout << "id:" << r->getId() 
              << " handle: " << r->getHandle() << std::endl;
    for (unsigned j=0; j<r->getACLSize(); j++)
      std::cout << "  cert:" << r->getACL(j)->getCertificateMD5()
		<< " pass:" << r->getACL(j)->getPassword()
		<< std::endl;
  }
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
