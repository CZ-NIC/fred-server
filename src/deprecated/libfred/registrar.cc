/*
 * Copyright (C) 2008-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/util/log.hh"
#include "src/deprecated/model/model_filters.hh"

#include "libfred/registrar/group/cancel_registrar_group.hh"
#include "libfred/registrar/group/create_registrar_group.hh"
#include "libfred/registrar/group/get_registrar_groups.hh"
#include "libfred/registrar/group/update_registrar_group.hh"
#include "libfred/registrar/group/membership/create_registrar_group_membership.hh"
#include "libfred/registrar/group/membership/end_registrar_group_membership.hh"
#include "libfred/registrar/group/membership/info_group_membership_by_group.hh"
#include "libfred/registrar/group/membership/info_group_membership_by_registrar.hh"

#include "libfred/registrar/certification/create_registrar_certification.hh"
#include "libfred/registrar/certification/get_registrar_certifications.hh"
#include "libfred/registrar/certification/registrar_certification_type.hh"
#include "libfred/registrar/certification/update_registrar_certification.hh"

#include "src/deprecated/libfred/common_impl.hh"
#include "src/deprecated/libfred/credit.hh"
#include "libfred/opcontext.hh"
#include "src/deprecated/libfred/registrable_object/domain.hh"
#include "src/deprecated/libfred/registrar.hh"
#include "src/deprecated/libfred/zone.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

#include "util/log/context.hh"
#include "util/log/logger.hh"

#include "util/password_storage.hh"
#include "src/util/subprocess.hh"
#include "src/util/types/money.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


namespace LibFred {
namespace Registrar {

class RegistrarImpl;


class ACLImpl:public ACL
{
private:
    TID id_;
    TID registrar_id_;
    std::string certificateMD5_;
    std::string password_;
public:
  ACLImpl()
  { }
  ACLImpl(TID _id,
          const std::string& _certificateMD5,
          const std::string& _plaintext_password)
      : id_(_id),
        certificateMD5_(_certificateMD5)
  {
        this->set_password(_plaintext_password);
  }
  TID getId() const override final
  {
      return id_;
  }
  void setId(TID _id) override final
  {
      id_ = _id;
  }
  void setRegistrarId(TID _registrar_id) override
  {
      registrar_id_ = _registrar_id;
  }
  const std::string& getCertificateMD5()const override
  {
      return certificateMD5_;
  }
  void setCertificateMD5(const std::string& _certificateMD5)override
  {
      certificateMD5_ = _certificateMD5;
  }
  void set_password(const std::string& _plaintext_password)override
  {
      const auto encrypted_password = ::PasswordStorage::encrypt_password_by_preferred_method(_plaintext_password);
      password_ = encrypted_password.get_value();
  }
  bool operator==(TID _id)const
  {
    return getId() == _id;
  }
};

class RegistrarImpl : public LibFred::CommonObjectImplNew,
                      virtual public Registrar
{
private:
  typedef std::shared_ptr<ACLImpl> ACLImplPtr;
  typedef std::vector<ACLImplPtr> ACLList;
  typedef ACLList::iterator ACLListIter;

  Money credit; ///< DB: registrar.credit

  ACLList acl; ///< access control
  typedef std::map<Database::ID, Money> ZoneCreditMap;
  ZoneCreditMap zone_credit_map;

  std::vector<std::shared_ptr<RegistrarZone>> actzones;

  TID id_;
  std::string ico_;
  std::string dic_;
  std::string var_symb_;
  bool vat_;
  std::string handle_;
  std::string name_;
  std::string url_;
  std::string organization_;
  std::string street1_;
  std::string street2_;
  std::string street3_;
  std::string city_;
  std::string province_;
  std::string postal_code_;
  std::string country_;
  std::string telephone_;
  std::string fax_;
  std::string email_;
  bool system_;
public:
  RegistrarImpl()
      : CommonObjectImplNew(),
        credit("0"),
        id_(0),
        vat_(true),
        system_(false)
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
                const std::string& _postal_code,
                const std::string& _country,
                const std::string& _telephone,
                const std::string& _fax,
                const std::string& _email,
                bool _system,
                Money _credit)
      : CommonObjectImplNew(),
        credit(_credit),
        id_(_id),
        ico_(_ico),
        dic_(_dic),
        var_symb_(_var_symb),
        vat_(_vat),
        handle_(_handle),
        name_(_name),
        url_(_url),
        organization_(_organization),
        street1_(_street1),
        street2_(_street2),
        street3_(_street3),
        city_(_city),
        province_(_province),
        postal_code_(_postal_code),
        country_(_country),
        telephone_(_telephone),
        fax_(_fax),
        email_(_email),
        system_(_system)
  {
  }
  void clear() {
    acl.clear();
    actzones.clear();
  }
  ~RegistrarImpl() {}

  TID getId() const override
  {
     return id_;
  }
  void setId(unsigned long long _id) override
  {
	id_ = _id;
  }
  const std::string& getIco() const override
  {
    return ico_;
  }
  void setIco(const std::string& _ico) override
  {
	ico_ = _ico;
  }
  const std::string& getDic() const override
  {
    return dic_;
  }
  void setDic(const std::string& _dic) override
  {
	dic_ = _dic;
  }
  const std::string& getVarSymb() const override
  {
    return var_symb_;
  }
  void setVarSymb(const std::string& _var_symb) override
  {
	var_symb_ = _var_symb;
  }
  bool getVat() const override
  {
    return vat_;
  }
  void setVat(bool _vat) override
  {
	vat_ = _vat;
  }
  const std::string& getHandle() const override
  {
    return handle_;
  }
  void setHandle(const std::string& _handle) override
  {
	handle_ = _handle;
  }
  const std::string& getName() const override
  {
    return name_;
  }
  void setName(const std::string& _name) override
  {
	name_ = _name;
  }
  const std::string& getURL() const override
  {
    return url_;
  }
  void setURL(const std::string& _url) override
  {
	url_ = _url;
  }
  const std::string& getOrganization() const override
  {
    return organization_;
  }
  void setOrganization(const std::string& _organization) override
  {
	organization_ = _organization;
  }
  const std::string& getStreet1() const override
  {
    return street1_;
  }
  void setStreet1(const std::string& _street1) override
  {
	street1_ = _street1;
  }
  const std::string& getStreet2() const override
  {
    return street2_;
  }
  void setStreet2(const std::string& _street2) override
  {
	street2_ = _street2;
  }
  const std::string& getStreet3() const override
  {
    return street3_;
  }
  void setStreet3(const std::string& _street3) override
  {
	street3_ = _street3;
  }
  const std::string& getCity() const override
  {
	return city_;
  }
  void setCity(const std::string& _city) override
  {
	city_ = _city;
  }
  const std::string& getProvince() const override
  {
    return province_;
  }
  void setProvince(const std::string& _province) override
  {
	province_ = _province;
  }
  const std::string& getPostalCode() const override
  {
    return postal_code_;
  }
  void setPostalCode(const std::string& _postal_code) override
  {
	postal_code_ = _postal_code;
  }
  const std::string& getCountry() const override
  {
    return country_;
  }
  void setCountry(const std::string& _country) override
  {
	country_ = _country;
  }
  const std::string& getTelephone() const override
  {
    return telephone_;
  }
  void setTelephone(const std::string& _telephone) override
  {
	telephone_ = _telephone;
  }
  const std::string& getFax() const override
  {
    return fax_;
  }
  void setFax(const std::string& _fax) override
  {
	fax_ = _fax;
  }
  const std::string& getEmail() const override
  {
    return email_;
  }
  void setEmail(const std::string& _email) override
  {
	email_ = _email;
  }
  bool getSystem() const override
  {
    return system_;
  }
  void setSystem(bool _system) override
  {
	system_ = _system;
  }

  Money getCredit() const override
  {
    return credit;
  }

  Money getCredit(Database::ID _zone_id) const override
  {
      Logging::Context ctx("RegistrarImpl::getCredit");
      try
      {
          Money ret("0");
          ZoneCreditMap::const_iterator it;
          it = zone_credit_map.find(_zone_id);
          if (it != zone_credit_map.end())
          {
              ret = it->second;
          }
          return ret;
      }
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }

  void setCredit(Database::ID _zone_id, Money _credit) override
  {
      Logging::Context ctx("RegistrarImpl::setCredit");
      try
      {
          zone_credit_map[_zone_id] = _credit;
          credit += _credit;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }

  unsigned getACLSize() const override
  {
    return acl.size();
  }

  unsigned getRegistrarZoneSize() const override
  {
      return actzones.size();
  }

  ACL* getACL(unsigned idx) const override
  {
    return idx < acl.size() ? acl[idx].get() : nullptr;
  }
  RegistrarZone* getRegistrarZone(unsigned idx) const override
  {
    return idx < actzones.size() ? actzones[idx].get() : NULL;
  }

  ACL* newACL() override
  {
    std::shared_ptr<ACLImpl> newACL(new ACLImpl());
    acl.push_back(newACL);
    return newACL.get();
  }

  RegistrarZone* newRegistrarZone() override
  {
    std::shared_ptr<RegistrarZone> newRegistrarZone ( new RegistrarZone());
    actzones.push_back(newRegistrarZone);
    return newRegistrarZone.get();
  }

  void deleteACL(unsigned idx) override
  {
    if (idx < acl.size())
    {
      acl.erase(acl.begin() + idx);
    }
  }

  void clearACLList() override
  {
    acl.clear();
  }

  void clearRegistrarZoneList() override
  {
      actzones.clear();
  }

  /// Look if registrar have access to zone by zone id
  bool isInZone(unsigned id) const override
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
            LOGGER.error("isInZone by id: an error has occured");
            throw SQL_ERROR();
        }

        unsigned count = res[0][0];
        if (count > 1 )
        {
            LOGGER.warning("isInZone by id: bad data in table registrarinvoice");
            ret = true;
        }

        if (count == 1 ) ret = true;

      }//try
      catch (...)
      {
          LOGGER.error("isInZone by id: an error has occured");
          throw SQL_ERROR();
      }//catch (...)

      return ret;
  }

  /// Look if registrar have access to zone by zone fqdn
  bool isInZone(const std::string& zone_fqdn) const override
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
            LOGGER.error("isInZone by fqdn: an error has occured");
            throw SQL_ERROR();
        }

        unsigned count = res[0][0];
        if (count > 1 )
        {
            LOGGER.error("isInZone by fqdn: bad data in table registrarinvoice");
            ret = true;
        }

        if (count == 1 ) ret = true;

      }//try
      catch (...)
      {
          LOGGER.error("isInZone by fqdn: an error has occured");
          throw SQL_ERROR();
      }//catch (...)

      return ret;
  }

  void putACL(TID _id,
              const std::string& certificateMD5,
              const std::string& password)
  {
    acl.push_back(std::make_shared<ACLImpl>(_id, certificateMD5, password));
  }

  void putRegistrarZone(
          TID _id,
          std::string _name,
          Money _credit,
          Database::Date _fromdate,
          Database::Date _todate)
  {
      actzones.push_back(std::make_shared<RegistrarZone>(_id,_name,_credit,_fromdate,_todate));
  }

  bool hasId(TID _id) const
  {
    return getId() == _id;
  }
  void resetId()
  {
    setId(0);
    for (TID i = 0; i < acl.size(); ++i)
    {
      acl[i]->setId(0);
    }
    for (TID i = 0; i < actzones.size(); ++i)
    {
        actzones[i]->id = 0;
    }
  }
};

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
  CompareCreditByZone(bool _asc, unsigned _zone_id, RegistrarZoneAccess* _rzaptr)
      : asc_(_asc), zone_id_(_zone_id), rzaptr_(_rzaptr) { }
  bool operator()(CommonObjectNew* _left, CommonObjectNew* _right) const
  {
    RegistrarImpl* l_casted = dynamic_cast<RegistrarImpl*>(_left);
    RegistrarImpl* r_casted = dynamic_cast<RegistrarImpl*>(_right);
    if ((l_casted == nullptr) || (r_casted == nullptr))
    {
      /* this should never happen */
      throw std::bad_cast();
    }

    Money lvalue = l_casted->getCredit(zone_id_);

    Money rvalue = r_casted->getCredit(zone_id_);

    if (rzaptr_)
    {
        if (!rzaptr_->isInZone(l_casted->getId(), zone_id_))
        {
            lvalue = Money("-1");
        }
        if (!rzaptr_->isInZone(r_casted->getId(), zone_id_))
        {
            rvalue = Money("-1");
        }
    }

    return (asc_ ? (lvalue < rvalue) : (lvalue > rvalue));
  }
};

