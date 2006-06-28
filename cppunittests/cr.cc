#include <cppunit/extensions/HelperMacros.h>
#include "ccReg.hh"
#include "ccReg_epp.h"
#include "conf.h"
#include <iostream>

class EPPServerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( EPPServerTest );
  CPPUNIT_TEST(testLoginLogout);
  CPPUNIT_TEST(testLoginLogoutBadPasswd);
  CPPUNIT_TEST(testPasswdChange);
  CPPUNIT_TEST_SUITE_END();
  ccReg_EPP_i *epp;
 public:
  void setUp()
  {
    epp = new ccReg_EPP_i;
    Conf config;
    if (config.ReadConfigFile("../ccReg.conf"))
      epp->TestDatabaseConnect(config.GetDBconninfo());
  }

  void tearDown() 
  {
    epp->_remove_ref();
  }

  void loginSequence(const char *registrar, const char *passwd, 
		     const char *newpasswd, unsigned errCode)
  {
    CORBA::Long clientID;
    ccReg::Response* rClientLogin = NULL;
    rClientLogin = epp->ClientLogin(registrar,passwd,newpasswd, 
				    "cppunitklient", clientID , "" , 
				    ccReg::EN);
    CPPUNIT_ASSERT(rClientLogin != NULL);
    CPPUNIT_ASSERT_EQUAL(errCode,(unsigned)rClientLogin->errCode);
    // pokud jsem testoval na chybu koncime
    delete rClientLogin;
    if (errCode > 1999) return;
    CPPUNIT_ASSERT(clientID > 0);
    ccReg::Response* rClientLogout = NULL;
    rClientLogout = epp->ClientLogout(clientID,"cppunitklient");
    CPPUNIT_ASSERT(rClientLogout != NULL);
    CPPUNIT_ASSERT_EQUAL(1500U,(unsigned)rClientLogout->errCode);
    delete rClientLogout;
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
  // test na kontrolu neexistujicich kontaktu
  void testContact()
  {
    ccReg::Response* rClientLogin = NULL;
    rClientLogin = epp->ClientLogin("REG_LRR,passwd,newpasswd, 
				    "cppunitklient", clientID , "" , 
				    ccReg::EN);
    CPPUNIT_ASSERT(rClientLogin != NULL);
  }
  
};

CPPUNIT_TEST_SUITE_REGISTRATION( EPPServerTest );
