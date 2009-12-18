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

#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <vector>
#include <algorithm>
#include <functional>


#include "registrar.h"
#include "common_impl.h"
#include "sql.h"
#include "old_utils/dbsql.h"
#include "old_utils/log.h"

#include "model/model_filters.h"
#include "log/logger.h"
#include "log/context.h"

#include "model_registrar_acl.h"
#include "model_registrar.h"

#include "zone.h"

namespace Register {
namespace Registrar {

class RegistrarImpl;


class ACLImpl : virtual public ACL,
				private ModelRegistrarAcl {

public:
  ACLImpl()
	  : ModelRegistrarAcl()
  {
	  //ModelRegistrarAcl::setId(0);
  }
  ACLImpl(TID _id,
          const std::string& _certificateMD5,
          const std::string& _password)
	  : ModelRegistrarAcl()
  {
	  ModelRegistrarAcl::setId(_id);
	  ModelRegistrarAcl::setCert(_certificateMD5);
	  ModelRegistrarAcl::setPassword(_password);
  }
  virtual const TID& getId() const
  {
      return ModelRegistrarAcl::getId();
  }
  virtual void setId(const TID &_id)
  {
	  ModelRegistrarAcl::setId(_id);
  }
  const TID& getRegistarId() const
  {
      return ModelRegistrarAcl::getRegistarId();
  }
  void setRegistrarId(const TID &_registrar_id)
  {
    	ModelRegistrarAcl::setRegistrarId(_registrar_id);
  }
  virtual const std::string& getCertificateMD5() const
  {
    return ModelRegistrarAcl::getCert();
  }
  virtual void setCertificateMD5(const std::string& _certificateMD5)
  {
	  ModelRegistrarAcl::setCert(_certificateMD5);
  }
  virtual const std::string& getPassword() const
  {
    return ModelRegistrarAcl::getPassword();
  }
  virtual void setPassword(const std::string& _password)
  {
	  ModelRegistrarAcl::setPassword(_password);
  }
  bool operator==(const TID _id) const
  {
    return getId() == _id;
  }
  virtual void save() throw (SQL_ERROR)
  {
      TRACE("[CALL] ACLImpl::save()");
	try
	{
		Database::Connection conn = Database::Manager::acquire();
		TID id = getId();
		LOGGER(PACKAGE).debug(boost::format("ACLImpl::save id: %1% RegistrarId: %2%")
		% id % getRegistarId());
		if (id)
			ModelRegistrarAcl::update();
		else
			ModelRegistrarAcl::insert();
	}//try
	catch (...)
	{
	 LOGGER(PACKAGE).error("save: an error has occured");
	 throw SQL_ERROR();
	}//catch (...)
  }//save
};//class ACLImpl


class RegistrarImpl : public Register::CommonObjectImplNew,
                      virtual public Registrar,
                      private ModelRegistrar
{
  typedef boost::shared_ptr<ACLImpl> ACLImplPtr;
  typedef std::vector<ACLImplPtr> ACLList;
  typedef ACLList::iterator ACLListIter;

  unsigned long credit; ///< DB: registrar.credit

  ACLList acl; ///< access control
  typedef std::map<Database::ID, unsigned long> ZoneCreditMap;
  ZoneCreditMap zone_credit_map;

  typedef  boost::shared_ptr<ZoneAccess> ZoneAccessPtr;
  typedef std::vector<ZoneAccessPtr> ZoneAccessList;
  typedef ZoneAccessList::iterator ZoneAccessListIter;
  ZoneAccessList actzones;

public:
  RegistrarImpl()
  : CommonObjectImplNew()
  , ModelRegistrar()

  {}
  RegistrarImpl(TID _id,
                const std::string& _ico,
                const std::string& _dic,
                const std::string& _var_symb,
                bool _vat,
                const std::string& _handle,
                const std::string& _name,
                const std::string& _url,
                const std::string& _organization,
                const std::string& _street1,
                const std::string& _street2,
                const std::string& _street3,
                const std::string& _city,
                const std::string& _province,
                const std::string& _postalCode,
                const std::string& _country,
                const std::string& _telephone,
                const std::string& _fax,
                const std::string& _email,
                bool _system,
                unsigned long _credit) :
        CommonObjectImplNew(),
        ModelRegistrar(),
        credit(_credit)
  {
	ModelRegistrar::setId(_id);
	ModelRegistrar::setIco(_ico);
	ModelRegistrar::setDic(_dic);
	ModelRegistrar::setVarsymb(_var_symb);
	ModelRegistrar::setVat(_vat);
	ModelRegistrar::setHandle(_handle);
	ModelRegistrar::setName(_name);
	ModelRegistrar::setOrganization(_organization);
	ModelRegistrar::setStreet1(_street1);
	ModelRegistrar::setStreet2(_street2);
	ModelRegistrar::setStreet3(_street3);
	ModelRegistrar::setCity(_city);
	ModelRegistrar::setStateorprovince(_province);
	ModelRegistrar::setPostalcode(_postalCode);
	ModelRegistrar::setCountry(_country);
	ModelRegistrar::setTelephone(_telephone);
	ModelRegistrar::setFax(_fax);
	ModelRegistrar::setEmail(_email);
	ModelRegistrar::setUrl(_url);
	ModelRegistrar::setSystem(_system);
  }
  void clear() {
    acl.clear();
    actzones.clear();
  }
  ~RegistrarImpl() {}

  virtual const TID& getId() const
  {
     return ModelRegistrar::getId();
  }
  virtual void setId(const TID &_id)
  {
	ModelRegistrar::setId(_id);
  }
  virtual const std::string& getIco() const
  {
    return ModelRegistrar::getIco();
  }
  virtual void setIco(const std::string& _ico)
  {
	ModelRegistrar::setIco(_ico);
  }
  virtual const std::string& getDic() const
  {
    return ModelRegistrar::getDic();
  }
  virtual void setDic(const std::string& _dic)
  {
	ModelRegistrar::setDic(_dic);
  }
  virtual const std::string& getVarSymb() const
  {
    return ModelRegistrar::getVarsymb();
  }
  virtual void setVarSymb(const std::string& _var_symb)
  {
	ModelRegistrar::setVarsymb(_var_symb);
  }
  virtual bool getVat() const
  {
    return ModelRegistrar::getVat();
  }
  virtual void setVat(bool _vat)
  {
	ModelRegistrar::setVat(_vat);
  }
  virtual const std::string& getHandle() const
  {
    return ModelRegistrar::getHandle();
  }
  virtual void setHandle(const std::string& _handle)
  {
	ModelRegistrar::setHandle(_handle);
  }
  virtual const std::string& getName() const
  {
    return ModelRegistrar::getName();
  }
  virtual void setName(const std::string& _name)
  {
	ModelRegistrar::setName(_name);
  }
  virtual const std::string& getURL() const
  {
    return ModelRegistrar::getUrl();
  }
  virtual void setURL(const std::string& _url)
  {
	ModelRegistrar::setUrl(_url);
  }
  virtual const std::string& getOrganization() const
  {
    return ModelRegistrar::getOrganization();
  }
  virtual void setOrganization(const std::string& _organization)
  {
	ModelRegistrar::setOrganization(_organization);
  }
  virtual const std::string& getStreet1() const
  {
    return ModelRegistrar::getStreet1();
  }
  virtual void setStreet1(const std::string& _street1)
  {
	ModelRegistrar::setStreet1(_street1);
  }
  virtual const std::string& getStreet2() const
  {
    return ModelRegistrar::getStreet2();
  }
  virtual void setStreet2(const std::string& _street2)
  {
	ModelRegistrar::setStreet2(_street2);
  }
  virtual const std::string& getStreet3() const
  {
    return ModelRegistrar::getStreet3();
  }
  virtual void setStreet3(const std::string& _street3)
  {
	ModelRegistrar::setStreet3(_street3);
  }
  virtual const std::string& getCity() const
  {
	return ModelRegistrar::getCity();
  }
  virtual void setCity(const std::string& _city)
  {
	ModelRegistrar::setCity(_city);
  }
  virtual const std::string& getProvince() const
  {
    return ModelRegistrar::getStateorprovince();
  }
  virtual void setProvince(const std::string& _province)
  {
	ModelRegistrar::setStateorprovince(_province);
  }
  virtual const std::string& getPostalCode() const
  {
    return ModelRegistrar::getPostalcode();
  }
  virtual void setPostalCode(const std::string& _postalCode)
  {
	ModelRegistrar::setPostalcode(_postalCode);
  }
  virtual const std::string& getCountry() const
  {
    return ModelRegistrar::getCountry();
  }
  virtual void setCountry(const std::string& _country)
  {
	ModelRegistrar::setCountry(_country);
  }
  virtual const std::string& getTelephone() const
  {
    return ModelRegistrar::getTelephone();
  }
  virtual void setTelephone(const std::string& _telephone)
  {
	ModelRegistrar::setTelephone(_telephone);
  }
  virtual const std::string& getFax() const
  {
    return ModelRegistrar::getFax();
  }
  virtual void setFax(const std::string& _fax)
  {
	ModelRegistrar::setFax(_fax);
  }
  virtual const std::string& getEmail() const
  {
    return ModelRegistrar::getEmail();
  }
  virtual void setEmail(const std::string& _email)
  {
	ModelRegistrar::setEmail(_email);
  }
  virtual bool getSystem() const
  {
    return ModelRegistrar::getSystem();
  }
  virtual void setSystem(bool _system)
  {
	ModelRegistrar::setSystem(_system);
  }

  virtual unsigned long getCredit() const {
    return credit;
  }
  
  virtual unsigned long getCredit(Database::ID _zone_id) const
  {
      unsigned long ret = 0;
      ZoneCreditMap::const_iterator it;
      it = zone_credit_map.find(_zone_id);
      if(it != zone_credit_map.end())
          ret = it->second;
    return ret;
  }
  
  virtual void setCredit(Database::ID _zone_id, unsigned long _credit) {
    zone_credit_map[_zone_id] = _credit;
    credit += _credit;
  }
  
  virtual unsigned getACLSize() const {
    return acl.size();
  }
  
  virtual unsigned getZoneAccessSize() const {
      return actzones.size();
    }

  virtual ACL* getACL(unsigned idx) const
  {
    return idx < acl.size() ? acl[idx].get() : NULL;
  }
  virtual ZoneAccess* getZoneAccess(unsigned idx) const
  {
    return idx < actzones.size() ? actzones[idx].get() : NULL;
  }

  virtual ACL* newACL()
  {
    boost::shared_ptr<ACLImpl> newACL ( new ACLImpl());
    acl.push_back(newACL);
    return newACL.get();
  }

  virtual ZoneAccess* newZoneAccess()
  {
    boost::shared_ptr<ZoneAccess> newZoneAccess ( new ZoneAccess());
    actzones.push_back(newZoneAccess);
    return newZoneAccess.get();
  }

  virtual void deleteACL(unsigned idx)
  {
    if (idx < acl.size()) {
      acl.erase(acl.begin()+idx);
    }
  }

  virtual void deleteZoneAccess(unsigned idx)
  {
    if (idx < actzones.size()) {
        actzones.erase(actzones.begin()+idx);
    }
  }


  virtual void clearACLList()
  {
    acl.clear();
  }

  virtual void clearZoneAccessList()
  {
      actzones.clear();
  }


  virtual unsigned long addRegistrarZone(
          const std::string& registrarHandle,
          const std::string zone,
          const Database::Date &fromDate,
          const Database::Date &toDate) throw (SQL_ERROR)
  {///expecting external transaction, no transaction inside
      try
      {
        Database::Connection conn = Database::Manager::acquire();

        std::string fromStr;
        std::string toStr;

        if (fromDate != Database::Date())
        {
            fromStr = "'" + fromDate.to_string() + "'";
        }
        else
        {
            fromStr = "CURRENT_DATE";
        }
        if (toDate != Database::Date())
        {
            toStr = "'" + toDate.to_string() + "'";
        }
        else
        {
            toStr = "NULL";
        }
        std::stringstream sql;
        sql << "INSERT INTO registrarinvoice (registrarid,zone,fromdate,lastdate) "
            << "SELECT r.id,z.id, " << fromStr << "," << toStr << " FROM ("
            << "SELECT id FROM registrar WHERE handle='" << conn.escape(registrarHandle)
            << "') r " << "JOIN (SELECT id FROM zone WHERE fqdn='" << conn.escape(zone)
            << "') z ON (1=1) " << "LEFT JOIN registrarinvoice ri ON "
            << "(ri.registrarid=r.id AND ri.zone=z.id) " << "WHERE ri.id ISNULL";


        conn.exec(sql.str());

        std::stringstream sql2;
        sql2 << "SELECT ri.id"
         "FROM (SELECT id FROM registrar WHERE handle='" << conn.escape(registrarHandle) << "') r "
         "JOIN (SELECT id FROM zone WHERE fqdn='" << conn.escape(zone) << "') z ON (1=1) "
         "LEFT JOIN registrarinvoice ri ON (ri.registrarid=r.id AND ri.zone=z.id) "
         "WHERE fromdate = date" << fromStr << " and ";
        if(toStr.compare("NULL") == 0)
            sql2 << "todate isnull";
        else
            sql2 <<  "todate = date " << toStr;

        Database::Result res = conn.exec(sql2.str());

        if((res.size() == 1) && (res[0].size() == 1))
        {
            Database::ID ret = res[0][0];
            return ret;
        }
        else
        {
            LOGGER(PACKAGE).error("addRegistrarZone: an error has occured");
            throw SQL_ERROR();
        }
      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("addRegistrarZone: an error has occured");
          throw SQL_ERROR();
      }//catch (...)
  }//addRegistrarZone

  virtual void updateRegistrarZone(
          const TID& id,
          const Database::Date &fromDate,
          const Database::Date &toDate) throw (SQL_ERROR)
  {///expecting external transaction, no transaction inside
      try
      {
        Database::Connection conn = Database::Manager::acquire();

        std::string fromStr;
        std::string toStr;

        if (fromDate != Database::Date())
        {
            fromStr = "'" + fromDate.to_string() + "'";
        }
        else
        {
            fromStr = "CURRENT_DATE";
        }
        if (toDate != Database::Date())
        {
            toStr = "'" + toDate.to_string() + "'";
        }
        else
        {
            toStr = "NULL";
        }
        std::stringstream sql;

        sql << "UPDATE registrarinvoice SET fromdate = date " << fromStr
            << ",todate =  date " << toStr
            << " WHERE id = " << id << ";";

        conn.exec(sql.str());

      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("updateRegistrarZone: an error has occured");
          throw SQL_ERROR();
      }//catch (...)
  }//updateRegistrarZone

  /// Look if registrar have access to zone by zone id
  virtual bool isInZone(unsigned id) const
  {
      bool ret = false;
      try
      {
        Database::Connection conn = Database::Manager::acquire();
        std::stringstream sql;

        sql << "select count(*) from registrarinvoice ri "
               "where fromdate <= CURRENT_DATE and (todate >= CURRENT_DATE or todate is null) "
               "and ri.registrarid = " << getId() << " and ri.zone = " << id << " ;";

        Database::Result res = conn.exec(sql.str());

        if((res.size() != 1) && (res[0].size() != 1))
        {
            LOGGER(PACKAGE).error("isInZone by id: an error has occured");
            throw SQL_ERROR();
        }

        unsigned count = res[0][0];
        if (count > 1 )
        {
            LOGGER(PACKAGE).error("isInZone by id: bad data in table registrarinvoice");
            ret = true;
        }

        if (count == 1 ) ret = true;

      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("isInZone by id: an error has occured");
          throw SQL_ERROR();
      }//catch (...)

      return ret;
  }//isInZone by id

  /// Look if registrar have access to zone by zone fqdn
  virtual bool isInZone(std::string zone_fqdn) const
  {
      bool ret = false;
      try
      {
        Database::Connection conn = Database::Manager::acquire();
        std::stringstream sql;

        sql << "select count(*) from registrarinvoice ri join zone z on ri.zone = z.id "
               "where fromdate <= CURRENT_DATE and (todate >= CURRENT_DATE or todate is null) "
               "and ri.registrarid = " << getId() << " and z.fqdn = " << conn.escape(zone_fqdn) << " ;";

        Database::Result res = conn.exec(sql.str());

        if((res.size() != 1) && (res[0].size() != 1))
        {
            LOGGER(PACKAGE).error("isInZone by fqdn: an error has occured");
            throw SQL_ERROR();
        }

        unsigned count = res[0][0];
        if (count > 1 )
        {
            LOGGER(PACKAGE).error("isInZone by fqdn: bad data in table registrarinvoice");
            ret = true;
        }

        if (count == 1 ) ret = true;

      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("isInZone by fqdn: an error has occured");
          throw SQL_ERROR();
      }//catch (...)

      return ret;
  }//isInZone by fqdn


  virtual void save() throw (SQL_ERROR)
  {
    TRACE("[CALL] RegistrarImpl::save()");
      // save registrar data
	try
	{
		Database::Connection conn = Database::Manager::acquire();
		Database::Transaction tx(conn);
		TID id = getId();
	    LOGGER(PACKAGE).debug(boost::format
	            ("RegistrarImpl::save : id: %1%")
	                % id);

		if (id)
			ModelRegistrar::update();
		else
		{
			ModelRegistrar::insert();
			ModelRegistrar::reload();
			id = getId();
	        LOGGER(PACKAGE).debug(boost::format
	                ("RegistrarImpl::save after reload: id: %1%")
	                    % id);
		}


    	std::ostringstream sql;
		sql << "DELETE FROM registraracl WHERE registrarid=" << id;
		conn.exec(sql.str());
		for (unsigned j = 0; j < acl.size(); j++)
		{
			acl[j]->setRegistrarId(id);
			acl[j]->save();
		}//for acl

		for (unsigned i = 0; i < actzones.size(); i++)
		{
		    if(actzones[i]->id)//use addRegistrarZone or updateRegistrarZone
                this->updateRegistrarZone (actzones[i]->id
                            ,actzones[i]->fromdate
                            ,actzones[i]->todate);
		    else
		    {
		        this->addRegistrarZone (getHandle()
                            ,actzones[i]->name
                            ,actzones[i]->fromdate
                            ,actzones[i]->todate);
		    }//else id
		}//for actzones

		tx.commit();
	}//try
	catch (...)
	{
	 LOGGER(PACKAGE).error("save: an error has occured");
	 throw SQL_ERROR();
	}//catch (...)
  }//save()

  void putACL(TID _id,
              const std::string& certificateMD5,
              const std::string& password)
  {
    acl.push_back(ACLImplPtr(new ACLImpl(_id,certificateMD5,password)));
  }

  void putZoneAccess(TID _id
          , std::string _name
          , unsigned long _credit
          , Database::Date _fromdate
          , Database::Date _todate)
  {
      actzones.push_back(ZoneAccessPtr(new ZoneAccess(_id,_name,_credit,_fromdate,_todate)));
  }

  bool hasId(TID _id) const
  {
    return getId() == _id;
  }
  void resetId()
  {
    setId(0);
    for (TID i = 0; i < acl.size(); i++)
      acl[i]->setId(0);
    for (TID i = 0; i < actzones.size(); i++)
        actzones[i]->id=0;
  }
};//class RegistrarImpl

COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Name)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Handle)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, URL)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Email)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Credit)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Ico)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Dic)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, VarSymb)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Vat)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Organization)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Street1)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Street2)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Street3)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, City)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Province)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, PostalCode)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Country)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Telephone)
COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Fax)

