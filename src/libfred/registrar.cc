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

// deprecated headers
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/util/log.hh"
#include "src/deprecated/model/model_filters.hh"
#include "src/libfred/model_registrar_acl.hh"
#include "src/libfred/model_registrar_group_map.hh"
#include "src/libfred/model_registrar.hh"

#include "src/libfred/registrar/group/cancel_registrar_group.hh"
#include "src/libfred/registrar/group/create_registrar_group.hh"
#include "src/libfred/registrar/group/get_registrar_groups.hh"
#include "src/libfred/registrar/group/update_registrar_group.hh"

#include "src/libfred/registrar/certification/create_registrar_certification.hh"
#include "src/libfred/registrar/certification/get_registrar_certifications.hh"
#include "src/libfred/registrar/certification/registrar_certification_type.hh"
#include "src/libfred/registrar/certification/update_registrar_certification.hh"

#include "src/libfred/common_impl.hh"
#include "src/libfred/credit.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/domain.hh"
#include "src/libfred/registrar.hh"
#include "src/libfred/zone.hh"
#include "src/libfred/invoicing/invoice.hh"

#include "src/util/log/context.hh"
#include "src/util/log/logger.hh"
#include "src/util/password_storage.hh"
#include "src/util/subprocess.hh"
#include "src/util/types/money.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


namespace LibFred {
namespace Registrar {

class RegistrarImpl;


class ACLImpl:public ACL, private ModelRegistrarAcl
{
private:
    boost::optional<unsigned long long> password_same_as_acl_id_;

public:
  ACLImpl()
      : ModelRegistrarAcl(),
        password_same_as_acl_id_()
  { }
  ACLImpl(TID _id,
          const std::string& _certificateMD5,
          const std::string& _plaintext_password)
      : ModelRegistrarAcl(),
        password_same_as_acl_id_()
  {
	  this->ModelRegistrarAcl::setId(_id);
	  this->ModelRegistrarAcl::setCert(_certificateMD5);
	  this->set_password(_plaintext_password);
  }
  TID getId()const
  {
      return this->ModelRegistrarAcl::getId();
  }
  void setId(TID _id)
  {
      this->ModelRegistrarAcl::setId(_id);
  }
  TID getRegistarId()const
  {
      return this->ModelRegistrarAcl::getRegistarId();
  }
  void setRegistrarId(const TID& _registrar_id)override
  {
      this->ModelRegistrarAcl::setRegistrarId(_registrar_id);
  }
  const std::string& getCertificateMD5()const override
  {
      return this->ModelRegistrarAcl::getCert();
  }
  void setCertificateMD5(const std::string& _certificateMD5)override
  {
      this->ModelRegistrarAcl::setCert(_certificateMD5);
  }
  void set_password(const std::string& _plaintext_password)override
  {
      const auto encrypted_password = ::PasswordStorage::encrypt_password_by_preferred_method(_plaintext_password);
      this->ModelRegistrarAcl::setPassword(encrypted_password.get_value());
  }
  void set_password_same_as_acl_id(const unsigned long long _acl_id) override
  {
      this->ModelRegistrarAcl::setPassword("");
      password_same_as_acl_id_ = _acl_id;
  }
  bool operator==(TID _id)const
  {
    return getId() == _id;
  }
  void save()
  {
    TRACE("[CALL] ACLImpl::save()");
	try
	{
		Database::Connection conn = Database::Manager::acquire();
		const TID id = this->getId();
		LOGGER(PACKAGE).debug(boost::format("ACLImpl::save id: %1% RegistrarId: %2%")
		% id % this->getRegistarId());

        if (id != 0)
        {
            if (!this->ModelRegistrarAcl::getPassword().empty())
            {
                conn.exec_params(
                    "UPDATE registraracl SET cert = $1::text, password = $2::text"
                    " WHERE registrarid = $3::bigint and id = $4::bigint",
                    Database::query_param_list
                        (this->ModelRegistrarAcl::getCert())
                        (this->ModelRegistrarAcl::getPassword())
                        (this->ModelRegistrarAcl::getRegistarId())
                        (id)
                );
            }
            else
            {
                conn.exec_params(
                    "UPDATE registraracl SET cert = $1::text"
                    " WHERE registrarid = $2::bigint and id = $3::bigint",
                    Database::query_param_list
                        (this->ModelRegistrarAcl::getCert())
                        (this->ModelRegistrarAcl::getRegistarId())
                        (id)
                );
            }
        }
        else
        {
            if (password_same_as_acl_id_ == boost::none)
            {
                Database::Result r_id = conn.exec_params(
                    "INSERT INTO registraracl (registrarid, cert, password) VALUES"
                    " ($1::bigint, $2::text, $3::text)"
                    " RETURNING id",
                    Database::query_param_list
                        (this->ModelRegistrarAcl::getRegistarId())
                        (this->ModelRegistrarAcl::getCert())
                        (this->ModelRegistrarAcl::getPassword())
                );
                if (r_id.size() == 1)
                {
                    this->setId(static_cast<unsigned long long>(r_id[0][0]));
                }
            }
            else
            {
                Database::Result r_id = conn.exec_params(
                    "INSERT INTO registraracl (registrarid, cert, password) VALUES"
                    " ($1::bigint, $2::text,"
                    " (SELECT password FROM registraracl WHERE id = $3 AND registrarid = $1))"
                    " RETURNING id",
                    Database::query_param_list
                        (this->ModelRegistrarAcl::getRegistarId())
                        (this->ModelRegistrarAcl::getCert())
                        (*password_same_as_acl_id_)
                );
                if (r_id.size() == 1)
                {
                    this->setId(static_cast<unsigned long long>(r_id[0][0]));
                }
            }
        }
	}
	catch (...)
	{
        LOGGER(PACKAGE).error("save: an error has occured");
        throw SQL_ERROR();
	}
  }
};

unsigned long addRegistrarZone(
          const std::string& registrarHandle,
          const std::string zone,
          const Database::Date &fromDate,
          const Database::Date &toDate)
  {
      try
      {

        LOGGER(PACKAGE).debug(
                boost::format("addRegistrarZone registrarHandle: %1% zone: %2% fromDate: %3% toDate: %4%")
                % registrarHandle % zone % fromDate.to_string() % toDate.to_string());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction trans(conn);

        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrarHandle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }

        Database::ID reg_id = res_reg[0][0];

        Database::Result res_zone = conn.exec_params(
                "SELECT id FROM zone WHERE fqdn=$1::text",
                Database::query_param_list(zone));
        if(res_zone.size() == 0) {
            throw std::runtime_error("Zone does not exist");
        }

        Database::ID zone_id = res_zone[0][0];

        Database::Result res = conn.exec_params(
               "INSERT INTO registrarinvoice (registrarid,zone,fromdate,todate) "
               "VALUES ($1::bigint, $2::bigint, $3::date, $4::date) RETURNING id",
                    Database::query_param_list(reg_id)
                                           (zone_id)
                                           (fromDate.is_special()
                                                ? Database::QueryParam(boost::posix_time::microsec_clock::local_time().date())
                                                : Database::QueryParam(fromDate.get()))
                                           (toDate.is_special()
                                                ? Database::QPNull
                                                : Database::QueryParam(toDate.get())));


        LibFred::Credit::init_new_registrar_credit(reg_id, zone_id);

        if((res.size() == 1) && (res[0].size() == 1))
        {
            trans.commit();
            return static_cast<unsigned long>(res[0][0]);
        }
        else
        {
            throw std::runtime_error("addRegistrarZone: invalid result size");
        }
      }//try
      catch (const std::exception& ex)
	  {
	  	LOGGER(PACKAGE).error(std::string("addRegistrarZone: ") + ex.what());
		throw;
	  }//catch (const std::exception& ex)
      catch (...)
      {
          LOGGER(PACKAGE).error("addRegistrarZone: an error has occured");
          throw;
      }//catch (...)
  }//addRegistrarZone


class RegistrarImpl : public LibFred::CommonObjectImplNew,
                      virtual public Registrar,
                      private ModelRegistrar
{
  typedef std::shared_ptr<ACLImpl> ACLImplPtr;
  typedef std::vector<ACLImplPtr> ACLList;
  typedef ACLList::iterator ACLListIter;

  Money credit; ///< DB: registrar.credit

  ACLList acl; ///< access control
  typedef std::map<Database::ID, Money> ZoneCreditMap;
  ZoneCreditMap zone_credit_map;

  typedef  std::shared_ptr<ZoneAccess> ZoneAccessPtr;
  typedef std::vector<ZoneAccessPtr> ZoneAccessList;
  typedef ZoneAccessList::iterator ZoneAccessListIter;
  ZoneAccessList actzones;

public:
  RegistrarImpl()
  : CommonObjectImplNew()
  , ModelRegistrar()
  , credit("0")

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
                Money _credit) :
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