class RegistrarListImpl : public LibFred::CommonListImplNew,
                          public RegistrarList
{
private:
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


  void reload(Database::Filters::Union &uf) override
  {
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
      LOGGER.error("wrong filter passed for reload!");
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
          registrar_ptr->putRegistrarZone(azone_id, zone_name, credit, fromdate, todate);
        }

      }//for r_azone

      /* checks if row number result load limit is active and set flag */
      CommonListImplNew::reload();
    }//try
    catch (Database::Exception& ex)
    {
        std::string message = ex.what();
        if (message.find(Database::Connection::getTimeoutString())
                != std::string::npos) {
            LOGGER.info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER.error(boost::format("%1%") % ex.what());
            clear();
        }
    }
  }

  Registrar* get(unsigned _idx) const override
  {
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
  Registrar* getAndRelease(unsigned int _idx) override
  {
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

  LibFred::Registrar::Registrar* findId(Database::ID id) const override
  {
	  std::vector<LibFred::CommonObjectNew*>::const_iterator it = std::find_if(m_data.begin(),
	  m_data.end(),
	  CheckIdNew<LibFred::Registrar::Registrar>(id));

	  if (it != m_data.end())
	  {
		  LOGGER.debug(boost::format("object list hit! object id=%1% found")
		  % id);
		  return dynamic_cast<LibFred::Registrar::Registrar*>(*it);
	  }
	  LOGGER.debug(boost::format("object list miss! object id=%1% not found")
	  % id);
	  throw LibFred::NOT_FOUND();
  }

  void sort(MemberType _member, bool _asc, unsigned _zone_id, RegistrarZoneAccess* rzaptr) override
  {
    switch (_member)
    {
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
        stable_sort(m_data.begin(), m_data.end(), CompareCreditByZone(_asc , _zone_id, rzaptr));
        break;
    }
  }

  const char* getTempTableName() const override
  {
    return "";
  }
};

/*
 * TEMP: Have to use own COMPARE_CLASS_IMPL because EPPAction class
 * is in same namespace as Registrar need prefix compare class name
 * - split file? split managers?
 */

#undef COMPARE_CLASS_IMPL

class ManagerImpl : virtual public Manager
{
public:
  ManagerImpl(DBSharedPtr db) : db_(db) { }

  bool checkHandle(const std::string& handle) const override
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
          LOGGER.error("checkHandle: an error has occured");
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
          LOGGER.error("addRegistrarAcl: an error has occured");
          throw SQL_ERROR();
      }
  }

  Registrar::AutoPtr createRegistrar() override
  {
      return Registrar::AutoPtr(static_cast<Registrar *>(new RegistrarImpl));
  }

  void updateRegistrarZone(
          TID id,
          const Database::Date& fromDate,
          const Database::Date& toDate) override
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
      }
      catch (...)
      {
          LOGGER.error("updateRegistrarZone: an error has occured");
          throw SQL_ERROR();
      }
  }

  ///list factory
    RegistrarList::AutoPtr createList() override
    {
        return RegistrarList::AutoPtr(new RegistrarListImpl());
    }

    ///registrar instance factory
    Registrar::AutoPtr getRegistrarByHandle(const std::string& handle) override
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

    unsigned long long getRegistrarByPayment(const std::string& varsymb,
                                             const std::string& memo) override
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
            LOGGER.error("createRegistrarGroup: an error has occurred");
            throw;
        }
    }

    ///cancel registrar group
    void cancelRegistrarGroup( TID _group_id) final override
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            LibFred::Registrar::CancelRegistrarGroup(_group_id).exec(ctx);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER.error("cancelRegistrarGroup: an error has occurred");
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
            LOGGER.error("updateRegistrarGroup: an error has occurred");
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
            LOGGER.error("getRegistrarGroups: an error has occurred");
            throw;
        }
    }


    ///create registrar certification
    unsigned long long createRegistrarCertification(
            TID _registrar_id,
            const Database::Date& _valid_from,
            const Database::Date& _valid_until,
            RegCertClass _classification,
            TID _eval_file_id) final override
    {
        try
        {
            OperationContextCreator ctx;
            const unsigned long long id = CreateRegistrarCertification(
                    _registrar_id,
                    _valid_from,
                    _classification,
                    _eval_file_id)
                    .set_valid_until(_valid_until.get())
                    .exec(ctx);
            ctx.commit_transaction();
            return id;
        }
        catch (...)
        {
            LOGGER.warning("createRegistrarCertification: an error has occurred");
            throw;
        }
    }

    ///create registrar certification by handle
    unsigned long long createRegistrarCertification(
            const std::string& _registrar_handle,
            const Database::Date& _valid_from,
            const Database::Date& _valid_until,
            RegCertClass _classification,
            TID _eval_file_id) final override
    {
        try
        {
            TID registrar_id = 0;

            OperationContextCreator ctx;
            const Database::Result res = ctx.get_conn().exec_params(
                    // format-clang off
                    "SELECT id FROM registrar WHERE handle = UPPER($1::text)",
                    // format-clang on
                    Database::query_param_list(_registrar_handle));
            if ((res.size() > 0) && (res[0].size() > 0))
            {
                registrar_id = static_cast<unsigned long long>(res[0][0]);
            }
            else
            {
                throw std::runtime_error("createRegistrarCertification: failed to find registrar in database");
            }

            return createRegistrarCertification(registrar_id, _valid_from, _valid_until, _classification, _eval_file_id);
        }
        catch (...)
        {
            LOGGER.warning("createRegistrarCertification: an error has occurred");
            throw;
        }
    }


    ///shorten registrar certification
    void shortenRegistrarCertification(TID _certification_id, const Database::Date& _valid_until) final override
    {
        try
        {
            OperationContextCreator ctx;
            UpdateRegistrarCertification(_certification_id, _valid_until).exec(ctx);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER.warning("shortenRegistrarCertification: an error has occurred");
            throw;
        }
    }

    ///update registrar certification
    void updateRegistrarCertification(TID _certification_id, RegCertClass _classification, TID _eval_file_id) final override
    {
        try
        {
            OperationContextCreator ctx;
            UpdateRegistrarCertification(_certification_id, _classification, _eval_file_id).exec(ctx);
            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER.warning("updateRegistrarCertification: an error has occurred");
            throw;
        }
    }

    ///get registrar certification
    CertificationSeq getRegistrarCertifications(TID _registrar_id) final override
    {
        OperationContextCreator ctx;
        std::vector<CertificationData> result;
        const std::vector<RegistrarCertification> registrar_certifications =
                GetRegistrarCertifications(_registrar_id).exec(ctx);
        result.reserve(registrar_certifications.size());
        for (const auto& certification : registrar_certifications)
        {
            CertificationData cd;
            cd.id = certification.id;
            cd.valid_from = certification.valid_from;
            cd.valid_until = certification.valid_until;
            cd.classification = static_cast<RegCertClass>(certification.classification);
            cd.eval_file_id = certification.eval_file_id;
            result.push_back(std::move(cd));
        }
        return result;
    }

    ///create membership of registrar in group
    unsigned long long createRegistrarGroupMembership(
            TID _registrar_id,
            TID _registrar_group_id,
            const Database::Date& _member_from,
            const Database::Date& _member_until) final override
    {
        try
        {
            LibFred::OperationContextCreator ctx;
            const unsigned long long id = LibFred::Registrar::CreateRegistrarGroupMembership(
                    _registrar_id,
                    _registrar_group_id,
                    _member_from,
                    _member_until)
                .exec(ctx);
            ctx.commit_transaction();
            return id;
        }
        catch (...)
        {
            LOGGER.error("createRegistrarGroupMembership: an error has occurred");
            throw;
        }
    }

    ///create membership of registrar in group by name
    unsigned long long createRegistrarGroupMembership(
            const std::string& _registrar_handle,
            const std::string& _registrar_group,
            const Database::Date& _member_from,
            const Database::Date& _member_until) final override
    {
        try
        {
            TID registrar_id = 0;
            TID registrar_group_id = 0;

            OperationContextCreator ctx;
            const Database::Result res_registrar = ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle = UPPER($1::text)",
                    Database::query_param_list(_registrar_handle));
            if ((res_registrar.size() > 0) && (res_registrar[0].size() > 0))
            {
                registrar_id = res_registrar[0][0];
            }
            else
            {
                throw std::runtime_error("createRegistrarGroupMembership: failed to find registrar in database");
            }
            const Database::Result res_group = ctx.get_conn().exec_params(
                    "SELECT id FROM registrar_group WHERE short_name = $1::text",
                    Database::query_param_list(_registrar_group));
            if ((res_group.size() > 0) && (res_group[0].size() > 0))
            {
                registrar_group_id = res_group[0][0];
            }
            else
            {
                throw std::runtime_error("createRegistrarGroupMembership: failed to find registrar group in database");
            }
            return createRegistrarGroupMembership(registrar_id,
                    registrar_group_id,
                    _member_from,
                    _member_until);
        }
        catch (...)
        {
            LOGGER.error("createRegistrarGroupMembership: an error has occurred");
            throw;
        }
    }

    ///end of registrar membership in group
    void endRegistrarGroupMembership(TID _registrar_id, TID _registrar_group_id) final override
    {
      try
      {
          LibFred::OperationContextCreator ctx;
          LibFred::Registrar::EndRegistrarGroupMembership(_registrar_id, _registrar_group_id).exec(ctx);
          ctx.commit_transaction();
      }
      catch (...)
      {
          LOGGER.error("endRegistrarGroupMembership: an error has occurred");
          throw;
      }
    }

    ///get membership by registrar
    MembershipByRegistrarSeq getMembershipByRegistrar(TID registrar_id) final override
    {
      LibFred::OperationContextCreator ctx;
      const std::vector<GroupMembershipByRegistrar> info =
              LibFred::Registrar::InfoGroupMembershipByRegistrar(registrar_id).exec(ctx);
      MembershipByRegistrarSeq result;
      result.reserve(info.size());
      for (const auto& i: info)
      {
          MembershipByRegistrar mbr;
          mbr.id = i.membership_id;
          mbr.group_id = i.group_id;
          mbr.member_from = i.member_from;
          mbr.member_until = i.member_until;
          result.push_back(std::move(mbr));
      }
      return result;
    }

    ///get membership by groups
    MembershipByGroupSeq getMembershipByGroup(TID group_id) final override
    {
      LibFred::OperationContextCreator ctx;
      const std::vector<GroupMembershipByGroup> info =
              LibFred::Registrar::InfoGroupMembershipByGroup(group_id).exec(ctx);
      MembershipByGroupSeq result;
      result.reserve(info.size());
      for (const auto& i: info)
      {
          MembershipByGroup mbg;
          mbg.id = i.membership_id;
          mbg.registrar_id = i.registrar_id;
          mbg.member_from = i.member_from;
          mbg.member_until = i.member_until;
          result.push_back(std::move(mbg));
      }
      return result;
    }

      // this method relies that records in registrar_disconnect table don't overlap
      // and it doesn't take ownership of epp_cli pointer
      bool blockRegistrar(TID registrar_id, const EppCorbaClient *epp_cli) override
      {
          Database::Connection conn = Database::Manager::acquire();

          LOGGER.info(boost::format("blockRegistrar(%1%) called") % registrar_id);

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

          if (res.size() > 0)
          {
              // there is a record, we have to deal with it

              if(res[0][2].isnull() ||  (res[0][2].operator ptime() > boost::posix_time::microsec_clock::universal_time()))
              {
                  boost::format msg = boost::format (
                          "Registrar %1% is already blocked, from: %2% record id: %3%")
                                          % registrar_id % res[0][1] % res[0][0];
                  LOGGER.notice(msg.str());
                  return false;
              }

              if ((bool)res[0][3])
              {
                  boost::format msg = boost::format (
                          "Registrar %1% has already been unblocked this month, from: %2%, to: %3%, record id: %4%")
                                  % registrar_id % res[0][1] % res[0][2] % res[0][0];
                  LOGGER.notice(msg.str());
                  return false;
              }
          }

          conn.exec_params ("INSERT INTO registrar_disconnect (registrarid, blocked_to) VALUES "
                  "($1::integer, date_trunc('month', now()) + interval '1 month') ",
                  Database::query_param_list(registrar_id));

          trans.commit();

          LOGGER.notice(boost::format("Registrar %1% blocked, destroying all his sessions ") % registrar_id);

          epp_cli->callDestroyAllRegistrarSessions(registrar_id);

          return true;
      }

      // this method relies that records in registrar_disconnect table don't overlap
      void unblockRegistrar(TID registrar_id, TID request_id) override
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
              LOGGER.error(msg);
              throw LibFred::NOT_BLOCKED();
          } else {
              Database::ID blocking_id = res[0][0];

              if(!res[0][1].isnull() && res[0][1].operator ptime() < boost::posix_time::microsec_clock::universal_time()) {
                  boost::format msg = boost::format(
                      "Trying to unblock registrar %1% which is not currently blocked: last blocking with ID %2% ended: %3%")
                          % registrar_id
                          % res[0][0]
                          % res[0][1].operator ptime();

                  LOGGER.error(msg);
                  throw LibFred::NOT_BLOCKED();
              }

              if(request_id == 0) {
                  conn.exec_params("UPDATE registrar_disconnect SET blocked_to = now() WHERE id = $1::integer",
                          Database::query_param_list(blocking_id) );
                  trans.commit();

                  LOGGER.notice(boost::format("Registrar %1% was unblocked (no request_id supplied)") % registrar_id);
              } else {
                  conn.exec_params("UPDATE registrar_disconnect SET blocked_to = now(), unblock_request_id = $1::bigint WHERE id = $2::integer",
                          Database::query_param_list(request_id)
                                                    (blocking_id) );
                  trans.commit();

                  LOGGER.notice(boost::format("Registrar %1% was unblocked by request ID %2% ")
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
            LOGGER.info("No registrars with request price limit found");
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

                       LOGGER.warning(msg.str());
                   }
                }
            } catch (std::exception &ex) {
                LOGGER.error(
                    boost::format("Exception caught while processing data for registrar %1%: %2%. ")
                        % reg_handle
                        % ex.what()
                        );
            } catch (...) {
                LOGGER.error(
                    boost::format("Unknown exception caught while processing data for registrar %1%.")
                        % reg_handle);
            }

        }
      }

    BlockedRegistrars getRegistrarsBlockedToday() override
    {
        Database::Connection conn = Database::Manager::acquire();

        Database::Result blocked = conn.exec(
            "SELECT rd.registrarid, "
                   "r.handle, "
                   "(rd.blocked_from AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague') AS blocked_from, "
                   "rfrp.request_price_limit, "
                   "rfrp.email, "
                   "rfrp.telephone "
            "FROM registrar_disconnect rd "
            "LEFT JOIN request_fee_registrar_parameter rfrp ON rfrp.registrar_id = rd.registrarid "
            "JOIN registrar r ON r.id = rd.registrarid "
            "WHERE rd.blocked_from AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' >= date_trunc('day', NOW() AT TIME ZONE 'Europe/Prague')"
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

    bool hasRegistrarZoneAccess(unsigned long long _registrar_id, unsigned long long _zone_id) override
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
    void checkRegistrarExists(TID registrar_id) override
    {
        Database::Connection conn = Database::Manager::acquire();

        Database::Result reg_id = conn.exec_params(
                "select id from registrar where id = $1::integer"
                , Database::query_param_list (registrar_id));
        if(reg_id.size() != 1)
        {
            LOGGER.error(
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
            Logger::LoggerClient* logger_client,
            const boost::posix_time::ptime& p_from,
            const boost::posix_time::ptime& p_to,
            const boost::gregorian::date& zone_access_date) override
    {
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
                LOGGER.info(boost::format(
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

            LOGGER.info(boost::format("Request count data for registrar"
                " %1%, requests: %2%, total free: %3%, price: %4%") % reg_handle
                    % request_count % total_free_count % price);
        }

        return ret;
    }
private:
    DBSharedPtr db_;
};

unsigned long long RegistrarZoneAccess::max_id(ColIndex idx, Database::Result& result)
{
    TRACE("[CALL] RegistrarZoneAccess::max_id");
    unsigned long long ret =0;
    for (unsigned i = 0; i < result.size() ; ++i)
        if((result[i].size() > static_cast<unsigned long long>(idx))
                && (static_cast<unsigned>(result[i][idx]) > ret))
            ret = result[i][idx];
    LOGGER.debug(boost::format
            ("[CALL] RegistrarZoneAccess::max_id: %1%  result.size(): %2%")
                % ret % result.size());
    return ret;
}

/// Look if registrar have currently access to zone by id
 bool RegistrarZoneAccess::isInZone(unsigned long long registrar_id, unsigned long long zone_id)
 {
     bool ret = false;
     if((registrar_id <= max_registrar_id)
             && (zone_id <= max_zone_id))
                ret = flag.at(registrar_id).at(zone_id);
     LOGGER.debug(boost::format
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
        LOGGER.debug(boost::format("RegistrarZoneAccess::reload "
                "res.size: %1% rza.max_registrar_id: %2% max_zone_id: %3% ")
                % res.size()
                % max_registrar_id
                % max_zone_id
                );
        flag = RegistrarZoneAccess::RegistrarZoneAccessArray (max_registrar_id + 1
                ,RegistrarZoneAccess::RegistrarZoneAccessRow(max_zone_id + 1,false));
        for (unsigned i = 0; i < res.size() ; ++i)
        {
            LOGGER.debug(boost::format
                    ("[CALL] RegistrarZoneAccess::reload() for i: %1% ") % i );
             if(res[i].size() > IsInZone)
             {
                 LOGGER.debug
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
        LOGGER.error(boost::format("RegistrarZoneAccess::reload exception: %1%") % ex.what());
    }
    catch(...)
    {
        LOGGER.error("RegistrarZoneAccess::reload error");
    }
}//RegistrarZoneAccess::reload

Manager::AutoPtr Manager::create(DBSharedPtr db)
{
  TRACE("[CALL] LibFred::Registrar::Manager::create(db)");
  return Manager::AutoPtr(new ManagerImpl(db));
}

}//namespace LibFred::Registrar
}//namespace LibFred