//COMPARE_CLASS_IMPL_NEW(RegistrarImpl, Credit)

class CompareCreditByZone
{
    bool asc_;
    unsigned zone_id_;
    RZAPtr rzaptr_;
public:
  CompareCreditByZone(bool _asc, unsigned _zone_id
          , RZAPtr _rzaptr)
      : asc_(_asc), zone_id_(_zone_id), rzaptr_(_rzaptr) { }
  bool operator()(CommonObjectNew *_left, CommonObjectNew *_right) const
  {
    RegistrarImpl *l_casted = dynamic_cast<RegistrarImpl *>(_left);
    RegistrarImpl *r_casted = dynamic_cast<RegistrarImpl *>(_right);
    if (l_casted == 0 || r_casted == 0)
    {
      /* this should never happen */
      throw std::bad_cast();
    }


    long long lvalue = l_casted->getCredit(zone_id_);

    long long rvalue = r_casted->getCredit(zone_id_);

     if (rzaptr_)
     {
         Register::Registrar::Manager::RegistrarZoneAccess* rzaptr =0;
         rzaptr = static_cast<Register::Registrar::Manager::RegistrarZoneAccess*>(rzaptr_);

         if(rzaptr->isInZone(l_casted->getId(),zone_id_) == false) lvalue = -1;
         if(rzaptr->isInZone(r_casted->getId(),zone_id_) == false) rvalue = -1;
     }

    return (asc_ ? (lvalue < rvalue) : (lvalue > rvalue));
  }
};//class CompareCreditByZone