  virtual Money getCredit() const {
    return credit;
  }
  
  virtual Money getCredit(Database::ID _zone_id) const
  {
      Logging::Context ctx("RegistrarImpl::getCredit");
      try
      {

      Money ret ("0");
      ZoneCreditMap::const_iterator it;
      it = zone_credit_map.find(_zone_id);
      if(it != zone_credit_map.end())
          ret = it->second;
    return ret;

      }//try
      catch(const std::exception& ex)
      {
          LOGGER(PACKAGE).debug(ex.what());
          throw;
      }
  }
  
  virtual void setCredit(Database::ID _zone_id, Money _credit)
  {
      Logging::Context ctx("RegistrarImpl::setCredit");
      try
      {
          zone_credit_map[_zone_id] = _credit;
          credit += _credit;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER(PACKAGE).debug(ex.what());
          throw;
      }
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
    std::shared_ptr<ACLImpl> newACL ( new ACLImpl());
    acl.push_back(newACL);
    return newACL.get();
  }

  virtual ZoneAccess* newZoneAccess()
  {
    std::shared_ptr<ZoneAccess> newZoneAccess ( new ZoneAccess());
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

  virtual void updateRegistrarZone(
          const TID& id,
          const Database::Date &fromDate,
          const Database::Date &toDate)
  {///expecting external transaction, no transaction inside
      try
      {
          LOGGER(PACKAGE).debug(boost::format("RegistrarImpl::updateRegistrarZone id: %1% fromDate: %2% toDate: %3%")
          % id % fromDate.to_string() % toDate.to_string() );

        Database::Connection conn = Database::Manager::acquire();

        std::string fromStr;
        std::string toStr;

        if (!fromDate.is_special())
        {
            fromStr = "'" + fromDate.to_string() + "'";
        }
        else
        {
            fromStr = "CURRENT_DATE";
        }

        if (!toDate.is_special())
        {
            toStr = "'" + toDate.to_string() + "'";
        }
        else
        {
            toStr = "NULL";
        }
        std::stringstream sql;

        sql << "UPDATE registrarinvoice SET fromdate = date (" << fromStr
            << "),todate =  date (" << toStr
            << ") WHERE id = " << id << ";";

        LOGGER(PACKAGE).debug(boost::format("RegistrarImpl::updateRegistrarZone Q: %1%")
        % sql.str() );


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
            LOGGER(PACKAGE).warning("isInZone by id: bad data in table registrarinvoice");
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


  virtual void save()
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

        std::set<unsigned long long> keep_acl_ids;
        for (unsigned j = 0; j < acl.size(); j++)
        {
            acl[j]->setRegistrarId(id);
            acl[j]->save();
            keep_acl_ids.insert(acl[j]->getId());
        }
        std::string delete_sql = "DELETE FROM registraracl WHERE registrarid = $1::bigint";
        Database::query_param_list delete_params(id);
        if (keep_acl_ids.size())
        {
            std::string id_list_sql;
            for (const auto id : keep_acl_ids)
            {
                if (!id_list_sql.empty())
                {
                    id_list_sql += ", ";
                }
                id_list_sql += "$" + delete_params.add(id) + "::bigint";
            }
            delete_sql += " AND id NOT IN (" + id_list_sql + ")";
        }
        conn.exec_params(delete_sql, delete_params);

		for (unsigned i = 0; i < actzones.size(); i++)
		{
		    if(actzones[i]->id)//use addRegistrarZone or updateRegistrarZone
                this->updateRegistrarZone (actzones[i]->id
                            ,actzones[i]->fromdate
                            ,actzones[i]->todate);
		    else
		    {
		        addRegistrarZone (getHandle()
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
    acl.push_back(std::make_shared<ACLImpl>(_id, certificateMD5, password));
  }

  void putZoneAccess(TID _id
          , std::string _name
          , Money _credit
          , Database::Date _fromdate
          , Database::Date _todate)
  {
      actzones.push_back(std::make_shared<ZoneAccess>(_id,_name,_credit,_fromdate,_todate));
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
    RegistrarZoneAccess* rzaptr_;
public:
  CompareCreditByZone(bool _asc, unsigned _zone_id
          , RegistrarZoneAccess* _rzaptr)
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

    Money lvalue = l_casted->getCredit(zone_id_);

    Money rvalue = r_casted->getCredit(zone_id_);

     if (rzaptr_)
     {
         if(rzaptr_->isInZone(l_casted->getId(),zone_id_) == false) lvalue = Money("-1");
         if(rzaptr_->isInZone(r_casted->getId(),zone_id_) == false) rvalue = Money("-1");
     }

    return (asc_ ? (lvalue < rvalue) : (lvalue > rvalue));
  }
};//class CompareCreditByZone



class RegistrarListImpl : public LibFred::CommonListImplNew,
                          public RegistrarList {
  
  std::string name;
  std::string handle;
  std::string xml;
  TID idFilter;
  std::string zoneFilter;
  long long ptr_idx_;//from CommonListImpl
public:
  RegistrarListImpl()
      : CommonListImplNew()
      , name("")
      , handle("")
      , xml("")
      , idFilter(0)
      , zoneFilter("")
      , ptr_idx_(-1)
  {}
  ~RegistrarListImpl() {
    clear();
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

    for ( ; ptr_idx_ < m_data_size
			; ptr_idx_++)
    {
        ret_ptr = dynamic_cast<RegistrarImpl* >(m_data.at(ptr_idx_));
        if (ret_ptr == 0) throw std::runtime_error("RegistrarImpl* findIDSequence: not a RegistrarImpl pointer");
        if (ret_ptr->getId() >=  _id) break;
    }

    if (ptr_idx_ == m_data_size
    		|| ret_ptr->getId() != _id)
    {
      TRACE(boost::format("find id sequence: not found in result set. (id=%1%, ptr_idx=%2%)")
                                          % _id % ptr_idx_);
      resetIDSequence();
      return NULL;
    }//if
    return ret_ptr;
  }//findIDSequence


  virtual void reload(Database::Filters::Union &uf) {
    TRACE("[CALL] RegistrarListImpl::reload()");
    clear();
    uf.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery info_query;
    std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
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
        Money credit ("0");

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
      credit_query.select() << "zone_id, registrar_id, credit";
      credit_query.from() << "registrar_credit";
      credit_query.order_by() << "registrar_id";

      resetIDSequence();
      Database::Result r_credit = conn.exec(credit_query);
      for (Database::Result::Iterator it = r_credit.begin(); it != r_credit.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID  zone_id      = *col;
        Database::ID  registrar_id = *(++col);
        Money credit (std::string(*(++col)));
        
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
      azone_query.select() <<   "ri.id as id, ri.registrarid as registrarid, z.fqdn as name, "
                                "case when ri.fromdate = mtd.max_fromdate then cr.credit else null end as credit  "
                                ", ri.fromdate as fromdate , ri.todate as todate ";
      azone_query.from() <<   "registrarinvoice ri "
                              "join zone z on ri.zone = z.id "
                              "left join (select zone_id, registrar_id, credit "
                                  "from registrar_credit "
                                  ") as cr on cr.zone_id = ri.zone "
                                  "and ri.registrarid = cr.registrar_id "
                              "join (select ri.registrarid as rid, ri.zone as zid "
                                  ",max(fromdate) as max_fromdate "
                                  "from registrarinvoice ri "
                                  "join zone z on ri.zone = z.id "
                                  "group by ri.registrarid, ri.zone ) as mtd "
                                  "on mtd.rid = ri.registrarid "
                                  "and mtd.zid = ri.zone ";
      azone_query.order_by() << "ri.registrarid,ri.zone,ri.fromdate desc,ri.todate desc ";

      Database::Result r_azone = conn.exec(azone_query);
      for (Database::Result::Iterator it = r_azone.begin(); it != r_azone.end(); ++it)
      {
        Database::Row::Iterator col = (*it).begin();

        Database::ID azone_id = *col;
        Database::ID registrar_id = *(++col);
        std::string zone_name = *(++col);
        Money credit (std::string( *(++col)));
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
        std::string message = ex.what();
        if (message.find(Database::Connection::getTimeoutString())
                != std::string::npos) {
            LOGGER(PACKAGE).info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
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

  virtual LibFred::Registrar::Registrar* findId(Database::ID id) const
  {
	  std::vector<LibFred::CommonObjectNew*>::const_iterator it = std::find_if(m_data.begin(),
	  m_data.end(),
	  CheckIdNew<LibFred::Registrar::Registrar>(id));

	  if (it != m_data.end())
	  {
		  LOGGER(PACKAGE).debug(boost::format("object list hit! object id=%1% found")
		  % id);
		  return dynamic_cast<LibFred::Registrar::Registrar*>(*it);
	  }
	  LOGGER(PACKAGE).debug(boost::format("object list miss! object id=%1% not found")
	  % id);
	  throw LibFred::NOT_FOUND();
  }

  virtual void sort(MemberType _member, bool _asc, unsigned _zone_id
          , RegistrarZoneAccess* rzaptr) {
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

  virtual const char* getTempTableName() const
  {
    return "";
  }
  
};//class RegistrarListImpl


/*
 * TEMP: Have to use own COMPARE_CLASS_IMPL because EPPAction class
 * is in same namespace as Registrar need prefix compare class name
 * - split file? split managers?
 */

#undef COMPARE_CLASS_IMPL

class ManagerImpl : virtual public Manager
{ DBSharedPtr db_;
  //RegistrarListImpl rl;

public:
  ManagerImpl(DBSharedPtr db)
  :db_(db)//,rl()
  
  {
  }

  virtual bool checkHandle(const std::string handle) const
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

  void addRegistrarAcl(
          const std::string& _registrar_handle,
          const std::string& _cert,
          const std::string& _plaintext_password)override
  {
      try
      {
    	  Database::Connection conn = Database::Manager::acquire();
    	  const std::string encrypted_password =
    	          ::PasswordStorage::encrypt_password_by_preferred_method(_plaintext_password).get_value();

    	  std::ostringstream sql;
		  sql << "INSERT INTO registraracl (registrarid,cert,password) "
			     "SELECT r.id,'" << conn.escape(_cert) << "',"
			            "'" << conn.escape(encrypted_password) << "' "
                 "FROM registrar r "
			     "WHERE r.handle='" << conn.escape(_registrar_handle) << "'";

		  Database::Transaction tx(conn);
		  conn.exec(sql.str());
		  tx.commit();
      }
      catch (...)
      {
          LOGGER(PACKAGE).error("addRegistrarAcl: an error has occured");
          throw SQL_ERROR();
      }
  }

  virtual Registrar::AutoPtr createRegistrar()
  {
      return Registrar::AutoPtr(static_cast<Registrar *>(new RegistrarImpl));
  }

  virtual void updateRegistrarZone(
          const TID& id,
          const Database::Date &fromDate,
          const Database::Date &toDate)
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

		sql << "UPDATE registrarinvoice SET fromdate = date (" << fromStr
			<< "),todate =  date (" << toStr
			<< ") WHERE id = " << id << ";";

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
    virtual RegistrarList::AutoPtr createList()
    {
        return RegistrarList::AutoPtr(new RegistrarListImpl());
    }

    ///registrar instance factory
    virtual Registrar::AutoPtr getRegistrarByHandle(const std::string& handle)
    {
        RegistrarList::AutoPtr registrarlist ( createList());

        Database::Filters::UnionPtr unionFilter = Database::Filters::CreateClearedUnionPtr();
        std::unique_ptr<Database::Filters::Registrar> r ( new Database::Filters::RegistrarImpl(true));
        r->addHandle().setValue(handle);
        unionFilter->addFilter( r.release() );
        registrarlist->reload(*unionFilter.get());

        if (registrarlist->size() != 1)
        {
            return Registrar::AutoPtr();
        }
        return Registrar::AutoPtr(registrarlist->getAndRelease(0));
    }//getRegistrarByHandle

    virtual unsigned long long getRegistrarByPayment(const std::string &varsymb,
                                                     const std::string &memo)
    {
        Database::Query query;
        query.buffer() << "SELECT id FROM registrar WHERE varsymb = "
                       << Database::Value(varsymb)
                       << " OR (length(trim(regex)) > 0 AND "
                       << Database::Value(memo) << " ~* trim(regex))";

        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec(query);
        if (result.size() == 1) {
            return static_cast<unsigned long long>(result[0][0]);
        }
        return 0;
    }

    ///create registrar group
    unsigned long long createRegistrarGroup(const std::string& _group_name) final override
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            const unsigned long long id = LibFred::Registrar::CreateRegistrarGroup(_group_name).exec(ctx);
            ctx.commit_transaction();
            return id;
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("createRegistrarGroup: an error has occurred");
            throw;
        }
    }

    ///cancel registrar group
    void cancelRegistrarGroup(const TID _group_id) final override
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            LibFred::Registrar::CancelRegistrarGroup(_group_id).exec(ctx);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("cancelRegistrarGroup: an error has occurred");
            throw;
        }
    }

    ///update registrar group
    void updateRegistrarGroup(const TID _group_id,
            const std::string& _group_name) final override
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            LibFred::Registrar::UpdateRegistrarGroup(_group_id, _group_name).exec(ctx);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("updateRegistrarGroup: an error has occurred");
            throw;
        }
    }

    GroupSeq getRegistrarGroups() final override
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            GroupSeq result;
            const std::vector<RegistrarGroup> registrar_groups = LibFred::Registrar::GetRegistrarGroups().exec(ctx);
            result.reserve(registrar_groups.size());
            for (const auto& it : registrar_groups)
            {
                GroupData gd;
                gd.id = it.id;
                gd.name = it.name;
                gd.cancelled = it.cancelled;
                result.push_back(std::move(gd));
            }
            return result;
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("getRegistrarGroups: an error has occurred");
            throw;
        }
    }

    ///create registrar certification
    unsigned long long createRegistrarCertification(const TID _registrar_id,
        const Database::Date& _valid_from,
        const Database::Date& _valid_until,
        const RegCertClass _classification,
        const TID _eval_file_id) final override
    {
        try
        {
            OperationContextCreator ctx;
            const unsigned long long id = CreateRegistrarCertification(_registrar_id,
                        _valid_from, _valid_until, _classification, _eval_file_id)
                    .exec(ctx);
            ctx.commit_transaction();
            return id;
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("createRegistrarCertification: an error has occurred");
            throw;
        }
    }

    ///create registrar certification by handle
    unsigned long long createRegistrarCertification(const std::string& _registrar_handle,
            const Database::Date& _valid_from,
            const Database::Date& _valid_until,
            const RegCertClass _classification,
            const TID _eval_file_id) final override
    {
        try
        {
            TID registrar_id = 0;

            OperationContextCreator ctx;
            const Database::Result res = ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle = UPPER($1:text)",
                    Database::query_param_list(_registrar_handle));
            if ((res.size() > 0) && (res[0].size() > 0))
            {
                registrar_id = res[0][0];
            }
            else
            {
                throw std::runtime_error("createRegistrarCertification: failed to find registrar in database");
            }

            return createRegistrarCertification(registrar_id, _valid_from, _valid_until, _classification, _eval_file_id);
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("createRegistrarCertification: an error has occurred");
            throw;
        }
    }


    ///shorten registrar certification
    void shortenRegistrarCertification(const TID _certification_id,
        const Database::Date& _valid_until) final override
    {
        try
        {
            OperationContextCreator ctx;
            UpdateRegistrarCertification(_certification_id, _valid_until).exec(ctx);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("shortenRegistrarCertification: an error has occurred");
            throw;
        }
    }

    ///update registrar certification
    void updateRegistrarCertification(const TID _certification_id,
        const RegCertClass _classification,
        const TID _eval_file_id) final override
    {
        try
        {
            OperationContextCreator ctx;//TODO checking namespace
            UpdateRegistrarCertification(_certification_id, _classification, _eval_file_id).exec(ctx);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("updateRegistrarCertification: an error has occurred");
            throw;
        }
    }

    ///get registrar certification
    CertificationSeq getRegistrarCertifications(const TID _registrar_id) final override
    {
        OperationContextCreator ctx;
        std::vector<CertificationData> result;
        const std::vector<RegistrarCertification> registrar_certifications = GetRegistrarCertifications(_registrar_id).exec(ctx);
        result.reserve(registrar_certifications.size());
        for (const auto& it : registrar_certifications)
        {
            CertificationData cd;
            cd.id = it.id;
            cd.valid_from = it.valid_from;
            cd.valid_until = it.valid_until;
            cd.classification = static_cast<RegCertClass>(it.classification);
            cd.eval_file_id = it.eval_file_id;
            result.push_back(std::move(cd));
        }
        return result;
    }

    ///create membership of registrar in group
    virtual unsigned long long createRegistrarGroupMembership( const TID& registrar_id
        , const TID& registrar_group_id
        , const Database::Date &member_from
        , const Database::Date &member_until)
    {
        try
        {
            ModelRegistrarGroupMap mrgm;
            mrgm.setRegistrarId(registrar_id);
            mrgm.setRegistrarGroupId(registrar_group_id);
            mrgm.setMemberFrom(member_from);
            if (member_until != Database::Date())
                mrgm.setMemberUntil(member_until);
            mrgm.insert();
            return mrgm.getId();
        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("createRegistrarGroupMembership: an error has occured");
            throw;
        }//catch (...)
    }

    ///create membership of registrar in group by name
    virtual unsigned long long createRegistrarGroupMembership( const std::string& registrar_handle
        , const std::string& registrar_group
        , const Database::Date &member_from
        , const Database::Date &member_until)
    {
        try
        {
            TID registrar_id = 0;
            TID registrar_group_id = 0;

            Database::Connection conn = Database::Manager::acquire();

            std::stringstream sql_registrar;
            sql_registrar << "SELECT id FROM registrar WHERE handle = UPPER('"
                    << conn.escape(registrar_handle) << "')";

            Database::Result res_registrar = conn.exec(sql_registrar.str());
            if((res_registrar.size() > 0)&&(res_registrar[0].size() > 0))
            {
                registrar_id = res_registrar[0][0];
            }
            else
                throw std::runtime_error(
                        "createRegistrarGroupMembership error: SELECT id "
                        "FROM registrar "
                        "returned empty result ");


            std::stringstream sql_group;
            sql_group << "SELECT id FROM registrar_group WHERE short_name = '"
                    << conn.escape(registrar_group) << "'";

            Database::Result res_group = conn.exec(sql_group.str());
            if((res_group.size() > 0)&&(res_group[0].size() > 0))
            {
                registrar_group_id = res_group[0][0];
            }
            else
                throw std::runtime_error(
                        "createRegistrarGroupMembership error: SELECT id "
                        "FROM registrar_group "
                        "returned empty result ");


            return createRegistrarGroupMembership( registrar_id
                    , registrar_group_id
                    , member_from
                    , member_until);
        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("createRegistrarGroupMembership: an error has occured");
            throw;
        }//catch (...)
    }


    ///update membership of registrar in group
    virtual void updateRegistrarGroupMembership( const TID& mebership_id
        , const TID& registrar_id
        , const TID& registrar_group_id
        , const Database::Date &member_from
        , const Database::Date &member_until)
    {
        try
        {
            ModelRegistrarGroupMap mrgm;
            mrgm.setId(mebership_id);
            mrgm.setRegistrarId(registrar_id);
            mrgm.setRegistrarGroupId(registrar_group_id);
            if (member_from != Database::Date())
                mrgm.setMemberFrom(member_from);
            if (member_until != Database::Date())
                mrgm.setMemberUntil(member_until);
            mrgm.update();
        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("updateRegistrarGroupMembership: an error has occured");
            throw;
        }//catch (...)
    }

    ///end of registrar membership in group
      virtual void endRegistrarGroupMembership(const TID& registrar_id
          , const TID& registrar_group_id)
      {
          try
          {
              Database::Connection conn = Database::Manager::acquire();
              Database::Transaction tx(conn);
              std::string lock_query
                  ("LOCK TABLE registrar_group_map IN ACCESS EXCLUSIVE MODE");
              conn.exec(lock_query);

              std::stringstream query;
              query << "update registrar_group_map set member_until = CURRENT_DATE"
              << " where id = (select id from registrar_group_map where"
              << " registrar_id = "
              << conn.escape(boost::lexical_cast<std::string>(registrar_id))
              << " and registrar_group_id = "
              << conn.escape(boost::lexical_cast<std::string>(registrar_group_id))
              << " order by member_from desc, id desc limit 1)"  ;

              conn.exec(query.str());

              tx.commit();
          }//try
          catch (...)
          {
              LOGGER(PACKAGE).error("endRegistrarGroupMembership: an error has occured");
              throw;
          }//catch (...)
      }


      ///get membership by registrar
      virtual MembershipByRegistrarSeq getMembershipByRegistrar( const TID& registrar_id)
      {
          Database::Connection conn = Database::Manager::acquire();

          MembershipByRegistrarSeq ret;//returned

          std::stringstream query;
          query << "select id, registrar_group_id, member_from, member_until "
              << "from registrar_group_map where registrar_id='"
              << conn.escape(boost::lexical_cast<std::string>(registrar_id))
              << "' order by member_from desc, id desc";

          Database::Result res = conn.exec(query.str());
          ret.reserve(res.size());
          for (Database::Result::Iterator it = res.begin(); it != res.end(); ++it)
          {
            Database::Row::Iterator col = (*it).begin();
            MembershipByRegistrar mbr;
            mbr.id = *col;
            mbr.group_id = *(++col);
            mbr.member_from = *(++col);
            mbr.member_until = *(++col);

            ret.push_back(mbr);
          }//for res
          return ret;
      }//getMembershipByRegistrar

      ///get membership by groups
      virtual MembershipByGroupSeq getMembershipByGroup( const TID& group_id)
      {
          Database::Connection conn = Database::Manager::acquire();

          MembershipByGroupSeq ret;//returned

          std::stringstream query;
          query << "select id, registrar_id, member_from, member_until "
              << "from registrar_group_map where registrar_group_id='"
              << conn.escape(boost::lexical_cast<std::string>(group_id))
              << "' order by member_from desc, id desc";

          Database::Result res = conn.exec(query.str());
          ret.reserve(res.size());
          for (Database::Result::Iterator it = res.begin(); it != res.end(); ++it)
          {
            Database::Row::Iterator col = (*it).begin();
            MembershipByGroup mbg;
            mbg.id = *col;
            mbg.registrar_id = *(++col);
            mbg.member_from = *(++col);
            mbg.member_until = *(++col);

            ret.push_back(mbg);
          }//for res
          return ret;
      }//getMembershipByGroup

      // this method relies that records in registrar_disconnect table don't overlap
      // and it doesn't take ownership of epp_cli pointer
      virtual bool blockRegistrar(const TID &registrar_id, const EppCorbaClient *epp_cli)
      {
          Database::Connection conn = Database::Manager::acquire();

          LOGGER(PACKAGE).info(boost::format("blockRegistrar(%1%) called") % registrar_id);

          Database::Transaction trans(conn);

          // thread safety - don't create 2 records
          conn.exec("LOCK TABLE registrar_disconnect IN SHARE MODE");

          // TODO maybe check registrar_id existence
          Database::Result res =
          conn.exec_params("SELECT id, blocked_from, blocked_to, "
                  "   date_trunc('month', blocked_from) = date_trunc('month', now()) AS blocked_this_month"
                  " FROM registrar_disconnect "
                  " WHERE registrarid = $1::integer"
                  " ORDER BY blocked_from ASC"
                  " LIMIT 1",
                  Database::query_param_list(registrar_id));

          if(res.size() > 0) {
              // there is a record, we have to deal with it

              if(res[0][2].isnull() ||  (res[0][2].operator ptime() > boost::posix_time::microsec_clock::universal_time()))  {
                  boost::format msg = boost::format (
                          "Registrar %1% is already blocked, from: %2% record id: %3%")
                                          % registrar_id % res[0][1] % res[0][0];
                  LOGGER(PACKAGE).notice(msg.str());
                  return false;
              }

              if((bool)res[0][3] == true) {
                  boost::format msg = boost::format (
                          "Registrar %1% has already been unblocked this month, from: %2%, to: %3%, record id: %4%")
                                  % registrar_id % res[0][1] % res[0][2] % res[0][0];
                  LOGGER(PACKAGE).notice(msg.str());
                  return false;
              }
          }

          conn.exec_params ("INSERT INTO registrar_disconnect (registrarid, blocked_to) VALUES "
                  "($1::integer, date_trunc('month', now()) + interval '1 month') ",
                  Database::query_param_list(registrar_id));

          trans.commit();

          LOGGER(PACKAGE).notice(boost::format("Registrar %1% blocked, destroying all his sessions ") % registrar_id);

          epp_cli->callDestroyAllRegistrarSessions(registrar_id);

          return true;
      }

      // this method relies that records in registrar_disconnect table don't overlap
      virtual void unblockRegistrar(const TID &registrar_id, const TID &request_id)
      {
          Database::Connection conn = Database::Manager::acquire();

          Database::Transaction trans(conn);

          Database::Result res = conn.exec_params("SELECT id, blocked_to"
                  " FROM registrar_disconnect"
                  " WHERE registrarid = $1::integer"
                  " ORDER BY blocked_from ASC"
                  " LIMIT 1"
                  " FOR UPDATE",
                  Database::query_param_list(registrar_id) );

          if(res.size() == 0) {
              boost::format msg = boost::format("Trying to unblock registrar %1% which is not blocked. ") % registrar_id;
              LOGGER(PACKAGE).error(msg);
              throw LibFred::NOT_BLOCKED();
          } else {
              Database::ID blocking_id = res[0][0];

              if(!res[0][1].isnull() && res[0][1].operator ptime() < boost::posix_time::microsec_clock::universal_time()) {
                  boost::format msg = boost::format(
                      "Trying to unblock registrar %1% which is not currently blocked: last blocking with ID %2% ended: %3%")
                          % registrar_id
                          % res[0][0]
                          % res[0][1].operator ptime();

                  LOGGER(PACKAGE).error(msg);
                  throw LibFred::NOT_BLOCKED();
              }

              if(request_id == 0) {
                  conn.exec_params("UPDATE registrar_disconnect SET blocked_to = now() WHERE id = $1::integer",
                          Database::query_param_list(blocking_id) );
                  trans.commit();

                  LOGGER(PACKAGE).notice(boost::format("Registrar %1% was unblocked (no request_id supplied)") % registrar_id);
              } else {
                  conn.exec_params("UPDATE registrar_disconnect SET blocked_to = now(), unblock_request_id = $1::bigint WHERE id = $2::integer",
                          Database::query_param_list(request_id)
                                                    (blocking_id) );
                  trans.commit();

                  LOGGER(PACKAGE).notice(boost::format("Registrar %1% was unblocked by request ID %2% ")
                      % registrar_id % request_id );
              }
          }

      }

      // throws when no registrars are present in DB
      void blockRegistrarsOverLimit(const EppCorbaClient *epp_client,
            Logger::LoggerClient *logger_client) {
        // from & to date for the calculation (in local time)
        boost::gregorian::date today = boost::gregorian::day_clock::local_day();
        boost::gregorian::date p_from(today.year(), today.month(), 1);
        boost::posix_time::ptime p_to(boost::posix_time::microsec_clock::local_time());

        // iterate registrars who have record in request_fee_registrar_parameter
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res_registrars = conn.exec(
                "SELECT r.handle, rp.request_price_limit, rp.email FROM registrar r"
                " JOIN request_fee_registrar_parameter rp"
                " ON r.id = rp.registrar_id");
        if (res_registrars.size() == 0) {
            LOGGER(PACKAGE).info("No registrars with request price limit found");
            return;
        }

        std::unique_ptr<RequestFeeDataMap> request_fee_data =
                getRequestFeeDataMap(logger_client, boost::posix_time::ptime(p_from), p_to, today);

        for (unsigned i = 0; i < res_registrars.size(); i++) {
            std::string reg_handle = res_registrars[i][0];
            Decimal reg_price_limit((std::string)res_registrars[i][1]);

            try {
                RequestFeeDataMap::iterator it = request_fee_data->find(reg_handle);
                if(it == request_fee_data->end()) {
                    // data for registrar not found in map, proceed to next
                    continue;
                }

                RequestFeeData &rfd = it->second;

                // reg_price_limit always has valid value,
                // while price for requests can be 0
                if(rfd.price > Decimal("0") && rfd.price > reg_price_limit) {
                   if (blockRegistrar(rfd.reg_id, epp_client)) {
                       boost::format msg = boost::format(
                               "Registrar %1% blocked: price limit %2% exceeded. Current price: %3%")
                               % reg_handle
                               % reg_price_limit
                               % rfd.price;

                       LOGGER(PACKAGE).warning(msg.str());
                   }
                }
            } catch (std::exception &ex) {
                LOGGER(PACKAGE).error(
                    boost::format("Exception caught while processing data for registrar %1%: %2%. ")
                        % reg_handle
                        % ex.what()
                        );
            } catch (...) {
                LOGGER(PACKAGE).error(
                    boost::format("Unknown exception caught while processing data for registrar %1%.")
                        % reg_handle);
            }

        }
      }


    virtual BlockedRegistrars getRegistrarsBlockedToday()
    {
        Database::Connection conn = Database::Manager::acquire();

        Database::Result blocked = conn.exec(
            "SELECT rd.registrarid, r.handle, (rd.blocked_from AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague') as blocked_from, rfrp.request_price_limit, rfrp.email, rfrp.telephone "
            "FROM registrar_disconnect rd "
            "LEFT JOIN request_fee_registrar_parameter rfrp ON rfrp.registrar_id = rd.registrarid "
            "JOIN registrar r ON r.id = rd.registrarid "
            "WHERE blocked_from >= date_trunc('day', now())"
        );

        BlockedRegistrars ret(new std::vector<BlockedReg>);
        if(blocked.size() == 0) {
            return ret;
        }

        ret->reserve(blocked.size());

        for(unsigned i=0; i<blocked.size(); ++i) {
            BlockedReg br;
            br.reg_id         = blocked[i][0]; //reg_id
            br.reg_handle     = std::string(blocked[i][1]); //reg_handle
            br.from_timestamp = std::string(blocked[i][2]); //from_timestamp
            br.price_limit    = Decimal(std::string(blocked[i][3])); //price_limit
            br.email          = std::string(blocked[i][4]); // email
            br.telephone      = std::string(blocked[i][5]);  // telephone

            ret->push_back(br);
        }

        return ret;
    }

    virtual bool hasRegistrarZoneAccess(const unsigned long long &_registrar_id,
                                        const unsigned long long &_zone_id)
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result za = conn.exec_params(
                "SELECT ri.id FROM registrar r"
                " JOIN registrarinvoice ri ON ri.registrarid = r.id"
                " WHERE ri.registrarid = $1::integer"
                " AND ri.zone = $2::integer"
                " AND ri.fromdate <= current_date"
                " AND (ri.todate >= current_date OR ri.todate is null)",
                Database::query_param_list(_registrar_id)(_zone_id));
        return za.size() != 0;
    }

