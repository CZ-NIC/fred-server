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

#include <memory>
#include <iomanip>
#include <stdexcept>
#include <corba/Whois.hh>

#include "common.h"
#include "whois_impl.h"
#include "old_utils/log.h"
#include "old_utils/dbsql.h"
#include "fredlib/registry.h"

#include "log/logger.h"
#include "log/context.h"

#include "corba/connection_releaser.h"


ccReg_Whois_i::ccReg_Whois_i(const std::string& _database
        , const std::string& _server_name
        , bool _registry_restricted_handles
        )
: m_connection_string(_database)
, server_name_(_server_name)
, registry_restricted_handles_(_registry_restricted_handles)
, db_disconnect_guard_ ( connect_DB(m_connection_string
    , std::runtime_error(
        std::string("ccReg_Whois_i::ccReg_Whois_i cannot connect to DATABASE ")
        + m_connection_string)))
, registry_manager_(Fred::Manager::create(db_disconnect_guard_
    , registry_restricted_handles_))
{
  Logging::Context ctx(server_name_);
  registry_manager_->initStates();
}

ccReg_Whois_i::~ccReg_Whois_i()
{
  TRACE("[CALL] ccReg_Whois_i::~ccReg_Whois_i()");
}

void ccReg_Whois_i::fillRegistrar(ccReg::WhoisRegistrar& creg
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
  creg.credit = DUPSTRC(formatMoney(reg->getCredit()));
  creg.access.length(reg->getACLSize());
  for (unsigned i=0; i<reg->getACLSize(); i++) {
    creg.access[i].md5Cert = DUPSTRFUN(reg->getACL(i)->getCertificateMD5);
    creg.access[i].password = DUPSTRFUN(reg->getACL(i)->getPassword);
  }
  creg.hidden = reg->getHandle() == "REG-CZNIC" ? true : false;
}//ccReg_Whois_i::fillRegistrar

void ccReg_Whois_i::fillContact(ccReg::ContactDetail* cc,
                                Fred::Contact::Contact* c)
{
  cc->id = c->getId();
  cc->handle = DUPSTRFUN(c->getHandle);
  cc->roid = DUPSTRFUN(c->getROID);
  cc->registrarHandle = DUPSTRFUN(c->getRegistrarHandle);
  cc->transferDate = DUPSTRDATE(c->getTransferDate);
  cc->updateDate = DUPSTRDATE(c->getUpdateDate);
  cc->createDate = DUPSTRDATE(c->getCreateDate);
  cc->createRegistrarHandle = DUPSTRFUN(c->getCreateRegistrarHandle);
  cc->updateRegistrarHandle = DUPSTRFUN(c->getUpdateRegistrarHandle);
  cc->authInfo = DUPSTRFUN(c->getAuthPw);
  cc->name = DUPSTRFUN(c->getName);
  cc->organization = DUPSTRFUN(c->getOrganization);
  cc->street1 = DUPSTRFUN(c->getStreet1);
  cc->street2 = DUPSTRFUN(c->getStreet2);
  cc->street3 = DUPSTRFUN(c->getStreet3);
  cc->province = DUPSTRFUN(c->getProvince);
  cc->postalcode = DUPSTRFUN(c->getPostalCode);
  cc->city = DUPSTRFUN(c->getCity);
  cc->country = DUPSTRFUN(c->getCountry);
  cc->telephone = DUPSTRFUN(c->getTelephone);
  cc->fax = DUPSTRFUN(c->getFax);
  cc->email = DUPSTRFUN(c->getEmail);
  cc->notifyEmail = DUPSTRFUN(c->getNotifyEmail);
  cc->ssn = DUPSTRFUN(c->getSSN);
  cc->ssnType = DUPSTRFUN(c->getSSNType);
  cc->vat = DUPSTRFUN(c->getVAT);
  cc->discloseName = c->getDiscloseName();
  cc->discloseOrganization = c->getDiscloseOrganization();
  cc->discloseAddress = c->getDiscloseAddr();
  cc->discloseEmail = c->getDiscloseEmail();
  cc->discloseTelephone = c->getDiscloseTelephone();
  cc->discloseFax = c->getDiscloseFax();
  cc->discloseIdent = c->getDiscloseIdent();
  cc->discloseVat = c->getDiscloseVat();
  cc->discloseNotifyEmail = c->getDiscloseNotifyEmail();
  std::vector<unsigned> slist;
  for (unsigned i=0; i<c->getStatusCount(); i++) {
    if (registry_manager_->getStatusDesc(
        c->getStatusByIdx(i)->getStatusId()
    )->getExternal())
      slist.push_back(c->getStatusByIdx(i)->getStatusId());
  }
  cc->statusList.length(slist.size());
  for (unsigned i=0; i<slist.size(); i++)
    cc->statusList[i] = slist[i];
}//ccReg_Whois_i::fillContact

