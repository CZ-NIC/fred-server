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
  /// -=-=-=-=-=-
  /// DOMAINS
  /// -=-=-=-=-=-
  Register::Domain::List *dl = dm->getList();
  dl->reload();
  for (unsigned i=0; i<dl->getCount(); i++)
    std::cout << dl->get(i)->getFQDN() << std::endl;
  /// -=-=-=-=-=-
  /// ACTIONS
  /// -=-=-=-=-=-
  /*
  Register::Registrar::EPPActionList *eal = rm->getEPPActionList();
  eal->reload();
  for (unsigned i=0; i<eal->size(); i++) {
    const Register::Registrar::EPPAction *a = eal->get(i);
    std::cout << "id:" << a->getType() 
              << " handle: " << a->getRegistrarHandle() << std::endl;
  }
  */
  /// -=-=-=-=-=-
  /// REGISTARS
  /// -=-=-=-=-=-
  Register::Registrar::RegistrarList *rl = rm->getList();
  std::string filtr;
  std::cout << "Registrar filter: ";
  std::cin >> filtr;
  rl->setFulltextFilter(filtr);
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
  /// -=-=-=-=-=-
  /// DOMAINS
  /// -=-=-=-=-=-
  std::string input;
  std::cout << "Domain: ";
  std::cin >> input;
  Register::CheckHandle ch;
  m->checkHandle(input,ch);
  std::cout << "Result of checkHandle: " << ch.handleClass 
	    << " NewHandle: " << ch.newHandle << std::endl;
  Register::Domain::CheckAvailType ca = dm->checkAvail(input);
  std::cout << "Result of checkAvail: " << ca << std::endl;

}