    /// logs error and throws when registrar doesn't exist
    virtual void checkRegistrarExists( const TID & registrar_id)
    {
        Database::Connection conn = Database::Manager::acquire();

        Database::Result reg_id = conn.exec_params(
                "select id from registrar where id = $1::integer"
                , Database::query_param_list (registrar_id));
        if(reg_id.size() != 1)
        {
            LOGGER(PACKAGE).error(
                std::string("checkRegistrarExists: registrar with id: ")
                + boost::lexical_cast<std::string>(registrar_id)
                + " doesn't exist" );
            throw LibFred::NOT_FOUND();
        }
    }

    bool isRegistrarBlocked(Database::ID regId)
    {
        Database::Connection conn = Database::Manager::acquire();

        Result block_res
            = conn.exec_params(" SELECT id FROM registrar_disconnect"
                        " WHERE blocked_from <= now()"
                        " AND (now() < blocked_to OR blocked_to IS NULL)"
                        " AND registrarid = $1::integer", Database::query_param_list (regId)
                );

        if(block_res.size() > 0) {
            return true;
        } else {
            return false;
        }
    }

    std::unique_ptr<RequestFeeDataMap> getRequestFeeDataMap(
            Logger::LoggerClient *logger_client, boost::posix_time::ptime p_from,
            boost::posix_time::ptime p_to,
            boost::gregorian::date zone_access_date) {
        std::unique_ptr<RequestFeeDataMap> ret(new RequestFeeDataMap());

        std::string price_unit_request;
        unsigned int base_free_count;
        unsigned int per_domain_free_count;
        unsigned int zone_id;
        LibFred::Invoicing::getRequestFeeParams(&zone_id, &base_free_count,
                &per_domain_free_count);
        price_unit_request = LibFred::Invoicing::getRequestUnitPrice(zone_id);

        // get registrars who has access to configured zone
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res_registrars = conn.exec_params(
                "SELECT r.id, r.handle FROM registrar r"
                    " JOIN registrarinvoice ri ON ri.registrarid = r.id"
                    " WHERE ri.zone = $1::integer"
                    " AND ri.fromdate <= $2::date"
                    " AND (ri.todate >= $2::date OR ri.todate is null)",
                Database::query_param_list(zone_id)
                                            (zone_access_date)
            );

        if (res_registrars.size() == 0) {
            throw std::runtime_error("getRequestFeeDataMap: No registrars found");
        }

        // TODO why should we compute request count for all of them? But maybe it's not so much different
        // to think
        std::unique_ptr<LibFred::Logger::RequestCountInfo> request_counts =
                logger_client->getRequestCountUsers(p_from, p_to, "EPP");

        for (unsigned i = 0; i < res_registrars.size(); i++) {
            Database::ID reg_id = res_registrars[i][0];
            std::string reg_handle = res_registrars[i][1];

            // find request count for this registrar
            unsigned long long request_count = 0;
            LibFred::Logger::RequestCountInfo::iterator it = request_counts->find(
                    reg_handle);

            if (it == request_counts->end()) {
                LOGGER(PACKAGE).info(boost::format(
                        "No request count found for registrar %1%, skipping.")
                        % reg_handle);
                request_count = 0;
            } else {
                request_count = it->second;
            }

            // get domain count for registrar
            unsigned long long domain_count =
                    LibFred::Domain::getRegistrarDomainCount(reg_id,
                            boost::gregorian::date(p_from.date()), zone_id);

            // now count all the number for poll message
            unsigned long long total_free_count = std::max(
                    static_cast<unsigned long long> (base_free_count), domain_count
                            * per_domain_free_count);

            Money price("0");
            if (request_count > total_free_count) {
                Money count_diff(boost::lexical_cast<std::string>(request_count
                        - total_free_count));
                price = count_diff * Decimal(price_unit_request);
            }

            ret->insert(RequestFeeDataMap::value_type(reg_handle, RequestFeeData(
                    reg_handle, reg_id, request_count, total_free_count, price)));

            LOGGER(PACKAGE).info(boost::format("Request count data for registrar"
                " %1%, requests: %2%, total free: %3%, price: %4%") % reg_handle
                    % request_count % total_free_count % price);
        }

        return ret;
    }

}; // class ManagerImpl