void ccReg_Whois_i::fillNSSet(ccReg::NSSetDetail* cn, Fred::NSSet::NSSet* n)
{
  cn->id = n->getId();
  cn->handle = DUPSTRFUN(n->getHandle);
  cn->roid = DUPSTRFUN(n->getROID);
  cn->registrarHandle = DUPSTRFUN(n->getRegistrarHandle);
  cn->transferDate = DUPSTRDATE(n->getTransferDate);
  cn->updateDate = DUPSTRDATE(n->getUpdateDate);
  cn->createDate = DUPSTRDATE(n->getCreateDate);
  cn->createRegistrarHandle = DUPSTRFUN(n->getCreateRegistrarHandle);
  cn->updateRegistrarHandle = DUPSTRFUN(n->getUpdateRegistrarHandle);
  cn->authInfo = DUPSTRFUN(n->getAuthPw);
  cn->admins.length(n->getAdminCount());
  try {
    for (unsigned i=0; i<n->getAdminCount(); i++)
    cn->admins[i] = DUPSTRC(n->getAdminByIdx(i));
  }
  catch (Fred::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
  cn->hosts.length(n->getHostCount());
  for (unsigned i=0; i<n->getHostCount(); i++) {
    cn->hosts[i].fqdn = DUPSTRFUN(n->getHostByIdx(i)->getNameIDN);
    cn->hosts[i].inet.length(n->getHostByIdx(i)->getAddrCount());
    for (unsigned j=0; j<n->getHostByIdx(i)->getAddrCount(); j++)
      cn->hosts[i].inet[j] = DUPSTRC(n->getHostByIdx(i)->getAddrByIdx(j));
  }
  std::vector<unsigned> slist;
  for (unsigned i=0; i<n->getStatusCount(); i++) {
    if (registry_manager_->getStatusDesc(
        n->getStatusByIdx(i)->getStatusId()
    )->getExternal())
      slist.push_back(n->getStatusByIdx(i)->getStatusId());
  }
  cn->statusList.length(slist.size());
  for (unsigned i=0; i<slist.size(); i++)
    cn->statusList[i] = slist[i];

  cn->reportLevel = n->getCheckLevel();
}//ccReg_Whois_i::fillNSSet

void ccReg_Whois_i::fillKeySet(ccReg::KeySetDetail *ck
        , Fred::KeySet::KeySet *k)
{
    ck->id = k->getId();
    ck->handle          = DUPSTRFUN(k->getHandle);
    ck->roid            = DUPSTRFUN(k->getROID);
    ck->registrarHandle = DUPSTRFUN(k->getRegistrarHandle);
    ck->transferDate    = DUPSTRDATE(k->getTransferDate);
    ck->updateDate      = DUPSTRDATE(k->getUpdateDate);
    ck->createDate      = DUPSTRDATE(k->getCreateDate);
    ck->createRegistrarHandle = DUPSTRFUN(k->getCreateRegistrarHandle);
    ck->updateRegistrarHandle = DUPSTRFUN(k->getUpdateRegistrarHandle);
    ck->authInfo        = DUPSTRFUN(k->getAuthPw);
    ck->admins.length(k->getAdminCount());
    try {
        for (unsigned int i = 0; i < k->getAdminCount(); i++)
            ck->admins[i] = DUPSTRC(k->getAdminByIdx(i));
    }
    catch (Fred::NOT_FOUND) {
        // TODO implement error handling
    }

    ck->dsrecords.length(k->getDSRecordCount());
    for (unsigned int i = 0; i < k->getDSRecordCount(); i++) {
        ck->dsrecords[i].keyTag = k->getDSRecordByIdx(i)->getKeyTag();
        ck->dsrecords[i].alg = k->getDSRecordByIdx(i)->getAlg();
        ck->dsrecords[i].digestType = k->getDSRecordByIdx(i)->getDigestType();
        ck->dsrecords[i].digest = DUPSTRC(k->getDSRecordByIdx(i)->getDigest());
        ck->dsrecords[i].maxSigLife = k->getDSRecordByIdx(i)->getMaxSigLife();
    }

    ck->dnskeys.length(k->getDNSKeyCount());
    for (unsigned int i = 0; i < k->getDNSKeyCount(); i++) {
        ck->dnskeys[i].flags = k->getDNSKeyByIdx(i)->getFlags();
        ck->dnskeys[i].protocol = k->getDNSKeyByIdx(i)->getProtocol();
        ck->dnskeys[i].alg = k->getDNSKeyByIdx(i)->getAlg();
        ck->dnskeys[i].key = DUPSTRFUN(k->getDNSKeyByIdx(i)->getKey);
    }

    std::vector<unsigned int> slist;
    for (unsigned int i = 0; i < k->getStatusCount(); i++)
        if (registry_manager_->getStatusDesc(
                    k->getStatusByIdx(i)->getStatusId())->getExternal())
            slist.push_back(k->getStatusByIdx(i)->getStatusId());

    ck->statusList.length(slist.size());
    for (unsigned int i = 0; i < slist.size(); i++)
        ck->statusList[i] = slist[i];
}//ccReg_Whois_i::fillKeySet

void ccReg_Whois_i::fillDomain(ccReg::DomainDetail* cd,
                               Fred::Domain::Domain* d)
{
  cd->id = d->getId();
  cd->fqdn = DUPSTRFUN(d->getFQDNIDN);
  cd->roid = DUPSTRFUN(d->getROID);
  cd->registrarHandle = DUPSTRFUN(d->getRegistrarHandle);
  cd->transferDate = DUPSTRDATE(d->getTransferDate);
  cd->updateDate = DUPSTRDATE(d->getUpdateDate);
  cd->createDate = DUPSTRDATE(d->getCreateDate);
  cd->createRegistrarHandle = DUPSTRFUN(d->getCreateRegistrarHandle);
  cd->updateRegistrarHandle = DUPSTRFUN(d->getUpdateRegistrarHandle);
  cd->authInfo = DUPSTRFUN(d->getAuthPw);
  cd->registrantHandle = DUPSTRFUN(d->getRegistrantHandle);
  cd->expirationDate = DUPSTRDATED(d->getExpirationDate);
  cd->valExDate = DUPSTRDATED(d->getValExDate);
  cd->nssetHandle = DUPSTRFUN(d->getNSSetHandle);
  cd->keysetHandle = DUPSTRFUN(d->getKeySetHandle);
  cd->admins.length(d->getAdminCount(1));
  cd->temps.length(d->getAdminCount(2));
  std::vector<unsigned> slist;
  for (unsigned i=0; i<d->getStatusCount(); i++) {
    if (registry_manager_->getStatusDesc(
        d->getStatusByIdx(i)->getStatusId()
    )->getExternal())
      slist.push_back(d->getStatusByIdx(i)->getStatusId());
  }
  cd->statusList.length(slist.size());
  for (unsigned i=0; i<slist.size(); i++)
    cd->statusList[i] = slist[i];
  try {
    for (unsigned i=0; i<d->getAdminCount(1); i++)
    cd->admins[i] = DUPSTRC(d->getAdminHandleByIdx(i,1));
    for (unsigned i=0; i<d->getAdminCount(2); i++)
    cd->temps[i] = DUPSTRC(d->getAdminHandleByIdx(i,2));
  }
  catch (Fred::NOT_FOUND) {
    /// some implementation error - index is out of bound - WHAT TO DO?
  }
}//ccReg_Whois_i::fillDomain


ccReg::WhoisRegistrar* ccReg_Whois_i::getRegistrarByHandle(const char* handle)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( NOTICE_LOG, "getRegistarByHandle: handle -> %s", handle );

        if (!handle || !*handle) throw ccReg::Whois::ObjectNotFound();

        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                , ccReg::Whois::InternalServerError());

        try {
        std::auto_ptr<Fred::Manager> regm(
            Fred::Manager::create(ldb_disconnect_guard, registry_restricted_handles_)
        );
        Fred::Registrar::Manager *rm = regm->getRegistrarManager();
        Fred::Registrar::RegistrarList::AutoPtr rl = rm->createList();
        Database::Filters::UnionPtr unionFilter
            = Database::Filters::CreateClearedUnionPtr();
        std::auto_ptr<Database::Filters::Registrar>
            r ( new Database::Filters::RegistrarImpl(true));
        r->addHandle().setValue(handle);
        unionFilter->addFilter( r.release() );
        rl->reload(*unionFilter.get());

        if (rl->size() < 1) {
          throw ccReg::Whois::ObjectNotFound();
        }
        ccReg::WhoisRegistrar* creg = new ccReg::WhoisRegistrar;
        fillRegistrar(*creg,rl->get(0));
        return creg;
        }
        catch (Fred::SQL_ERROR) {
        throw ccReg::Whois::InternalServerError();
        }
    }//try
    catch (const ccReg::Whois::ObjectNotFound& )
    {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( NOTICE_LOG
                  , "getRegistarByHandle: ccReg::Whois::ObjectNotFound");
      throw;
    }
    catch (const ccReg::Whois::InternalServerError& )
    {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( ERROR_LOG
                  , "getRegistarByHandle: ccReg::Whois::InternalServerError");
      throw;
    }
    catch (const std::exception& ex)
    {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( ERROR_LOG
                  , "getRegistarByHandle: std::exception %s", ex.what());
      throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( ERROR_LOG
                  , "getRegistarByHandle: unknown exception ");
      throw ccReg::Whois::InternalServerError();
    }
}//ccReg_Whois_i::getRegistrarByHandle


