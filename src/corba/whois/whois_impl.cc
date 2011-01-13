/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>

#include <math.h>
#include <memory>
#include <iomanip>
#include <corba/Registry.hh>

#include "common.h"
#include "whois_impl.h"
#include "old_utils/log.h"
#include "old_utils/dbsql.h"
#include "fredlib/registry.h"
#include "fredlib/notify.h"
#include "corba/mailer_manager.h"
#include "fredlib/messages/messages_impl.h"
#include "fredlib/object_states.h"
#include "bank_payment.h"

#include "log/logger.h"
#include "log/context.h"

#include "random.h"

#include "corba/connection_releaser.h"

ccReg_Whois_i::ccReg_Whois_i(const std::string& _database
        , const std::string& _server_name
        , bool _registry_restricted_handles
        )
: m_connection_string(_database)
, server_name_(_server_name)
, registry_restricted_handles_(_registry_restricted_handles)
{
  Logging::Context ctx(server_name_);
}

ccReg_Whois_i::~ccReg_Whois_i()
{
  TRACE("[CALL] ccReg_Whois_i::~ccReg_Whois_i()");
}

void ccReg_Whois_i::fillRegistrar(ccReg::AdminRegistrar& creg
                            , Fred::Registrar::Registrar *reg)
{
  creg.id = reg->getId();
  creg.name = DUPSTRFUN(reg->getName);
  creg.handle = DUPSTRFUN(reg->getHandle);
  creg.url = DUPSTRFUN(reg->getURL);
  creg.organization = DUPSTRFUN(reg->getOrganization);
  creg.street1 = DUPSTRFUN(reg->getStreet1);
  creg.street2 = DUPSTRFUN(reg->getStreet2);
  creg.street3 = DUPSTRFUN(reg->getStreet3);
  creg.city = DUPSTRFUN(reg->getCity);
  creg.postalcode = DUPSTRFUN(reg->getPostalCode);
  creg.stateorprovince = DUPSTRFUN(reg->getProvince);
  creg.country = DUPSTRFUN(reg->getCountry);
  creg.telephone = DUPSTRFUN(reg->getTelephone);
  creg.fax = DUPSTRFUN(reg->getFax);
  creg.email = DUPSTRFUN(reg->getEmail);
  creg.credit = DUPSTRC(formatMoney(reg->getCredit()*100));
  creg.access.length(reg->getACLSize());
  for (unsigned i=0; i<reg->getACLSize(); i++) {
    creg.access[i].md5Cert = DUPSTRFUN(reg->getACL(i)->getCertificateMD5);
    creg.access[i].password = DUPSTRFUN(reg->getACL(i)->getPassword);
  }
  creg.hidden = reg->getHandle() == "REG-CZNIC" ? true : false;
}//ccReg_Whois_i::fillRegistrar


ccReg::AdminRegistrar* ccReg_Whois_i::getRegistrarByHandle(const char* handle)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  LOG( NOTICE_LOG, "getRegistarByHandle: handle -> %s", handle );
  if (!handle || !*handle) throw ccReg::Admin::ObjectNotFound();
  DB ldb;
  if (!ldb.OpenDatabase(m_connection_string.c_str())) {
    throw ccReg::Admin::SQL_ERROR();
  }
  try {
    std::auto_ptr<Fred::Manager> regm(
        Fred::Manager::create(&ldb, registry_restricted_handles_)
    );
    Fred::Registrar::Manager *rm = regm->getRegistrarManager();
    Fred::Registrar::RegistrarList::AutoPtr rl = rm->createList();
    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    std::auto_ptr<Database::Filters::Registrar> r ( new Database::Filters::RegistrarImpl(true));
    r->addHandle().setValue(handle);
    unionFilter->addFilter( r.release() );
    rl->reload(*unionFilter.get());

    if (rl->size() < 1) {
      ldb.Disconnect();
      throw ccReg::Admin::ObjectNotFound();
    }
    ccReg::AdminRegistrar* creg = new ccReg::AdminRegistrar;
    fillRegistrar(*creg,rl->get(0));
    ldb.Disconnect();
    return creg;
  }
  catch (Fred::SQL_ERROR) {
    ldb.Disconnect();
    throw ccReg::Admin::SQL_ERROR();
  }
}//ccReg_Whois_i::getRegistrarByHandle