class RegistrarListImpl : public Register::CommonListImplNew,
                          public RegistrarList {
  
  std::string name;
  std::string handle;
  std::string xml;
  TID idFilter;
  std::string zoneFilter;
  long long ptr_idx_;//from CommonListImpl
public:
  RegistrarListImpl() :
    CommonListImplNew(), idFilter(0) {
  }
  ~RegistrarListImpl() {
    clear();
  }
  virtual void setIdFilter(TID _idFilter) {
    idFilter = _idFilter;
  }
  virtual void setHandleFilter(const std::string& _handle) {
    handle = _handle;
  }
  virtual void setXMLFilter(const std::string& _xml) {
    xml = _xml;
  }
  virtual void setNameFilter(const std::string& _name) {
    name = _name;
  }
  virtual void setZoneFilter(const std::string& _zone) {
    zoneFilter = _zone;
  }
  void resetIDSequence()
  {
    ptr_idx_ = -1;
  }
  RegistrarImpl* findIDSequence(TID _id)
  {
    // must be sorted by ID to make sence
    if (ptr_idx_ < 0)
      ptr_idx_ = 0;
    long long m_data_size = m_data.size();
    RegistrarImpl* ret_ptr=0;

    for (; ptr_idx_ < m_data_size
			&& ((dynamic_cast<RegistrarImpl* >(m_data[ptr_idx_]))->getId()<_id)
			; ptr_idx_++);
    if (ptr_idx_ == m_data_size
    		|| (ret_ptr = dynamic_cast<RegistrarImpl* >(m_data[ptr_idx_]))->getId() != _id)
    {
      LOGGER(PACKAGE).debug(boost::format("find id sequence: not found in result set. (id=%1%, ptr_idx=%2%)")
                                          % _id % ptr_idx_);
      resetIDSequence();
      return NULL;
    }//if
    return ret_ptr;
  }//findIDSequence

  virtual void reload() throw (SQL_ERROR)
  {
	try
	{
		Database::Connection conn = Database::Manager::acquire();
		clear();
		std::ostringstream sql;
		sql << "SELECT r.id,r.handle,r.name,r.url,r.organization,"
		 << "r.street1,r.street2,r.street3,r.city,r.stateorprovince,"
		 << "r.postalcode,r.country,r.telephone,r.fax,r.email,r.system,"
		 << "COALESCE(SUM(i.credit),0) " << "FROM registrar r "
		 << "LEFT JOIN invoice i ON (r.id=i.registrarid AND "
		 << "NOT(i.credit ISNULL)) ";
		if (!zoneFilter.empty())
		sql << "LEFT JOIN (SELECT z.fqdn, ri.registrarid "
		   << "FROM zone z, registrarinvoice ri "
		   << "WHERE z.id=ri.zone AND ri.fromdate<=CURRENT_DATE) t "
		   << "ON (t.registrarid=r.id AND t.fqdn='" << zoneFilter << "') ";
		sql << "WHERE 1=1 ";
		if (!zoneFilter.empty())
		sql << "AND NOT(t.fqdn ISNULL) ";
		if (idFilter) sql << "AND " << "r.id" << "=" << idFilter << " ";
		if (!name.empty())
			sql << "AND " << "r.name" << " ILIKE TRANSLATE('"
				<< conn.escape(name) << "','*?','%_') ";
		if (!handle.empty())
		{
		    if (handle.find('?') != std::string::npos)
		      sql << "AND " << "r.handle"
		      << " ILIKE TRANSLATE('" << conn.escape(handle) << "','*?','%_') ";
		    else
		    	sql << "AND " << "r.handle" << "="
		           << "'" << conn.escape(handle) << "'" << " ";
		}
		sql << "GROUP BY r.id,r.handle,r.name,r.url,r.organization,"
		 << "r.street1,r.street2,r.street3,r.city,r.stateorprovince,"
		 << "r.postalcode,r.country,r.telephone,r.fax,r.email,r.system ";
		sql << "ORDER BY r.id ";

		Database::Result res = conn.exec(sql.str());

		for (unsigned i=0; i < static_cast<unsigned>(res.size()); i++)
		{
			appendToList(new RegistrarImpl
							(
							  res[i][0]
							  ,""
							  ,""
							  ,""
							  ,true
							  ,res[i][1]
							  ,res[i][2]
							  ,res[i][3]
							  ,res[i][4]
							  ,res[i][5]
							  ,res[i][6]
							  ,res[i][7]
							  ,res[i][8]
							  ,res[i][9]
							  ,res[i][10]
							  ,res[i][11]
							  ,res[i][12]
							  ,res[i][13]
							  ,res[i][14]
							  ,res[i][15]
							  ,res[i][16]
							)//new
						   );//push_back
		}//for

        resetIDSequence();
		sql.str("");
		sql << "SELECT registrarid,cert,password " << "FROM registraracl ORDER BY registrarid";

		Database::Result res2 = conn.exec(sql.str());
		for (unsigned i=0; i < static_cast<unsigned>(res2.size()); i++)
		{
		  // find associated registrar
		  unsigned registrarId = res2[i][0];
		  RegistrarImpl *r = dynamic_cast<RegistrarImpl* >(findIDSequence(registrarId));

		  if (r)
		  {
			r->putACL(0, res2[i][1], res2[i][2]);
		  }
		}//for

	}//try
	catch (...)
	{
	 LOGGER(PACKAGE).error("reload: an error has occured");
	 throw SQL_ERROR();
	}//catch (...)
  }//reload

  virtual void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] RegistrarListImpl::reload()");
    clear();
    uf.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery info_query;
    std::auto_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    for (fit->first(); !fit->isDone(); fit->next()) {
      Database::Filters::Registrar *rf =
          dynamic_cast<Database::Filters::Registrar*>(fit->get());
      if (!rf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect("id ico dic varsymb vat handle name url organization street1 street2 street3 "
                       "city stateorprovince postalcode country telephone fax email system",
                     rf->joinRegistrarTable());
      uf.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    /* manually add query part to order result by id
     * need for findIDSequence() */
    uf.serialize(info_query);
    std::string info_query_str = str(boost::format("%1% ORDER BY id LIMIT %2%") % info_query.str() % m_limit);
    try {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result r_info = conn.exec(info_query_str);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID  rid          = *col;
        std::string   ico          = *(++col);
        std::string   dic          = *(++col);
        std::string   var_symb     = *(++col);
        bool          vat          = *(++col);
        std::string   handle       = *(++col);
        std::string   name         = *(++col);
        std::string   url          = *(++col);
        std::string   organization = *(++col);
        std::string   street1      = *(++col);
        std::string   street2      = *(++col);
        std::string   street3      = *(++col);
        std::string   city         = *(++col);
        std::string   province     = *(++col);
        std::string   postal_code  = *(++col);
        std::string   country      = *(++col);
        std::string   telephone    = *(++col);
        std::string   fax          = *(++col);
        std::string   email        = *(++col);
        bool          system       = *(++col);
        unsigned long credit       = 0;

        m_data.push_back(new RegistrarImpl(
                rid,
                ico,
                dic,
                var_symb,
                vat,
                handle,
                name,
                url,
                organization,
                street1,
                street2,
                street3,
                city,
                province,
                postal_code,
                country,
                telephone,
                fax,
                email,
                system,
                credit));
      }

      if (m_data.empty())
        return;
      
      Database::SelectQuery credit_query;
      credit_query.select() << "zone, registrarid, COALESCE(SUM(credit), 0)";
      credit_query.from() << "invoice";
      credit_query.group_by() << "registrarid, zone";
      credit_query.order_by() << "registrarid";

      resetIDSequence();
      Database::Result r_credit = conn.exec(credit_query);
      for (Database::Result::Iterator it = r_credit.begin(); it != r_credit.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID  zone_id      = *col;
        Database::ID  registrar_id = *(++col);
        unsigned long credit       = *(++col);
        
        RegistrarImpl *registrar_ptr = dynamic_cast<RegistrarImpl* >(findIDSequence(registrar_id));
        if (registrar_ptr)
        {
          registrar_ptr->setCredit(zone_id, credit);
        }
      }
      
      resetIDSequence();
      Database::SelectQuery acl_query;
      acl_query.select() << "id, registrarid, cert, password ";
      acl_query.from() << "registraracl";
      acl_query.order_by() << "registrarid";
      
      Database::Result r_acl = conn.exec(acl_query);
      for (Database::Result::Iterator it = r_acl.begin(); it != r_acl.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID acl_id       = *col;
        Database::ID registrar_id = *(++col);
        std::string  cert_md5     = *(++col);
        std::string  password     = *(++col);

        RegistrarImpl *registrar_ptr = dynamic_cast<RegistrarImpl* >(findIDSequence(registrar_id));
        if (registrar_ptr) {
          registrar_ptr->putACL(acl_id, cert_md5, password);
        }
      }//for acl_query

      resetIDSequence();
      Database::SelectQuery azone_query;
      azone_query.select() <<   "ri.id as id, ri.registrarid as registrarid ,z.fqdn as name, "
                                "case when ri.todate = mtd.max_todate "
                                    "then cr.credit "
                                    "else null end as credit "
                                ", ri.fromdate as fromdate , ri.todate as todate ";
      azone_query.from() <<   "registrarinvoice ri "
                              "join zone z on ri.zone = z.id "
                              "left join (select i.zone, i.registrarid "
                                  ", COALESCE(SUM(credit), 0) as credit "
                                  "from invoice i "
                                  "group by i.registrarid, i.zone "
                                  ") as cr on cr.zone = ri.zone "
                                  "and ri.registrarid = cr.registrarid "
                              "join (select ri.registrarid as rid, ri.zone as zid "
                                  ",max(todate) as max_todate "
                                  "from registrarinvoice ri "
                                  "join zone z on ri.zone = z.id "
                                  "group by ri.registrarid, ri.zone ) as mtd "
                                  "on mtd.rid = ri.registrarid "
                                  "and mtd.zid = ri.zone ";
      azone_query.order_by() << "ri.registrarid,ri.zone,ri.todate desc ";

      Database::Result r_azone = conn.exec(azone_query);
      for (Database::Result::Iterator it = r_azone.begin(); it != r_azone.end(); ++it)
      {
        Database::Row::Iterator col = (*it).begin();

        Database::ID azone_id = *col;
        Database::ID registrar_id = *(++col);
        std::string zone_name = *(++col);
        unsigned long credit = *(++col);
        Database::Date fromdate = *(++col);
        Database::Date todate = *(++col);

        RegistrarImpl *registrar_ptr = dynamic_cast<RegistrarImpl* >(findIDSequence(registrar_id));
        if (registrar_ptr)
        {
          registrar_ptr->putZoneAccess(azone_id, zone_name, credit, fromdate, todate);
        }

      }//for r_azone

      /* checks if row number result load limit is active and set flag */ 
      CommonListImplNew::reload();
    }//try
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }
  
  virtual Registrar* get(unsigned _idx) const {
    try {
      Registrar *registrar = dynamic_cast<Registrar* >(m_data.at(_idx));
      if (registrar) 
        return registrar;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    } 
  }

  /* XXX Better if we have list of shared pointers than this */
  virtual Registrar* getAndRelease(unsigned int _idx) {
    try {
      Registrar *registrar = dynamic_cast<Registrar* >(m_data.at(_idx));
      if (registrar) {
        m_data.erase(m_data.begin() + _idx);
        return registrar;
      }
      else {
        throw std::exception();
      }
    }
    catch (...) {
      throw std::exception();
    }
  }

  virtual Register::Registrar::Registrar* findId(Database::ID id) const
  {
	  std::vector<Register::CommonObjectNew*>::const_iterator it = std::find_if(m_data.begin(),
	  m_data.end(),
	  CheckIdNew<Register::Registrar::Registrar>(id));

	  if (it != m_data.end())
	  {
		  LOGGER(PACKAGE).debug(boost::format("object list hit! object id=%1% found")
		  % id);
		  return dynamic_cast<Register::Registrar::Registrar*>(*it);
	  }
	  LOGGER(PACKAGE).debug(boost::format("object list miss! object id=%1% not found")
	  % id);
	  throw Register::NOT_FOUND();
  }
  
  virtual Registrar* create()
  {
    return new RegistrarImpl();
  }
  void clearFilter() {
    name = "";
    handle = "";
    zoneFilter = "";
  }
  
  virtual void sort(MemberType _member, bool _asc, unsigned _zone_id
          , RZAPtr rzaptr) {
    switch (_member) {
      case MT_NAME:
        stable_sort(m_data.begin(), m_data.end(), CompareName(_asc));
        break;
      case MT_HANDLE:
        stable_sort(m_data.begin(), m_data.end(), CompareHandle(_asc));
        break;
      case MT_URL:
        stable_sort(m_data.begin(), m_data.end(), CompareURL(_asc));
        break;
      case MT_MAIL:
        stable_sort(m_data.begin(), m_data.end(), CompareEmail(_asc));
        break;
      case MT_CREDIT:
        stable_sort(m_data.begin(), m_data.end(), CompareCredit(_asc));
        break;
      case MT_ICO:
        stable_sort(m_data.begin(), m_data.end(), CompareIco(_asc));
        break;
      case MT_DIC:
        stable_sort(m_data.begin(), m_data.end(), CompareDic(_asc));
        break;
      case MT_VARSYMB:
        stable_sort(m_data.begin(), m_data.end(), CompareVarSymb(_asc));
        break;
      case MT_VAT:
        stable_sort(m_data.begin(), m_data.end(), CompareVat(_asc));
        break;
      case MT_ORGANIZATION:
        stable_sort(m_data.begin(), m_data.end(), CompareOrganization(_asc));
        break;
      case MT_STREET1:
        stable_sort(m_data.begin(), m_data.end(), CompareStreet1(_asc));
        break;
      case MT_STREET2:
        stable_sort(m_data.begin(), m_data.end(), CompareStreet2(_asc));
        break;
      case MT_STREET3:
        stable_sort(m_data.begin(), m_data.end(), CompareStreet3(_asc));
        break;
      case MT_CITY:
        stable_sort(m_data.begin(), m_data.end(), CompareCity(_asc));
        break;
      case MT_PROVINCE:
        stable_sort(m_data.begin(), m_data.end(), CompareProvince(_asc));
        break;
      case MT_POSTALCODE:
        stable_sort(m_data.begin(), m_data.end(), ComparePostalCode(_asc));
        break;
      case MT_COUNTRY:
        stable_sort(m_data.begin(), m_data.end(), CompareCountry(_asc));
        break;
      case MT_TELEPHONE:
        stable_sort(m_data.begin(), m_data.end(), CompareTelephone(_asc));
        break;
      case MT_FAX:
        stable_sort(m_data.begin(), m_data.end(), CompareFax(_asc));
        break;
      case MT_ZONE:
        stable_sort(m_data.begin(), m_data.end()
                , CompareCreditByZone(_asc , _zone_id, rzaptr));
        break;
    }
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const
  {
    // empty implementation
  }
  virtual const char* getTempTableName() const
  {
    return "";
  }
  
};//class RegistrarListImpl