ccReg::WhoisRegistrarList* ccReg_Whois_i::getRegistrarsByZone(const char *zone)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;

  try
  {
    DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                                                  , ccReg::Admin::SQL_ERROR());
    std::auto_ptr<Fred::Manager> regm(
        Fred::Manager::create(ldb_disconnect_guard,registry_restricted_handles_)
    );
    Fred::Registrar::Manager *rm = regm->getRegistrarManager();
    Fred::Registrar::RegistrarList::AutoPtr rl = rm->createList();

    Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
    std::auto_ptr<Database::Filters::Registrar> r ( new Database::Filters::RegistrarImpl(true));
    r->addZoneFqdn().setValue(zone);
    unionFilter->addFilter( r.release() );
    rl->reload(*unionFilter.get());

    Logging::Manager::instance_ref()
        .get(server_name_.c_str())
        .message( NOTICE_LOG
                , "getRegistrarsByZone: num -> %d", rl->size());

    ccReg::WhoisRegistrarList* reglist = new ccReg::WhoisRegistrarList;
    reglist->length(rl->size());
    for (unsigned i=0; i<rl->size(); i++)
    fillRegistrar((*reglist)[i],rl->get(i));
    return reglist;
  }//try
  catch (Fred::SQL_ERROR)
  {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( ERROR_LOG
                  , "getRegistrarsByZone: Fred::SQL_ERROR exception ");
      throw ccReg::Whois::InternalServerError();
  }
  catch (const ccReg::Whois::InternalServerError& )
  {
    Logging::Manager::instance_ref()
        .get(server_name_.c_str())
        .message( ERROR_LOG
                , "getRegistrarsByZone: ccReg::Whois::InternalServerError");
    throw;
  }
  catch (const std::exception& ex)
  {
    Logging::Manager::instance_ref()
        .get(server_name_.c_str())
        .message( ERROR_LOG
                , "getRegistrarsByZone: std::exception %s", ex.what());
    throw ccReg::Whois::InternalServerError();
  }
  catch (...)
  {
    Logging::Manager::instance_ref()
        .get(server_name_.c_str())
        .message( ERROR_LOG
                , "getRegistrarsByZone: unknown exception ");
    throw ccReg::Whois::InternalServerError();
  }
}//ccReg_Whois_i::getRegistrarsByZone