unsigned long long RegistrarZoneAccess::max_id(ColIndex idx, Database::Result& result)
{
    TRACE("[CALL] RegistrarZoneAccess::max_id");
    unsigned long long ret =0;
    for (unsigned i = 0; i < result.size() ; ++i)
        if((result[i].size() > static_cast<unsigned long long>(idx))
                && (static_cast<unsigned>(result[i][idx]) > ret))
            ret = result[i][idx];
    LOGGER(PACKAGE).debug(boost::format
            ("[CALL] RegistrarZoneAccess::max_id: %1%  result.size(): %2%")
                % ret % result.size());
    return ret;
}//max_id

/// Look if registrar have currently access to zone by id
 bool RegistrarZoneAccess::isInZone(unsigned long long registrar_id,unsigned long long zone_id)
 {
     bool ret = false;
     if((registrar_id <= max_registrar_id)
             && (zone_id <= max_zone_id))
                ret = flag.at(registrar_id).at(zone_id);
     LOGGER(PACKAGE).debug(boost::format
             ("[CALL] RegistrarZoneAccess::isInZone() registrar_id: %1% zone_id: %2% ret: %3%")
                 % registrar_id % zone_id % ret);
    return ret;
 }//isInZone

void RegistrarZoneAccess::reload()
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
        LOGGER(PACKAGE).debug(boost::format("RegistrarZoneAccess::reload "
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
                    ("[CALL] RegistrarZoneAccess::reload() for i: %1% ") % i );
             if(res[i].size() > IsInZone)
             {
                 LOGGER(PACKAGE).debug
                 (boost::format
                     ("[CALL] RegistrarZoneAccess::reload()"
                         " if size reg_id: %1% zone_id: %2% flag: %3% "
                     )
                     % static_cast<unsigned long long>(res[i][RegistrarCol])
                     % static_cast<unsigned long long>(res[i][ZoneCol])
                     % (static_cast<unsigned long long>(res[i][IsInZone]) > 0)
                 );
                 std::size_t regid = static_cast<std::size_t>(res[i][RegistrarCol]);
                 std::size_t zonid = static_cast<std::size_t>(res[i][ZoneCol]);
                 flag.at(regid).at(zonid) = (static_cast<unsigned long long>(res[i][IsInZone]) > 0);
             }//if size
        }//for i
    }//try
    catch(const std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("RegistrarZoneAccess::reload exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("RegistrarZoneAccess::reload error");
    }
}//RegistrarZoneAccess::reload

Manager::AutoPtr Manager::create(DBSharedPtr db)
{
  TRACE("[CALL] LibFred::Registrar::Manager::create(db)");
  return Manager::AutoPtr(new ManagerImpl(db));
}



}
; // namespace Registrar
}
; // namespace LibFred