class EPPActionImpl : public CommonObjectImpl,
                      virtual public EPPAction {
  TID sessionId;
  unsigned type;
  std::string typeName;
  ptime startTime;
  std::string serverTransactionId;
  std::string clientTransactionId;
  unsigned result;
  TID registrarId;
  std::string registrarHandle;
  std::string handle;
  std::string message;
  std::string message_out;
  
public:
  EPPActionImpl(
				TID _id,
                TID _sessionId,
                unsigned _type,
                const std::string& _typeName,
                ptime _startTime,
                const std::string& _serverTransactionId,
                const std::string& _clientTransactionId,
                unsigned _result,
                TID _registrarId,
                const std::string& _registrarHandle,
                const std::string& _handle,
                const std::string& _message = std::string(),
                const std::string& _message_out = std::string()) :
    CommonObjectImpl(), sessionId(_sessionId), type(_type), typeName(_typeName),
        startTime(_startTime), serverTransactionId(_serverTransactionId),
        clientTransactionId(_clientTransactionId), result(_result), registrarId(_registrarId),
        registrarHandle(_registrarHandle), handle(_handle), 
        message(_message), message_out(_message_out) {
  }
  
  virtual TID getSessionId() const {
    return sessionId;
  }
  virtual unsigned getType() const {
    return type;
  }
  virtual const std::string& getTypeName() const {
    return typeName;
  }
  virtual const ptime getStartTime() const {
    return startTime;
  }
  virtual const std::string& getServerTransactionId() const {
    return serverTransactionId;
  }
  virtual const std::string& getClientTransactionId() const {
    return clientTransactionId;
  }
  virtual const std::string& getEPPMessageIn() const {
    return message;
  }
  virtual const std::string& getEPPMessageOut() const {
    return message_out;
  }
  virtual unsigned getResult() const {
    return result;
  }
  virtual std::string getResultStatus() const {
    return (result < 2000 && result) ? "OK" : "FAILED";
  }
  virtual TID getRegistrarId() const {
    return registrarId;
  }
  virtual const std::string& getRegistrarHandle() const {
    return registrarHandle;
  }
  virtual const std::string& getHandle() const {
    return handle;
  }
};