ccReg::ContactDetail* ccReg_Whois_i::getContactByHandle(const char* handle)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        Logging::Manager::instance_ref()
        .get(server_name_.c_str())
        .trace(boost::format(
                "[CALL] ccReg_Whois_i::getContactByHandle('%1%')") % handle);

        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                        , ccReg::Whois::InternalServerError());

        if (!handle || !*handle)
            throw ccReg::Whois::ObjectNotFound();

        std::auto_ptr<Fred::Manager> r(Fred::Manager::create(ldb_disconnect_guard
                , registry_restricted_handles_));
        Fred::Contact::Manager *cr = r->getContactManager();
        std::auto_ptr<Fred::Contact::List> cl(cr->createList());
        cl->setWildcardExpansion(false);
        cl->setHandleFilter(handle);
        cl->reload();
        if (cl->getCount() != 1) {
        throw ccReg::Whois::ObjectNotFound();
        }
        ccReg::ContactDetail* cc = new ccReg::ContactDetail;
        fillContact(cc, cl->getContact(0));
        return cc;
    }//try
    catch (const ccReg::Whois::ObjectNotFound& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( NOTICE_LOG
                , "getContactByHandle: ccReg::Whois::ObjectNotFound");
        throw;
    }
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getContactByHandle: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getContactByHandle: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getContactByHandle: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }

}//ccReg_Whois_i::getContactByHandle

