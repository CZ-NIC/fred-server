#include <iostream>
#include <fstream>
#include "admin/admin_impl.h"
#include "corba/nameservice.cc"
#include "corba/ccReg.hh"
#include "log/logger.h"

const char *corbaOpts[][2] = {
    {"nativeCharCodeSet", "UTF-8"},
    {NULL, NULL},
};

class CorbaClient {
    CORBA::ORB_var orb;
    std::auto_ptr<NameService> ns;
public:
    CorbaClient(int argc, char **argv, const std::string &nshost)
    {
        orb = CORBA::ORB_init(argc, argv, "", corbaOpts);
        ns.reset(new NameService(orb, nshost));
    }
    ~CorbaClient()
    {
        orb->destroy();
    }
    NameService *getNS()
    {
        return ns.get();
    }
};

int
main(int argc, char *argv[])
{
    std::stringstream nsAddr, database;
    //nsAddr << varMap["nshost"].as<std::string>() << ":" << varMap["nsport"].as<unsigned int>();
    nsAddr << "localhost:22346";
    database << "dbname=fred user=fred host=localhost port=22345";
    CorbaClient cc(argc, argv, nsAddr.str());
    CORBA::Object_var o = cc.getNS()->resolve("EPP");
    Conf cfg;
    ccReg_Admin_i *adminImpl = new ccReg_Admin_i(database.str(), cc.getNS(), cfg );
    ccReg::DomainDetails *ddets;
    ccReg::DomainDetail *ddet;
    ccReg::KeySetDetails *kdets;
    ccReg::KeySetDetail *kdet;

    ddet = adminImpl->getDomainByFQDN("pokus.cz");
    std::cout << ddet->fqdn << std::endl;
    std::cout << ddet->roid << std::endl;
    std::cout << ddet->nssetHandle << std::endl;
    std::cout << ddet->keysetHandle << std::endl;
    kdet = NULL;
    kdet = adminImpl->getKeySetByHandle(ddet->keysetHandle);
    if (kdet != NULL) {
        std::cout << "KeySet: " << std::endl;
        std::cout << kdet->handle << std::endl;
        for (int i = 0; i < kdet->dsrecords.length(); i++) {
            std::cout << kdet->dsrecords[i].keyTag << std::endl;
            std::cout << kdet->dsrecords[i].digest << std::endl;
        }
    }
    //kdet = adminImpl->getKeySetByHandle("KEY::001");
    // kdet = adminImpl->getKeySetById(11);
    // std::cout << kdet->id << std::endl;
    // std::cout << kdet->handle << std::endl;
    // std::cout << kdet->createDate << std::endl;
    // std::cout << kdet->authInfo << std::endl;
    //ddets = adminImpl->getDomainsByKeySetHandle("KEY::001");
    //ddets = adminImpl->getDomainsByKeySetId(28, 1000);
    //ddet = adminImpl->getDomainByKeySetHandle("KEY::001");

    // kdets = adminImpl->getKeySetsByContactId(6, 90);
// 
    // for (int i = 0; i < kdets->length(); i++)
        // std::cout << (*kdets)[i].id << " - " << (*kdets)[i].handle << std::endl;

    // for (int i = 0; i < ddets->length(); i++)
        // std::cout << (*ddets)[i].fqdn << std::endl;
    
    // std::cout << ddet->fqdn << std::endl;
    // std::cout << ddet->keysetHandle << std::endl;
    return 0;
}