/*
 * TEMP: Have to use own COMPARE_CLASS_IMPL because EPPAction class
 * is in same namespace as Registrar need prefix compare class name
 * - split file? split managers?
 */

#undef COMPARE_CLASS_IMPL
#define COMPARE_CLASS_IMPL(_object_type, _by_what)                        \
class EPPActionCompare##_by_what {                                        \
public:                                                                   \
  EPPActionCompare##_by_what(bool _asc) : asc_(_asc) { }                  \
  bool operator()(Register::CommonObject *_left,                          \
                  Register::CommonObject *_right) const {                 \
    _object_type *l_casted = dynamic_cast<_object_type *>(_left);         \
    _object_type *r_casted = dynamic_cast<_object_type *>(_right);        \
    if (l_casted == 0 || r_casted == 0) {                                 \
      /* this should never happen */                                      \
      throw std::bad_cast();                                              \
    }                                                                     \
                                                                          \
    bool result = l_casted->get##_by_what() <= r_casted->get##_by_what(); \
    return (asc_ && result) || (!asc_ && !result);  	                    \
}                                                                         \
private:                                                                  \
  bool asc_;                                                              \
};


COMPARE_CLASS_IMPL(EPPActionImpl, ServerTransactionId)
COMPARE_CLASS_IMPL(EPPActionImpl, ClientTransactionId)
COMPARE_CLASS_IMPL(EPPActionImpl, Type)
COMPARE_CLASS_IMPL(EPPActionImpl, Handle)
COMPARE_CLASS_IMPL(EPPActionImpl, RegistrarHandle)
COMPARE_CLASS_IMPL(EPPActionImpl, StartTime)
COMPARE_CLASS_IMPL(EPPActionImpl, Result)