ccReg::NSSetDetail* ccReg_Whois_i::getNSSetByHandle(const char* handle)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        Logging::Manager::instance_ref()
                .get(server_name_.c_str())
                .trace(boost::format(
                "[CALL] ccReg_Whois_i::getNSSetByHandle('%1%')") % handle);

        if (!handle || !*handle)
        throw ccReg::Whois::ObjectNotFound();

        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                                , ccReg::Whois::InternalServerError());

        std::auto_ptr<Fred::Manager>
            r(Fred::Manager::create(ldb_disconnect_guard, registry_restricted_handles_));
        Fred::NSSet::Manager *nr = r->getNSSetManager();
        std::auto_ptr<Fred::NSSet::List> nl(nr->createList());
        nl->setWildcardExpansion(false);
        nl->setHandleFilter(handle);
        nl->reload();
        if (nl->getCount() != 1) {
        throw ccReg::Whois::ObjectNotFound();
        }
        ccReg::NSSetDetail* cn = new ccReg::NSSetDetail;
        fillNSSet(cn, nl->getNSSet(0));
        return cn;
    }//try
    catch (const ccReg::Whois::ObjectNotFound& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( NOTICE_LOG
                , "getNSSetByHandle: ccReg::Whois::ObjectNotFound");
        throw;
    }
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getNSSetByHandle: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getNSSetByHandle: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getNSSetByHandle: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }
}//ccReg_Whois_i::getNSSetByHandle

ccReg::KeySetDetail * ccReg_Whois_i::getKeySetByHandle(const char *handle)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        Logging::Manager::instance_ref()
                .get(server_name_.c_str())
                .trace(boost::format(
                "[CALL] ccReg_Whois_i::getKeySetByHandle('%1%')") % handle);

        if (!handle || !*handle)
            throw ccReg::Whois::ObjectNotFound();

        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                                , ccReg::Whois::InternalServerError());

        std::auto_ptr<Fred::Manager> r(Fred::Manager::create(ldb_disconnect_guard
            ,registry_restricted_handles_));
        Fred::KeySet::Manager *kr = r->getKeySetManager();
        std::auto_ptr<Fred::KeySet::List> kl(kr->createList());
        kl->setWildcardExpansion(false);
        kl->setHandleFilter(handle);
        kl->reload();

        if (kl->getCount() != 1) {
            throw ccReg::Whois::ObjectNotFound();
        }

        ccReg::KeySetDetail *ck = new ccReg::KeySetDetail;
        fillKeySet(ck, kl->getKeySet(0));
        return ck;
    }//try
    catch (const ccReg::Whois::ObjectNotFound& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( NOTICE_LOG
                , "getKeySetByHandle: ccReg::Whois::ObjectNotFound");
        throw;
    }
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getKeySetByHandle: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getKeySetByHandle: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getKeySetByHandle: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }

}//ccReg_Whois_i::getKeySetByHandle



