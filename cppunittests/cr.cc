#include <cppunit/extensions/HelperMacros.h>
#include "ccReg.hh"
#include "ccReg_epp.h"
#include "conf.h"
#include <iostream>
#include <memory>

#define CERT "AE:B3:5F:FA:38:80:DB:37:53:6A:3E:D4:55:E2:91:97"

class EPPServerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( EPPServerTest );
  CPPUNIT_TEST(testLoginLogout);
  CPPUNIT_TEST(testLoginLogoutBadPasswd);
  CPPUNIT_TEST(testPasswdChange);
  CPPUNIT_TEST(testCheckContact);
  CPPUNIT_TEST_SUITE_END();
  ccReg_EPP_i *epp;
 public:
  void setUp()
  {
    epp = new ccReg_EPP_i(NULL);
    Conf config;
    if (config.ReadConfigFile("../ccReg.conf"))
      epp->TestDatabaseConnect(config.GetDBconninfo());
  }

  void tearDown() 
  {
    epp->_remove_ref();
  }

  // funkce na obecne prihlaseni
  CORBA::Long loginRegistrar(const char *registrar, const char *passwd, 
			     const char *newpasswd, unsigned errCode)
  {
    CORBA::Long clientID;
    std::auto_ptr<ccReg::Response> rClientLogin(
      epp->ClientLogin(registrar,passwd,newpasswd, 
		       "cppunitklient", "xml", clientID , CERT , 
		       ccReg::EN)
    );
    CPPUNIT_ASSERT(rClientLogin.get() != NULL);
    CPPUNIT_ASSERT_EQUAL(errCode,(unsigned)rClientLogin->errCode);
    return clientID;
  }

  // funkce pro obecne odhlaseni 
  void logoutRegistrar(CORBA::Long clientID)
  {
    std::auto_ptr<ccReg::Response> rClientLogout(
      epp->ClientLogout(clientID,"cppunitklient", "xml")
    );
    CPPUNIT_ASSERT(rClientLogout.get() != NULL);
    CPPUNIT_ASSERT_EQUAL(1500U,(unsigned)rClientLogout->errCode);
  }

  // funkce na prihlaseni a odhlaseni
  void loginSequence(const char *registrar, const char *passwd, 
		     const char *newpasswd, unsigned errCode)
  {
    CORBA::Long clientID = loginRegistrar(registrar,passwd,newpasswd,errCode);
    if (errCode > 1999) return;
    CPPUNIT_ASSERT(clientID > 0);
    logoutRegistrar(clientID);
  }

  // test na prihlaseni se spravnym heslem
  void testLoginLogout()
  {
    loginSequence("REG-LRR","123456789","",1000);
  }

  // test na prihlaseni se spatnyn heslem
  void testLoginLogoutBadPasswd()
  {
    loginSequence("REG-LRR","","",2501);
  }

  // test na zmenu hesla
  void testPasswdChange()
  {
    loginSequence("REG-LRR","123456789","cppunit123456789",1000);
    loginSequence("REG-LRR","cppunit123456789","123456789",1000);
    loginSequence("REG-LRR","123456789","",1000);
  }

  CORBA::Long login()
  {
    return loginRegistrar("REG-LRR","123456789","",1000);
  }

  // test na kontrolu neexistujicich kontaktu
  void testCheckContact()
  {
    CORBA::Long clID = login();
    const char *handle1 = "NSSID:N01";
    const char *handle2 = "cppunitNEW2";
    ccReg::NSSet_var ns;
    std::auto_ptr<ccReg::Response> rCheckContact(
      epp->NSSetInfo(handle1,ns,clID,"cppunitklient","xml")
    );
    
    logoutRegistrar(clID);
  }
  
};

CPPUNIT_TEST_SUITE_REGISTRATION( EPPServerTest );