#undef COMPARE_CLASS_IMPL

/// List of EPPAction objects
class EPPActionListImpl : public Register::CommonListImpl,
                          virtual public EPPActionList {
  TID id;
  TID sessionId;
  TID registrarId;
  std::string registrarHandle;
  time_period period;
  unsigned typeId;
  std::string type;
  unsigned returnCodeId;
  std::string clTRID;
  std::string svTRID;
  std::string handle;
  std::string xml;
  EPPActionResultFilter result;
  bool partialLoad;
public:
  EPPActionListImpl(DB *_db) : CommonListImpl(_db),
                               id(0), 
                               sessionId(0), 
                               registrarId(0), 
                               period(ptime(neg_infin), ptime(pos_infin)), 
                               typeId(0),
                               returnCodeId(0), 
                               result(EARF_ALL),  
                               partialLoad(false) {
  }
  ~EPPActionListImpl() {
    clear();
  }
  void setIdFilter(TID _id) {
    id = _id;
  }
  void setSessionFilter(TID _sessionId) {
    sessionId = _sessionId;
  }
  void setRegistrarFilter(TID _registrarId) {
    registrarId = _registrarId;
  }
  void setRegistrarHandleFilter(const std::string& _registrarHandle) {
    registrarHandle = _registrarHandle;
  }
  void setTimePeriodFilter(const time_period& _period) {
    period = _period;
  }
  virtual void setTypeFilter(unsigned _typeId) {
    typeId = _typeId;
  }
  virtual void setReturnCodeFilter(unsigned _returnCodeId) {
    returnCodeId = _returnCodeId;
  }
  virtual void setResultFilter(EPPActionResultFilter _result) {
    result = _result;
  }
  virtual void setHandleFilter(const std::string& _handle) {
    handle = _handle;
  }
  virtual void setXMLFilter(const std::string& _xml) {
    xml = _xml;
  }
  virtual void setTextTypeFilter(const std::string& _textType) {
    type = _textType;
  }
  virtual void setClTRIDFilter(const std::string& _clTRID) {
    clTRID = _clTRID;
  }
  virtual void setSvTRIDFilter(const std::string& _svTRID) {
    svTRID = _svTRID;
  }
#define DB_NULL_INT(i,j) \
  (db->IsNotNull(i,j)) ? atoi(db->GetFieldValue(i,j)) : 0
#define DB_NULL_STR(i,j) \
  (db->IsNotNull(i,j)) ? db->GetFieldValue(i,j) : ""
  virtual void reload()
  {
	try
	{
		Database::Connection conn = Database::Manager::acquire();

		clear();
		std::ostringstream sql;
		sql << "SELECT a.id,a.clientid,a.action,ea.status,a.startdate,"
			<< "a.servertrid,a.clienttrid, a.response,r.id,r.handle,MIN(al.value) ";
		if (partialLoad)
		  sql << ",'','' ";
		else
		  sql << ",ax.xml,ax.xml_out ";
		sql << "FROM action a "
			<< "JOIN enum_action ea ON (a.action=ea.id) "
			<< "JOIN enum_error er ON (a.response=er.id) "
			<< "JOIN login l ON (l.id=a.clientid) "
			<< "JOIN registrar r ON (r.id=l.registrarid) "
			<< "LEFT JOIN action_elements al ON (a.id=al.actionid) ";
		if (!partialLoad)
		  sql << "LEFT JOIN action_xml ax ON (a.id=ax.actionid) ";
		sql << "WHERE 1=1 ";
		if (id) sql << "AND " << "a.id" << "=" << id << " ";
		if (registrarId) sql << "AND " << "r.id" << "=" << registrarId << " ";

		if (!registrarHandle.empty())
		{
		    if ((registrarHandle.find('*') != std::string::npos ||
		              registrarHandle.find('?') != std::string::npos))
		      sql << "AND "
		       << "r.handle" << " ILIKE TRANSLATE('" << conn.escape(registrarHandle) << "','*?','%_') ";
		    else sql << "AND " << (0?"UPPER(":"") << "r.handle" << (0?")":"")
		           << "=" << (0?"UPPER(":"")
		           << "'" << conn.escape(registrarHandle) << "'" <<  (0?")":"") << " ";
		}

		if (!period.begin().is_special())
		     sql << "AND " << "a.startdate" << ">='"
		       <<  to_iso_extended_string(period.begin())
		       << "' ";
		  if (!period.end().is_special())
		     sql << "AND " << "a.startdate" << "<='"
		       <<  to_iso_extended_string(period.end())
		       << "' ";

		  if (!type.empty())
		  {
		      if ((type.find('*') != std::string::npos ||
		                type.find('?') != std::string::npos))
		        sql << "AND "
		         << "ea.status" << " ILIKE TRANSLATE('" << conn.escape(type) << "','*?','%_') ";
		      else sql << "AND " << (0?"UPPER(":"") << "ea.status" << (0?")":"")
		             << "=" << (0?"UPPER(":"")
		             << "'" << conn.escape(type) << "'" <<  (0?")":"") << " ";
		  }

		  if (returnCodeId) sql << "AND " << "a.response" << "=" << returnCodeId << " ";

		  if (!clTRID.empty())
		  {
		      if ((clTRID.find('*') != std::string::npos ||
		                clTRID.find('?') != std::string::npos))
		        sql << "AND "
		         << "a.clienttrid" << " ILIKE TRANSLATE('" << conn.escape(clTRID) << "','*?','%_') ";
		      else sql << "AND " << (0?"UPPER(":"") << "a.clienttrid" << (0?")":"")
		             << "=" << (0?"UPPER(":"")
		             << "'" << conn.escape(clTRID) << "'" <<  (0?")":"") << " ";
		  }

		  if (!svTRID.empty())
		  {
		      if ((svTRID.find('*') != std::string::npos ||
		                svTRID.find('?') != std::string::npos))
		        sql << "AND "
		         << "a.servertrid" << " ILIKE TRANSLATE('" << conn.escape(svTRID) << "','*?','%_') ";
		      else sql << "AND " << (0?"UPPER(":"") << "a.servertrid" << (0?")":"")
		             << "=" << (0?"UPPER(":"")
		             << "'" << conn.escape(svTRID) << "'" <<  (0?")":"") << " ";
		  }

		  /// TODO - handle has to have special data column

		  if (!handle.empty())
		  {
		      if ((handle.find('*') != std::string::npos ||
		                handle.find('?') != std::string::npos))
		        sql << "AND "
		         << "al.value" << " ILIKE TRANSLATE('" << conn.escape(handle) << "','*?','%_') ";
		      else sql << "AND " << (0?"UPPER(":"") << "al.value" << (0?")":"")
		             << "=" << (0?"UPPER(":"")
		             << "'" << conn.escape(handle) << "'" <<  (0?")":"") << " ";
		  }

		if (result != EARF_ALL)
		  sql << "AND (a.response "
			  << (result == EARF_OK ? "<" : " IS NULL OR a.response >=")
			  << " 2000) ";
		sql << "GROUP BY a.id,a.clientid,a.action,ea.status,a.startdate,"
			<< "a.servertrid,a.clienttrid,a.response,r.handle ";
		if (!partialLoad)
		  sql << ",ax.xml,ax.xml_out ";
		sql << "LIMIT 1000";

		Database::Result res = conn.exec(sql.str());

		for (unsigned i=0; i < static_cast<unsigned>(res.size()); i++)
		{
		  data_.push_back(new EPPActionImpl(
			res[i][0]
			,res[i][1]
			,res[i][2]
			,res[i][3]
			,res[i][4]
			,res[i][5]
			,res[i][6]
			,res[i][7]
			,res[i][8]
			,res[i][9]
			,res[i][10]
			,res[i][11]
			,res[i][12]

		  ));
		}

	}//try
	catch (...)
	{
	 LOGGER(PACKAGE).error("reload: an error has occured");
	 throw SQL_ERROR();
	}//catch (...)
  }//reload

  virtual EPPAction* get(unsigned _idx) const {
    try {
      EPPAction *action = dynamic_cast<EPPAction* >(data_.at(_idx));
      if (action) 
        return action;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    } 
  }
  
  virtual void clearFilter() {
    id = 0;
    sessionId = 0;
    registrarId = 0;
    registrarHandle = "";
    period = time_period(ptime(neg_infin), ptime(pos_infin));
    typeId = 0;
    type = "";
    returnCodeId = 0;
    clTRID = "";
    svTRID = "";
    handle = "";
    xml = "";
    result = EARF_ALL;
    partialLoad = false;
  }
  virtual void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] EPPActionListImpl::reload()");
    clear();
    uf.clearQueries();

    // TEMP: should be cached for quicker
    std::map<Database::ID, std::string> registrars_table;

    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Union::iterator fit = uf.begin();
    for (; fit != uf.end(); ++fit) {
      Database::Filters::EppAction *eaf =
          dynamic_cast<Database::Filters::EppAction*>(*fit);
      if (!eaf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect("id", eaf->joinActionTable());
      uf.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    id_query.limit(load_limit_);
    uf.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery("tmp_eppaction_filter_result", id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%")
        % "tmp_eppaction_filter_result" % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select()
        << "t_1.id, t_1.clientid, t_1.action, t_2.status, t_1.startdate, "
        << "t_1.servertrid, t_1.clienttrid, t_1.response, "
        << "t_4.registrarid, MIN(t_5.value)";
    if (!partialLoad) {
      object_info_query.select() << ", t_6.xml, t_6.xml_out";
    }
    else {
      object_info_query.select() << ", '', ''";
    }
    object_info_query.from() << "tmp_eppaction_filter_result tmp "
                             << "JOIN action t_1 ON(tmp.id = t_1.id) "
                             << "JOIN enum_action t_2 ON (t_1.action = t_2.id) "
                             << "LEFT JOIN enum_error t_3 ON (t_1.response = t_3.id) "
                             << "LEFT JOIN login t_4 ON (t_1.clientid = t_4.id) "
                             << "LEFT JOIN action_elements t_5 ON (t_1.id = t_5.actionid)";
    if (!partialLoad)
      object_info_query.from() << "LEFT JOIN action_xml t_6 ON (t_1.id = t_6.actionid) ";

    object_info_query.order_by() << "tmp.id";
    object_info_query.group_by() << "t_1.id, t_1.clientid, t_1.action, "
                                 << "tmp.id, t_2.status, t_1.startdate, t_1.servertrid, t_1.clienttrid, "
                                 << "t_1.response, t_4.registrarid";
    if (!partialLoad) {
      object_info_query.group_by() << ", t_6.xml, t_6.xml_out";
    }

    try {
      Database::Connection conn = Database::Manager::acquire();

      Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string("tmp_eppaction_filter_result") + "')");
      conn.exec(create_tmp_table);
      conn.exec(tmp_table_query);

      // TEMP: should be cached somewhere
      Database::Query registrars_query("SELECT id, handle FROM registrar");
      Database::Result r_registrars = conn.exec(registrars_query);
      Database::Result::Iterator it = r_registrars.begin();
      for (; it != r_registrars.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID      id = *col;
        std::string   handle = *(++col);
        registrars_table[id] = handle;
      }

      Database::Result r_info = conn.exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID       aid              = *col;
        Database::ID       clid             = *(++col);
        unsigned           type             = *(++col);
        std::string        type_name        = *(++col);
        Database::DateTime start_time       = *(++col);
        std::string        server_trid      = *(++col);
        std::string        client_trid      = *(++col);
        unsigned           result           = *(++col);
        Database::ID       registrar_id     = *(++col);
        std::string        registrar_handle = registrars_table[registrar_id];
        std::string        object_handle    = *(++col);
        std::string        message          = *(++col);
        std::string        message_out      = *(++col);

        data_.push_back(
            new EPPActionImpl(
                aid,
                clid,
                type,
                type_name,
                start_time,
                server_trid,
                client_trid,
                result,
                registrar_id,
                registrar_handle,
                object_handle,
                message,
                message_out
            ));
      }
      /* checks if row number result load limit is active and set flag */ 
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
  }
  virtual void setPartialLoad(bool _partialLoad) {
    partialLoad = _partialLoad;
  }

  virtual void sort(EPPActionMemberType member, bool asc) {
    switch (member) {
      case MT_SVTRID:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareServerTransactionId(asc));
        break;
      case MT_CLTRID:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareClientTransactionId(asc));
        break;
      case MT_TYPE:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareType(asc));
        break;
      case MT_OBJECT_HANDLE:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareHandle(asc));
        break;
      case MT_REGISTRAR_HANDLE:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareRegistrarHandle(asc));
        break;
      case MT_TIME:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareStartTime(asc));
        break;
      case MT_RESULT:
        stable_sort(data_.begin(), data_.end(), EPPActionCompareResult(asc));
        break;
    }
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
    /* dummy implementation */
  }

  virtual const char* getTempTableName() const {
    return ""; /* dummy implementation */
  }

};