ccReg::DomainDetails* ccReg_Whois_i::getDomainsByInverseKey(const char* key,
                                                            ccReg::DomainInvKeyType type,
                                                            CORBA::Long limit)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        Logging::Manager::instance_ref()
                .get(server_name_.c_str())
                .trace(boost::format("[CALL] ccReg_Whois_i::getDomainsByInverseKey('%1%', %2%, %3%)")
          % key % type % limit);

        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                                , ccReg::Whois::InternalServerError());

        std::auto_ptr<Fred::Manager> r(Fred::Manager::create(ldb_disconnect_guard
                , registry_restricted_handles_));
        Fred::Domain::Manager *dm = r->getDomainManager();
        std::auto_ptr<Fred::Domain::List> dl(dm->createList());
        switch (type) {
        case ccReg::DIKT_REGISTRANT:
          dl->setRegistrantHandleFilter(key);
          break;
        case ccReg::DIKT_ADMIN:
          dl->setAdminHandleFilter(key);
          break;
        case ccReg::DIKT_TEMP:
          dl->setTempHandleFilter(key);
          break;
        case ccReg::DIKT_NSSET:
          dl->setNSSetHandleFilter(key);
          break;
        case ccReg::DIKT_KEYSET:
          dl->setKeySetHandleFilter(key);
          break;
        }
        dl->setLimit(limit);
        dl->reload();
        ccReg::DomainDetails_var dlist = new ccReg::DomainDetails;
        dlist->length(0);
        for (unsigned i=0; i<dl->getCount(); i++) {
            Fred::Domain::Domain *d = dl->getDomain(i);
            if (!d) {
                throw ccReg::Whois::InternalServerError();
                /* or continue? */
            }

            if (dm->isDeletePending(
                        r->getZoneManager()->encodeIDN(d->getFQDN())) == false)
            {
                unsigned int l = dlist->length();
                dlist->length(l + 1);
                fillDomain(&dlist[l], d);
            }
            else
            {
                LOGGER(PACKAGE).debug(boost::format(
                        "%1% removed from list - delete pending status")
                        % d->getFQDN());
            }
        }
        return dlist._retn();
    }//try
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getDomainsByInverseKey: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getDomainsByInverseKey: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getDomainsByInverseKey: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }
}//ccReg_Whois_i::getDomainsByInverseKey

ccReg::NSSetDetails* ccReg_Whois_i::getNSSetsByInverseKey(const char* key,
                                                          ccReg::NSSetInvKeyType type,
                                                          CORBA::Long limit)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                                , ccReg::Whois::InternalServerError());

        std::auto_ptr<Fred::Manager> r(
                Fred::Manager::create(ldb_disconnect_guard
                        , registry_restricted_handles_));
        Fred::Zone::Manager *zm = r->getZoneManager();
        Fred::NSSet::Manager *nm = r->getNSSetManager();
        std::auto_ptr<Fred::NSSet::List> nl(nm->createList());
        switch (type) {
        case ccReg::NIKT_NS : nl->setHostNameFilter(zm->encodeIDN(key)); break;
        case ccReg::NIKT_TECH : nl->setAdminFilter(key); break;
        }
        nl->setLimit(limit);
        nl->reload();
        ccReg::NSSetDetails_var nlist = new ccReg::NSSetDetails;
        nlist->length(nl->getCount());
        for (unsigned i=0; i<nl->getCount(); i++)
        fillNSSet(&nlist[i], nl->getNSSet(i));
        return nlist._retn();
    }//try
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getNSSetsByInverseKey: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getNSSetsByInverseKey: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getNSSetsByInverseKey: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }
}//ccReg_Whois_i::getNSSetsByInverseKey

