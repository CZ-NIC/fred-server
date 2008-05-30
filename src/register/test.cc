/*
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <iostream>
#include <string>
#include "invoice.h"
#include "register.h"
#include "old_utils/dbsql.h"


int main()
{
  DB db;
  //  db.OpenDatabase("host=localhost dbname=ccreg user=postgres");
  db.OpenDatabase("host=curlew dbname=ccreg user=ccreg");
  std::auto_ptr<Register::Manager> m(Register::Manager::create(&db, true));
  Register::Registrar::Manager *rm = m->getRegistrarManager();
/*
  Register::Registrar::RegistrarList *rl2 = rm->getList();
  //  rl2->setIdFilter(2);
  rl2->reload();
  for (unsigned i=0; i<rl2->size(); i++)
    std::cout << "R:" << rl2->get(i)->getHandle() 
              << " C:" << rl2->get(i)->getCredit()
              << std::endl;
  */
  /*
  Register::Registrar::Registrar *r = rl2->get(0);
  r->setName("Name2");
  r->setURL("URL");
  r->save();
  /// -=-=-=-=-=-
  /// DOMAINS
  /// -=-=-=-=-=-
  //Register::Domain::List *dl = dm->getList();
  ///dl->reload();
  //for (unsigned i=0; i<dl->getCount(); i++)
///    std::cout << dl->get(i)->getFQDN() << std::endl;
  /// -=-=-=-=-=-
  /// ACTIONS
  /// -=-=-=-=-=-
  Register::Registrar::EPPActionList *eal = rm->getEPPActionList();
  eal->setRegistrarHandleFilter("REG");
  eal->setTimePeriodFilter(
    time_period(
      ptime(date(2006,1,1),time_duration(0,0,0)),
      ptime(date(2006,1,1),time_duration(24,0,0))
    )
  );
  eal->setReturnCodeFilter(1000);
  eal->setHandleFilter("d");
  eal->setTextTypeFilter("c");
  eal->setClTRIDFilter("b");
  eal->setSvTRIDFilter("a");      
  eal->reload();
  for (unsigned i=0; i<eal->size(); i++) {
    const Register::Registrar::EPPAction *a = eal->get(i);
    std::cout << "id:" << a->getType() 
              << " handle: " << a->getRegistrarHandle() << std::endl;
  }
  /// -=-=-=-=-=-
  /// REGISTARS
  /// -=-=-=-=-=-
  Register::Registrar::RegistrarList *rl = rm->getList();
  std::string filtr;
//  std::cout << "Registrar filter: ";
//  std::cin >> filtr;
  rl->setNameFilter(filtr);
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
  Register::Domain::List *dl = dm->getList();
  dl->setRegistrarHandleFilter("REG");
  dl->setRegistrantHandleFilter("CID");
  dl->setCrDateIntervalFilter(
    time_period(
      ptime(date(2006,1,1),time_duration(0,0,0)),
      ptime(date(2006,1,1),time_duration(24,0,0))
    )
  );
  dl->setFQDNFilter("dom");
  dl->setAdminHandleFilter("a");
  dl->setNSSetHandleFilter("n");
  dl->reload();
  */
  // im->archiveInvoices();
  while (1) {
    std::string input;
    std::cout << "Handle: ";
    std::cin >> input;
    Register::CheckHandleList chl;
    m->checkHandle(input,chl);
    if (chl.size() < 1) return -1;
    std::map<Register::CheckHandleClass,std::string> chcMap;
    chcMap[Register::CH_UNREGISTRABLE] = "CH_UNREGISTRABLE";
    chcMap[Register::CH_UNREGISTRABLE_LONG] = "CH_UNREGISTRABLE_LONG";
    chcMap[Register::CH_REGISTRED] = "CH_REGISTRED";
    chcMap[Register::CH_REGISTRED_PARENT] = "CH_REGISTRED_PARENT";
    chcMap[Register::CH_REGISTRED_CHILD] = "CH_REGISTRED_CHILD";
    chcMap[Register::CH_PROTECTED] = "CH_PROTECTED";
    chcMap[Register::CH_FREE] = "CH_FREE";
    
    std::map<Register::HandleType,std::string> htMap;
    htMap[Register::HT_ENUM_NUMBER] = "HT_ENUM_NUMBER";
    htMap[Register::HT_ENUM_DOMAIN] = "HT_ENUM_DOMAIN";
    htMap[Register::HT_DOMAIN] = "HT_DOMAIN";
    htMap[Register::HT_CONTACT] = "HT_CONTACT";
    htMap[Register::HT_NSSET] = "HT_NSSET";
    htMap[Register::HT_REGISTRAR] = "HT_REGISTRAR";
    htMap[Register::HT_OTHER] = "HT_OTHER";
      
    std::cout << "Result of checkHandle: \n";
    for (unsigned i=0; i< chl.size(); i++) {
      std::cout << i+1 << ".\n" 
                << " Type: " << htMap[chl[i].type] << std::endl
                << " Class: " << chcMap[chl[i].handleClass] << std::endl
                << " NewHandle: " << chl[i].newHandle << std::endl
                << " Conflict: " << chl[i].conflictHandle << std::endl;
    }
  }
}