class ManagerImpl : virtual public Manager
{ DB * db_;
  //RegistrarListImpl rl;
  EPPActionListImpl eal;
  std::vector<EPPActionType> actionTypes;
public:
  ManagerImpl(DB* db)
  :db_(db)//,rl()
  , eal(db)
  {
      try
      {
    	Database::Connection conn = Database::Manager::acquire();

		Database::Result res = conn.exec("SELECT * FROM enum_action");

		unsigned rows = res.size();

		if((rows > 0)&&(res[0].size() > 0))
		{
		    actionTypes.clear();
		    for (unsigned i = 0; i < (unsigned)rows; ++i)
		    {
		      EPPActionType action;
		      action.id = res[i][0];
		      action.name = std::string(res[i][1]);
		      actionTypes.push_back(action);
		    }
		}
		else
			throw SQL_ERROR();
      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("ManagerImpl: an error has occured");
          throw SQL_ERROR();
      }//catch (...)
  }

/*
  virtual RegistrarList *getList()
  {
    return &rl;
  }
*/
  virtual EPPActionList *getEPPActionList()
  {
    return &eal;
  }
  virtual EPPActionList *createEPPActionList()
  {
    return new EPPActionListImpl(db_);
  }
  
  virtual unsigned getEPPActionTypeCount()
  {
    return actionTypes.size();
  }

  virtual const EPPActionType& getEPPActionTypeByIdx(unsigned idx) const
      throw (NOT_FOUND)
  {
    if (idx >= actionTypes.size())
      throw NOT_FOUND();
    return actionTypes[idx];
  }

  virtual bool checkHandle(const std::string handle) const
	  throw (SQL_ERROR)
  {
      try
      {
    	Database::Connection conn = Database::Manager::acquire();

		if (!boost::regex_match(handle, boost::regex("[rR][eE][gG]-.*")))
		  return false;
		std::stringstream sql;
		sql << "SELECT COUNT(*) FROM registrar " << "WHERE UPPER(handle)=UPPER('"
			<< handle << "')";

		Database::Result res = conn.exec(sql.str());
		if((res.size() > 0)&&(res[0].size() > 0))
		{
			unsigned count = res[0][0];
			bool ret = count;
			return ret;
		}
		else
			throw SQL_ERROR();
      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("checkHandle: an error has occured");
          throw SQL_ERROR();
      }//catch (...)
  }