ccReg::KeySetDetails* ccReg_Whois_i::getKeySetsByInverseKey(
        const char *key,
        ccReg::KeySetInvKeyType type,
        CORBA::Long limit)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                                , ccReg::Whois::InternalServerError());

        std::auto_ptr<Fred::Manager> r(
                Fred::Manager::create(ldb_disconnect_guard
                        , registry_restricted_handles_));
        Fred::KeySet::Manager *km = r->getKeySetManager();
        std::auto_ptr<Fred::KeySet::List> kl(km->createList());
        switch (type) {
            case ccReg::KIKT_TECH:
                kl->setAdminFilter(key);
                break;
        }
        kl->setLimit(limit);
        kl->reload();
        ccReg::KeySetDetails_var klist = new ccReg::KeySetDetails;
        klist->length(kl->getCount());
        for (unsigned int i = 0; i < kl->getCount(); i++)
            fillKeySet(&klist[i], kl->getKeySet(i));
        return klist._retn();
    }//try
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getKeySetsByInverseKey: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getKeySetsByInverseKey: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getKeySetsByInverseKey: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }
}//ccReg_Whois_i::getKeySetsByInverseKey

ccReg::DomainDetail* ccReg_Whois_i::getDomainByFQDN(const char* fqdn)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        Logging::Manager::instance_ref()
                .get(server_name_.c_str())
                .trace(boost::format(
                "[CALL] ccReg_Whois_i::getDomainByFQDN('%1%')") % fqdn);

        if (!fqdn || !*fqdn)
        throw ccReg::Whois::ObjectNotFound();

        DBSharedPtr ldb_disconnect_guard = connect_DB(m_connection_string
                                , ccReg::Whois::InternalServerError());

        std::auto_ptr<Fred::Manager>
            r(Fred::Manager::create(ldb_disconnect_guard
                    , registry_restricted_handles_));
        Fred::Domain::Manager *dm = r->getDomainManager();

        if (dm->isDeletePending(r->getZoneManager()->encodeIDN(fqdn)))
        {
            const Fred::StatusDesc *dc = registry_manager_->getStatusDesc("deleteCandidate");
            if (!dc) {
                throw ccReg::Whois::InternalServerError();
            }

            ccReg::DomainDetail *cd = new ccReg::DomainDetail();
            cd->id = 0;
            cd->fqdn = fqdn;
            cd->statusList.length(1);
            cd->statusList[0] = dc->getId();
            return cd;
        }
        else
        {
            std::auto_ptr<Fred::Domain::List> dl(dm->createList());
            dl->setWildcardExpansion(false);
            dl->setFQDNFilter(r->getZoneManager()->encodeIDN(fqdn));
            dl->reload();

            if (dl->getCount() != 1) {
                throw ccReg::Whois::ObjectNotFound();
            }
            else {
                ccReg::DomainDetail* cd = new ccReg::DomainDetail;
                fillDomain(cd, dl->getDomain(0));
                return cd;
            }
        }
    }//try
    catch (const ccReg::Whois::ObjectNotFound& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( NOTICE_LOG
                , "getDomainByFQDN: ccReg::Whois::ObjectNotFound");
        throw;
    }
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getDomainByFQDN: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getDomainByFQDN: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getDomainByFQDN: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }

}//ccReg_Whois_i::getDomainByFQDN

Registry::ObjectStatusDescSeq* ccReg_Whois_i::getDomainStatusDescList(const char *lang)
{
    Logging::Context ctx(server_name_);
    ConnectionReleaser releaser;

    try
    {
        Registry::ObjectStatusDescSeq* o = new Registry::ObjectStatusDescSeq;
        for (unsigned i=0; i<registry_manager_->getStatusDescCount(); i++) {
        const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
        if (sd->getExternal() && sd->isForType(3)) {
          o->length(o->length()+1);
          try {
            (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
          } catch (...) {
            // unknown language
            (*o)[o->length()-1].name = CORBA::string_dup("");
          }
          (*o)[o->length()-1].id    = sd->getId();
          (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
        }
        }
        return o;
    }//try
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getDomainStatusDescList: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getDomainStatusDescList: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getDomainStatusDescList: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }

}//ccReg_Whois_i::getDomainStatusDescList

Registry::ObjectStatusDescSeq* ccReg_Whois_i::getContactStatusDescList(const char *lang)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;
    try
    {
      Registry::ObjectStatusDescSeq* o = new Registry::ObjectStatusDescSeq;
      for (unsigned i=0; i<registry_manager_->getStatusDescCount(); i++) {
        const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
        if (sd->getExternal() && sd->isForType(1)) {
          o->length(o->length()+1);
          try {
            (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
          } catch (...) {
            // unknown language
            (*o)[o->length()-1].name = CORBA::string_dup("");
          }
          (*o)[o->length()-1].id    = sd->getId();
          (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
        }
      }
      return o;
    }//try
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getContactStatusDescList: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getContactStatusDescList: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getContactStatusDescList: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }
}//ccReg_Whois_i::getContactStatusDescList

Registry::ObjectStatusDescSeq* ccReg_Whois_i::getNSSetStatusDescList(const char *lang)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;
  try
  {
      Registry::ObjectStatusDescSeq* o = new Registry::ObjectStatusDescSeq;
      for (unsigned i=0; i<registry_manager_->getStatusDescCount(); i++) {
        const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
        if (sd->getExternal() && sd->isForType(2)) {
          o->length(o->length()+1);
          try {
            (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
          } catch (...) {
            // unknown language
            (*o)[o->length()-1].name = CORBA::string_dup("");
          }
          (*o)[o->length()-1].id    = sd->getId();
          (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
        }
      }
      return o;
  }//try
  catch (const ccReg::Whois::InternalServerError& )
  {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( ERROR_LOG
              , "getNSSetStatusDescList: ccReg::Whois::InternalServerError");
      throw;
  }
  catch (const std::exception& ex)
  {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( ERROR_LOG
              , "getNSSetStatusDescList: std::exception %s", ex.what());
      throw ccReg::Whois::InternalServerError();
  }
  catch (...)
  {
      Logging::Manager::instance_ref()
          .get(server_name_.c_str())
          .message( ERROR_LOG
                  , "getNSSetStatusDescList: unknown exception ");
      throw ccReg::Whois::InternalServerError();
  }
}//ccReg_Whois_i::getNSSetStatusDescList

Registry::ObjectStatusDescSeq* ccReg_Whois_i::getKeySetStatusDescList(const char *lang)
{
  Logging::Context ctx(server_name_);
  ConnectionReleaser releaser;
    try
    {
      Registry::ObjectStatusDescSeq *o = new Registry::ObjectStatusDescSeq;
        for (unsigned int i = 0; i < registry_manager_->getStatusDescCount(); i++) {
            const Fred::StatusDesc *sd = registry_manager_->getStatusDescByIdx(i);
            if (sd->getExternal() && sd->isForType(4)) {
                o->length(o->length() + 1);
                try {
                    (*o)[o->length()-1].name = DUPSTRC(sd->getDesc(lang));
                }
                catch (...) {
                    //unknown lang
                    (*o)[o->length()-1].name = CORBA::string_dup("");
                }
                (*o)[o->length()-1].id    = sd->getId();
                (*o)[o->length()-1].shortName = DUPSTRFUN(sd->getName);
            }
        }
        return o;
    }//try
    catch (const ccReg::Whois::InternalServerError& )
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getKeySetStatusDescList: ccReg::Whois::InternalServerError");
        throw;
    }
    catch (const std::exception& ex)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                , "getKeySetStatusDescList: std::exception %s", ex.what());
        throw ccReg::Whois::InternalServerError();
    }
    catch (...)
    {
        Logging::Manager::instance_ref()
            .get(server_name_.c_str())
            .message( ERROR_LOG
                    , "getKeySetStatusDescList: unknown exception ");
        throw ccReg::Whois::InternalServerError();
    }
}//ccReg_Whois_i::getKeySetStatusDescList