  virtual void addRegistrarAcl(
          const std::string &registrarHandle,
          const std::string &cert,
          const std::string &pass)
      throw (SQL_ERROR)
  {
      try
      {
    	  Database::Connection conn = Database::Manager::acquire();

    	  std::stringstream sql;
		  sql << "INSERT INTO registraracl (registrarid, cert, password) "
			  << "SELECT r.id, '" << conn.escape(cert) << "','"
			  << conn.escape(pass) << "' FROM registrar r "
			  << "WHERE r.handle='" << conn.escape(registrarHandle) << "'";

		  Database::Transaction tx(conn);
		  conn.exec(sql.str());
		  tx.commit();
      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("addRegistrarAcl: an error has occured");
          throw SQL_ERROR();
      }//catch (...)
  }//addRegistrarAcl

  virtual Registrar *createRegistrar()
  {
      return dynamic_cast<RegistrarImpl *>(new RegistrarImpl);
  }

  virtual void addRegistrarZone(
          const std::string& registrarHandle,
          const std::string zone,
          const Database::Date &fromDate,
          const Database::Date &toDate) throw (SQL_ERROR)
  {
      try
      {
      	Database::Connection conn = Database::Manager::acquire();

		std::string fromStr;
		std::string toStr;

		if (fromDate != Database::Date())
		{
			fromStr = "'" + fromDate.to_string() + "'";
		}
		else
		{
			fromStr = "CURRENT_DATE";
		}
		if (toDate != Database::Date())
		{
			toStr = "'" + toDate.to_string() + "'";
		}
		else
		{
			toStr = "NULL";
		}
		std::stringstream sql;
		sql << "INSERT INTO registrarinvoice (registrarid,zone,fromdate,lastdate) "
			<< "SELECT r.id,z.id, " << fromStr << "," << toStr << " FROM ("
			<< "SELECT id FROM registrar WHERE handle='" << registrarHandle
			<< "') r " << "JOIN (SELECT id FROM zone WHERE fqdn='" << zone
			<< "') z ON (1=1) " << "LEFT JOIN registrarinvoice ri ON "
			<< "(ri.registrarid=r.id AND ri.zone=z.id) " << "WHERE ri.id ISNULL";

		Database::Transaction tx(conn);
		conn.exec(sql.str());
		tx.commit();
      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("addRegistrarZone: an error has occured");
          throw SQL_ERROR();
      }//catch (...)
  }//addRegistrarZone

  virtual void updateRegistrarZone(
          const TID& id,
          const Database::Date &fromDate,
          const Database::Date &toDate) throw (SQL_ERROR)
  {
      try
      {
      	Database::Connection conn = Database::Manager::acquire();

		std::string fromStr;
		std::string toStr;

		if (fromDate != Database::Date())
		{
			fromStr = "'" + fromDate.to_string() + "'";
		}
		else
		{
			fromStr = "CURRENT_DATE";
		}
		if (toDate != Database::Date())
		{
			toStr = "'" + toDate.to_string() + "'";
		}
		else
		{
			toStr = "NULL";
		}
		std::stringstream sql;

		sql << "UPDATE registrarinvoice SET fromdate = date " << fromStr
			<< ",todate =  date " << toStr
			<< " WHERE id = " << id << ";";

		Database::Transaction tx(conn);
		conn.exec(sql.str());
		tx.commit();
      }//try
      catch (...)
      {
          LOGGER(PACKAGE).error("updateRegistrarZone: an error has occured");
          throw SQL_ERROR();
      }//catch (...)
  }//updateRegistrarZone

  ///list factory
    virtual RegistrarListPtr createList()
    {
        return RegistrarListPtr(new RegistrarListImpl());
    }

    ///registrar instance factory
    virtual RegistrarPtr getRegistrarByHandle(const std::string& handle)
    {
        RegistrarListPtr registrarlist ( createList());
        registrarlist->clearFilter();
        registrarlist->setHandleFilter(handle);
        registrarlist->reload();
        if (registrarlist->size() != 1)
        {
            return RegistrarPtr(0);
        }
        return RegistrarPtr(registrarlist->getAndRelease(0));
    }//getRegistrarByHandle
}; // class ManagerImpl

unsigned long long Manager::RegistrarZoneAccess::max_id(ColIndex idx, Database::Result& result)
{
    TRACE("[CALL] Manager::RegistrarZoneAccess::max_id");
    unsigned long long ret =0;
    for (unsigned i = 0; i < result.size() ; ++i)
        if((result[i].size() > idx)
                && (static_cast<unsigned>(result[i][idx]) > ret))
            ret = result[i][idx];
    LOGGER(PACKAGE).debug(boost::format
            ("[CALL] Manager::RegistrarZoneAccess::max_id: %1%  result.size(): %2%")
                % ret % result.size());
    return ret;
}//max_id

/// Look if registrar have currently access to zone by id
 bool Manager::RegistrarZoneAccess::isInZone(unsigned long long registrar_id,unsigned long long zone_id)
 {
     bool ret = false;
     if((registrar_id <= max_registrar_id)
             && (zone_id <= max_zone_id))
                ret = flag.at(registrar_id).at(zone_id);
    return ret;
 }//isInZone

void Manager::RegistrarZoneAccess::reload()
{
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        std::stringstream sql;
        sql <<  "select registrarid, zoneid, isinzone"
                " from (select r.id as registrarid, z.id as zoneid"
                " , (select count(*) from registrarinvoice ri"
                " where fromdate <= CURRENT_DATE"
                " and (todate >= CURRENT_DATE or todate is null)"
                " and ri.registrarid = r.id and ri.zone = z.id) as isinzone"
                " from registrar as r , zone as z) as rii where isinzone > 0";
        Database::Result res = conn.exec(sql.str());
        max_registrar_id = max_id(RegistrarCol, res);
        max_zone_id = max_id(ZoneCol, res);
        LOGGER(PACKAGE).debug(boost::format("Manager::RegistrarZoneAccess::reload "
                "res.size: %1% rza.max_registrar_id: %2% max_zone_id: %3% ")
                % res.size()
                % max_registrar_id
                % max_zone_id
                );
        flag = RegistrarZoneAccess::RegistrarZoneAccessArray (max_registrar_id + 1
                ,RegistrarZoneAccess::RegistrarZoneAccessRow(max_zone_id + 1,false));
        for (unsigned i = 0; i < res.size() ; ++i)
        {
            LOGGER(PACKAGE).debug(boost::format
                    ("[CALL] Manager::RegistrarZoneAccess::reload() for i: %1% ") % i );
             if(res[i].size() > IsInZone)
             {
                 LOGGER(PACKAGE).debug
                 (boost::format
                     ("[CALL] Manager::RegistrarZoneAccess::reload()"
                         " if size reg_id: %1% zone_id: %2% flag: %3% "
                     )
                     % static_cast<unsigned long long>(res[i][RegistrarCol])
                     % static_cast<unsigned long long>(res[i][ZoneCol])
                     % static_cast<bool>(res[i][IsInZone])
                 );
                 std::size_t regid = static_cast<std::size_t>(res[i][RegistrarCol]);
                 std::size_t zonid = static_cast<std::size_t>(res[i][ZoneCol]);
                 flag.at(regid).at(zonid) = static_cast<bool>(res[i][IsInZone]);
             }//if size
        }//for i
    }//try
    catch(const std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("Manager::RegistrarZoneAccess::reload exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("Manager::RegistrarZoneAccess::reload error");
    }
}//Manager::RegistrarZoneAccess::reload

Manager* Manager::create(DB * db)
{
  TRACE("[CALL] Register::Registrar::Manager::create()");
  return new ManagerImpl(db);
}

}
; // namespace Registrar
}
; // namespace Register

