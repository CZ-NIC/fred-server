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

#include <vector>
#include <algorithm>
#include <functional>
#include "zone.h"
#include <iostream>
#include <sstream>
#include "types.h"
#include <ctype.h>
#include <idna.h>

#include "model_zone.h"
#include "model_zone_ns.h"
#include "model_zone_soa.h"
#include "model_price_list.h"

#include "types/money.h"

#include "log/logger.h"
#include "log/context.h"


#define RANGE(x) x.begin(),x.end()
#define IS_NUMBER(x) (x >= '0' && x <= '9')
#define IS_LETTER(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))

namespace Fred
{
  namespace Zone
  {

   class ZoneNsImpl
       : virtual public ZoneNs
       , private ModelZoneNs

    {
    public:
           ZoneNsImpl()
           : ModelZoneNs()
           {}
           ZoneNsImpl(TID id
                   , TID zoneid
                   , const std::string& fqdn
                   , const std::string& addrs)
           : ModelZoneNs()
           {
               ModelZoneNs::setId(id);
               ModelZoneNs::setZoneId(zoneid);
               ModelZoneNs::setFqdn(fqdn);
               ModelZoneNs::setAddrs(addrs);
           }
           const TID &getId() const
           {
               return ModelZoneNs::getId();
           }
           const TID &getZoneId() const
           {
               return ModelZoneNs::getZoneId();
           }
           const std::string &getFqdn() const
           {
               return ModelZoneNs::getFqdn();
           }
           const std::string &getAddrs() const
           {
               return ModelZoneNs::getAddrs();
           }
           void setId(const TID id)
           {
               ModelZoneNs::setId(id);
           }
           void setZoneId(const TID zoneId)
           {
               ModelZoneNs::setZoneId(zoneId);
           }

           void setFqdn(const std::string &fqdn)
           {
               ModelZoneNs::setFqdn(fqdn);
           }
           void setAddrs(const std::string &addrs)
           {
               ModelZoneNs::setAddrs(addrs);
           }

           bool operator==(const TID _id) const
           {
             return getId() == _id;
           }

           virtual void save() throw (SQL_ERROR)
           {
             try
             {
                 Database::Connection conn = Database::Manager::acquire();
                 TID id = getId();

                 if (id)
                     ModelZoneNs::update();
                 else
                     ModelZoneNs::insert();
             }//try
             catch (...)
             {
              LOGGER(PACKAGE).error("save: an error has occured");
              throw SQL_ERROR();
             }//catch (...)
           }//save

    };//class ZoneNsImpl





    class ZoneImpl : public Fred::CommonObjectImplNew
                   , virtual public  Zone
                   , private ModelZoneSoa
    {
        typedef std::vector<boost::shared_ptr<ZoneNsImpl> > ZoneNsList;
        typedef ZoneNsList::iterator ZoneNsListIter;
        ZoneNsList zone_ns_list; /// zone ns records

        void operator==(const ZoneImpl&){};
    public:

        ZoneImpl()
            : CommonObjectImplNew()
            , ModelZoneSoa()
        {}
        ZoneImpl(
                    TID _id,
                    const std::string& _fqdn,
                    const int _exPeriodMin,
                    const int _exPeriodMax,
                    const int _valPeriod,
                    const int _dotsMax,
                    const bool _enumZone,
                    const int _ttl,
                    const std::string& _hostmaster,
                    const int _serial,
                    const int _refresh,
                    const int _updateRetr,
                    const int  _expiry,
                    const int _minimum,
                    const std::string& _nsFqdn
                )
            : CommonObjectImplNew()
            , ModelZoneSoa()
        {
            ModelZone::setId(_id);
            ModelZone::setFqdn(_fqdn);
            ModelZone::setExPeriodMin(_exPeriodMin);
            ModelZone::setExPeriodMax(_exPeriodMax);
            ModelZone::setValPeriod(_valPeriod);
            ModelZone::setDotsMax(_dotsMax);
            ModelZone::setEnumZone(_enumZone);

            ModelZoneSoa::setTtl(_ttl);
            ModelZoneSoa::setHostmaster(_hostmaster);
            ModelZoneSoa::setSerial(_serial);
            ModelZoneSoa::setRefresh(_refresh);
            ModelZoneSoa::setUpdateRetr(_updateRetr);
            ModelZoneSoa::setExpiry(_expiry);
            ModelZoneSoa::setMinimum(_minimum);
            ModelZoneSoa::setNsFqdn(_nsFqdn);
        }
        ~ZoneImpl()
        {}


      /// compare if domain belongs to this zone (according to suffix)
      bool isDomainApplicable(const std::string& domain) const
      {
          const std::string& fqdn = ModelZone::getFqdn();
        std::string copy = domain;
        for (unsigned i=0; i<copy.length(); i++)
          copy[i] = tolower(copy[i]);
        unsigned l = fqdn.length();
        if (copy.length() < l) return false;
        return copy.compare(copy.length()-l,l,fqdn) == 0;
      }

      /// max. number of labels in fqdn
      virtual unsigned getMaxLevel() const
      {
          const std::string& fqdn = ModelZone::getFqdn();
          int maxlevel = std::count(fqdn.begin(), fqdn.end(), '.')
          + 1 + ModelZone::getDotsMax();
          return  maxlevel;
      }

      /// Create new ZoneNs record
      virtual ZoneNs* newZoneNs()
      {
          boost::shared_ptr<ZoneNsImpl> new_ZoneNs ( new ZoneNsImpl());
          zone_ns_list.push_back(new_ZoneNs);
          return new_ZoneNs.get();
      }
      /// Return ZoneNs list size
      virtual unsigned getZoneNsSize() const
      {
          return zone_ns_list.size();
      }
      /// Return ZoneNs list member by index
      virtual ZoneNs* getZoneNs(unsigned idx) const
      {
          return idx < zone_ns_list.size() ? zone_ns_list[idx].get() : NULL;
      }
      /// Delete ZoneNs or do nothing
      virtual void deleteZoneNs(unsigned idx)
      {
          if (idx < zone_ns_list.size()) {
              zone_ns_list.erase(zone_ns_list.begin()+idx);
           }
      }
      /// Clear ZoneNs list
      virtual void clearZoneNsList()
      {
          zone_ns_list.clear();
      }
      /// Save changes to database
      virtual void save() throw (SQL_ERROR)
      {
          // save zone data
        try
        {
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);
            TID id = getId();
            if (id)
                ModelZoneSoa::update();
            else
                ModelZoneSoa::insert();
            std::ostringstream sql;
            sql << "DELETE FROM zone_ns WHERE zone=" << id;
            conn.exec(sql.str());
            for (unsigned j = 0; j < zone_ns_list.size(); j++)
            {
                zone_ns_list[j]->setZoneId(id);
                zone_ns_list[j]->save();
            }
            tx.commit();
        }//try
        catch (...)
        {
         LOGGER(PACKAGE).error("save: an error has occured");
         throw SQL_ERROR();
        }//catch (...)
      }//save

      //ModelZone
      const TID &getId() const
      {
          return ModelZone::getId();
      }
      void setId(const TID id)
      {
          ModelZone::setId(id);
      }
      const std::string &getFqdn() const
      {
          return ModelZone::getFqdn();
      }
      void setFqdn(const std::string &fqdn)
      {
          ModelZone::setFqdn(fqdn);
      }
      const int &getExPeriodMin() const
      {
          return ModelZone::getExPeriodMin();
      }
      void setExPeriodMin(const int exPeriodMin)
      {
          ModelZone::setExPeriodMin(exPeriodMin);
      }
      const int &getExPeriodMax() const
      {
          return ModelZone::getExPeriodMax();
      }
      void setExPeriodMax(const int exPeriodMax)
      {
          ModelZone::setExPeriodMax(exPeriodMax);
      }
      const int &getValPeriod() const
      {
          return ModelZone::getValPeriod();
      }
      void setValPeriod(const int valPeriod)
      {
          ModelZone::setValPeriod(valPeriod);
      }
      const int &getDotsMax() const
      {
          return ModelZone::getDotsMax();
      }
      void setDotsMax(const int dotsMax)
      {
          ModelZone::setDotsMax(dotsMax);
      }
      const bool &getEnumZone() const
      {
          return ModelZone::getEnumZone();
      }
      const bool &isEnumZone() const
      {
          return getEnumZone();
      }
      void setEnumZone(const bool enumZone)
      {
          ModelZone::setEnumZone(enumZone);
      }
      //ModelZoneSoa
      const int &getTtl() const
      {
          return ModelZoneSoa::getTtl();
      }
      const std::string &getHostmaster() const
      {
          return ModelZoneSoa::getHostmaster();
      }
      const int &getSerial() const
      {
          return ModelZoneSoa::getSerial();
      }
      const int &getRefresh() const
      {
          return ModelZoneSoa::getRefresh();
      }
      const int &getUpdateRetr() const
      {
          return ModelZoneSoa::getUpdateRetr();
      }
      const int &getExpiry() const
      {
          return ModelZoneSoa::getExpiry();
      }
      const int &getMinimum() const
      {
          return ModelZoneSoa::getMinimum();
      }
      const std::string &getNsFqdn() const
      {
          return ModelZoneSoa::getNsFqdn();
      }
      void setTtl(const int ttl)
      {
          ModelZoneSoa::setTtl(ttl);
      }
      void setHostmaster(const std::string &hostmaster)
      {
          ModelZoneSoa::setHostmaster(hostmaster);
      }
      void setSerial(const int serial)
      {
          ModelZoneSoa::setSerial(serial);
      }
      void setRefresh(const int refresh)
      {
          ModelZoneSoa::setRefresh(refresh);
      }
      void setUpdateRetr(const int updateRetr)
      {
          ModelZoneSoa::setUpdateRetr(updateRetr);
      }
      void setExpiry(const int expiry)
      {
          ModelZoneSoa::setExpiry(expiry);
      }
      void setMinimum(const int minimum)
      {
          ModelZoneSoa::setMinimum(minimum);
      }
      void setNsFqdn(const std::string &nsFqdn)
      {
          ModelZoneSoa::setNsFqdn(nsFqdn);
      }

      void putZoneNs(TID _id,
                      TID _zoneid,
                  const std::string& fqdn,
                  const std::string& addrs)
      {
        zone_ns_list.push_back(boost::shared_ptr<ZoneNsImpl>(new ZoneNsImpl(_id,_zoneid,fqdn,addrs)));
      }
      bool hasId(TID _id) const
      {
        return getId() == _id;
      }
      void resetId()
      {
        setId(0);
        for (TID i = 0; i < zone_ns_list.size(); i++)
            zone_ns_list[i]->setId(0);
      }

    };//class ZoneImpl


    COMPARE_CLASS_IMPL_NEW(ZoneImpl, Fqdn)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, ExPeriodMin)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, ExPeriodMax)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, ValPeriod)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, DotsMax)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, EnumZone)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, Ttl)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, Hostmaster)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, Serial)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, Refresh)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, UpdateRetr)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, Expiry)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, Minimum)
    COMPARE_CLASS_IMPL_NEW(ZoneImpl, NsFqdn)

    class ZoneListImpl
		: public Fred::CommonListImplNew
		, public ZoneList
	{
		    long long ptr_idx_;//from CommonListImpl
			std::string fqdn;
			std::string registrar_handle;
			TID idFilter;
	public:
			ZoneListImpl() :
			    CommonListImplNew()
			    , ptr_idx_(0)
			    , fqdn("")
			    , registrar_handle("")
			    , idFilter(0)
			  {}
			  ~ZoneListImpl()
			  {
			    clear();
			  }
			  virtual void setIdFilter(TID _idFilter)
			  {
			    idFilter = _idFilter;
			  }
			  virtual void setFqdnFilter(const std::string& _fqdn)
			  {
			    fqdn = _fqdn;
			  }
              virtual void setRegistrarHandleFilter(const std::string& _registrar_handle)
              {
                  registrar_handle = _registrar_handle;
              }

			  void resetIDSequence()
			  {
			    ptr_idx_ = -1;
			  }
			  ZoneImpl* findIDSequence(TID _id)
			  {
			    // must be sorted by ID to make sence
			    if (ptr_idx_ < 0)
			      ptr_idx_ = 0;
			    long long m_data_size = m_data.size();
			    ZoneImpl* ret_ptr=0;

			    for ( ; ptr_idx_ < m_data_size
			            ; ptr_idx_++)
			    {
			        ret_ptr = dynamic_cast<ZoneImpl* >(m_data.at(ptr_idx_));
			        if (ret_ptr == 0) throw std::runtime_error("ZoneImpl* findIDSequence: not a ZoneImpl pointer");
			        if (ret_ptr->getId() >=  _id) break;
			    }

			    if (ptr_idx_ == m_data_size
			            || ret_ptr->getId() != _id)
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
			      TRACE("[CALL] ZoneListImpl::reload()");
			      try
			      {
			          Database::Connection conn = Database::Manager::acquire();
			          clear();
			          std::ostringstream sql;
			          sql << "SELECT z.id, z.fqdn, z.ex_period_min, z.ex_period_max"
			          ", z.val_period, z.dots_max, z.enum_zone"
			          ", zs.ttl, zs.hostmaster, zs.serial, zs.refresh, zs.update_retr"
			          ", zs.expiry, zs.minimum, zs.ns_fqdn"
			          " FROM zone z LEFT JOIN zone_soa zs ON z.id = zs.zone";
			          if(!registrar_handle.empty())
			              sql << "JOIN registrarinvoice ri ON z.id = ri.zone"
			              "JOIN registrar r ON r.id = ri.registrarid AND ri.fromdate<=CURRENT_DATE"
			              "AND (ri.todate >=CURRENT_DATE OR ri.todate ISNULL)"
			              "AND r.handle ILIKE TRANSLATE('" << conn.escape(registrar_handle)
			              << "','*?','%_')";//if(!registrar_handle.empty())
			          sql << " WHERE 1=1 ";
			          if (!fqdn.empty())
			              sql << "AND " << "z.fqdn" << " ILIKE TRANSLATE('"
			              << conn.escape(fqdn) << "','*?','%_') ";//if (!fqdn.empty())

			          sql << " ORDER BY length(z.fqdn) DESC";

			          Database::Result res = conn.exec(sql.str());

			          TRACE(boost::format("[CALL] ZoneListImpl::reload(), registrar_handle.empty(): %1% fqdn: %2% res.size(): %3%")
                          % registrar_handle.empty() % fqdn % res.size());

			          for (unsigned i=0; i < static_cast<unsigned>(res.size()); i++)
			          {
			              appendToList(new ZoneImpl
			                              (
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
			                                ,res[i][13]
			                                ,res[i][14]
			                              )//new
			                             );//push_back
			          }//for

			          resetIDSequence();
                      sql.str("");
                      sql << "SELECT id,zone,fqdn,addrs " << "FROM zone_ns ORDER BY zone";

                      Database::Result res2 = conn.exec(sql.str());
                      for (unsigned i=0; i < static_cast<unsigned>(res2.size()); i++)
                      {
                        // find associated zone
                        unsigned zoneId = res2[i][1];
                        ZoneImpl *z = dynamic_cast<ZoneImpl* >(findIDSequence(zoneId));

                        if (z)
                        {
                          z->putZoneNs(res2[i][0], res2[i][1], res2[i][2], res2[i][3]);
                        }
                      }//for
			      }//try
			      catch (...)
			      {
			       LOGGER(PACKAGE).error("reload: an error has occured");
			       throw SQL_ERROR();
			      }//catch (...)
			  }//reload()

		      virtual void reload(Database::Filters::Union &uf)
		      {
		          TRACE("[CALL] ZoneListImpl::reload(uf)");
		          clear();
		          uf.clearQueries();

		          bool at_least_one = false;
		          Database::SelectQuery info_query;
		          std::auto_ptr<Database::Filters::Iterator> fit(uf.createIterator());
		          for (fit->first(); !fit->isDone(); fit->next())
		          {
		            Database::Filters::Zone *zf =
		                dynamic_cast<Database::Filters::Zone*>(fit->get());
		            if (!zf)
		              continue;

		            Database::SelectQuery *tmp = new Database::SelectQuery();
		            tmp->select() << "z.id, z.fqdn, z.ex_period_min, z.ex_period_max"
                      ", z.val_period, z.dots_max, z.enum_zone"
                      ", zs.ttl, zs.hostmaster, zs.serial, zs.refresh, zs.update_retr"
                      ", zs.expiry, zs.minimum, zs.ns_fqdn";
		            tmp->from() << "zone z JOIN zone_soa zs ON z.id = zs.zone";
		            tmp->order_by() << "z.id";

		            uf.addQuery(tmp);
		            at_least_one = true;
		          }//for fit
		          if (!at_least_one) {
		            LOGGER(PACKAGE).error("wrong filter passed for reload ZoneList!");
		            return;
		          }

		          /* manually add query part to order result by id
		           * need for findIDSequence() */
		          uf.serialize(info_query);
		          std::string info_query_str = str(boost::format("%1% LIMIT %2%") % info_query.str() % m_limit);
		          LOGGER(PACKAGE).debug(boost::format("reload(uf) ZoneList query: %1%") % info_query_str);
		          try {
		            Database::Connection conn = Database::Manager::acquire();
		            Database::Result z_info = conn.exec(info_query_str);

		            for (unsigned i=0; i < static_cast<unsigned>(z_info.size()); i++)
		            {
		                m_data.push_back(new ZoneImpl(
		                      z_info[i][0]
                              ,z_info[i][1]
                              ,z_info[i][2]
                              ,z_info[i][3]
                              ,z_info[i][4]
                              ,z_info[i][5]
                              ,z_info[i][6]
                              ,z_info[i][7]
                              ,z_info[i][8]
                              ,z_info[i][9]
                              ,z_info[i][10]
                              ,z_info[i][11]
                              ,z_info[i][12]
                              ,z_info[i][13]
                              ,z_info[i][14]
                                      ));
		            }//for i

		            if (m_data.empty())
		              return;


		            resetIDSequence();
		            Database::SelectQuery zns_query;
		            zns_query.select() << "id, zone, fqdn, addrs ";
		            zns_query.from() << "zone_ns";
		            zns_query.order_by() << "zone";

		            Database::Result r_zns = conn.exec(zns_query);

                    for (unsigned i=0; i < static_cast<unsigned>(r_zns.size()); i++)
                    {
                      // find associated zone
                      unsigned zoneId = r_zns[i][1];
                      ZoneImpl *z = dynamic_cast<ZoneImpl* >(findIDSequence(zoneId));

                      if (z)
                      {
                        z->putZoneNs(r_zns[i][0], r_zns[i][1], r_zns[i][2], r_zns[i][3]);
                      }
                    }//for

		            /* checks if row number result load limit is active and set flag */
		            CommonListImplNew::reload();
		          }
		          catch (std::exception& ex)
		          {
		            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
		          }
		      }//reload(Database::Filters::Union &uf)

		      virtual void sort(MemberType _member, bool _asc)
		      {
		        switch (_member)
		        {
		          case MT_FQDN:
		            stable_sort(m_data.begin(), m_data.end(), CompareFqdn(_asc));
		            break;
                  case MT_EXPERIODMIN:
                    stable_sort(m_data.begin(), m_data.end(), CompareExPeriodMin(_asc));
                    break;
                  case MT_EXPERIODMAX:
                    stable_sort(m_data.begin(), m_data.end(), CompareExPeriodMax(_asc));
                    break;
                  case MT_VALPERIOD:
                    stable_sort(m_data.begin(), m_data.end(), CompareValPeriod(_asc));
                    break;
                  case MT_DOTSMAX:
                    stable_sort(m_data.begin(), m_data.end(), CompareDotsMax(_asc));
                    break;
                  case MT_ENUMZONE:
                    stable_sort(m_data.begin(), m_data.end(), CompareEnumZone(_asc));
                    break;
                  case MT_TTL:
                    stable_sort(m_data.begin(), m_data.end(), CompareTtl(_asc));
                    break;
                  case MT_HOSTMASTER:
                    stable_sort(m_data.begin(), m_data.end(), CompareHostmaster(_asc));
                    break;
                  case MT_SERIAL:
                    stable_sort(m_data.begin(), m_data.end(), CompareSerial(_asc));
                    break;
                  case MT_REFRESH:
                    stable_sort(m_data.begin(), m_data.end(), CompareRefresh(_asc));
                    break;
                  case MT_UPDATERETR:
                    stable_sort(m_data.begin(), m_data.end(), CompareUpdateRetr(_asc));
                    break;
                  case MT_EXPIRY:
                    stable_sort(m_data.begin(), m_data.end(), CompareExpiry(_asc));
                    break;
                  case MT_MINIMUM:
                    stable_sort(m_data.begin(), m_data.end(), CompareMinimum(_asc));
                    break;
                  case MT_NSFQDN:
                    stable_sort(m_data.begin(), m_data.end(), CompareNsFqdn(_asc));
                    break;
		        }
		      }//sort

		      virtual void makeQuery(bool, bool, std::stringstream&) const
		      {
		        // empty implementation
		      }
		      virtual const char* getTempTableName() const
		      {
		        return "";
		      }

              const Zone* findApplicableZoneByDomainFqdn(const std::string& domain_fqdn) const
              {
                  Zone *z=0, *ret = 0;
                  for ( unsigned i = 0; i < m_data.size(); i++)
                  {
                      z = dynamic_cast<Zone*>(m_data[i]);
                      if (z == 0)
                            throw std::runtime_error("ZoneListImpl* findApplicableZoneByDomainFqdn: not a Zone pointer");
                      if(z->isDomainApplicable(domain_fqdn))
                      {
                          ret=z;
                          break;
                      }
                  };
                  return ret;
              }


		      const Zone* findZoneByFqdn(const std::string& fqdn) const
		      {
		          Zone *z=0, *ret = 0;
		          for ( unsigned i = 0; i < m_data.size(); i++)
		          {
		              z = dynamic_cast<Zone*>(m_data[i]);
		              if (z == 0)
		                  throw std::runtime_error("ZoneListImpl* findZoneByFqdn: not a Zone pointer");
		              if(z->getFqdn().compare(fqdn) == 0 )
		              {
		                  ret=z;
		                  break;
		              }
		          };
		          return ret;
		      }

		      virtual Zone* create()
		        {
		          return new ZoneImpl();
		        }

		      void clearFilter()
		      {
		          fqdn = "";
		          registrar_handle="";
		      }

		      virtual Fred::Zone::Zone* findId(Database::ID id) const
		      {
                  Zone *z=0, *ret = 0;
                  for ( unsigned i = 0; i < m_data.size(); i++)
                  {
                      z = dynamic_cast<Zone*>(m_data[i]);
                      if (z == 0)
                          throw std::runtime_error("ZoneListImpl* findId: not a Zone pointer");

                      if(z->getId() == id )
                      {
                          ret=z;
                          break;
                      }
                  };
                  return ret;
		      }

	};//class ZoneListImpl

    class ManagerImpl : virtual public Manager
    {
      ZoneListImpl zoneList;
      std::string defaultEnumSuffix;
      std::string enumZoneString;
      std::string defaultDomainSuffix;
      bool loaded;
     public:
      ManagerImpl() :
       defaultEnumSuffix("0.2.4"),
       enumZoneString("e164.arpa"),
       defaultDomainSuffix("cz"),
       loaded(false)
      {
      }
      void load()
      {
 		zoneList.reload();
        loaded = true;
      }//load()

      /// interface method implementation
      void parseDomainName(
        const std::string& fqdn, DomainName& domain, bool allowIDN
      ) const throw (INVALID_DOMAIN_NAME)
      {
        std::string part; // one part(label) of fqdn
        for (unsigned i=0; i<fqdn.size(); i++) {
          if (part.empty()) {
            // first character of every label has to be letter or digit
            // digit is not in RFC 1034 but it's required in ENUM domains
            if (!IS_NUMBER(fqdn[i]) && !IS_LETTER(fqdn[i]))
              throw INVALID_DOMAIN_NAME();
          }
          else {
            // dot '.' is a separator of labels, store and clear part
            if (fqdn[i] == '.') {
              // part must finish with letter or digit
              if (!IS_NUMBER(*part.rbegin()) && !IS_LETTER(*part.rbegin()))
                throw INVALID_DOMAIN_NAME();
              // length of part should be < 64
              if (part.length() > 63)
                throw INVALID_DOMAIN_NAME();
              domain.push_back(part);
              part.clear();
              continue;
            }
            else {
              if (fqdn[i] == '-') {
                // dash '-' is acceptable only if last character wasn't dash
                if (part[part.length()-1] == '-' &&
                    (!allowIDN || part != "xn-"))
                  throw INVALID_DOMAIN_NAME();
              }
              else {
                // other character could be only number or letter
                if (!IS_NUMBER(fqdn[i]) && !IS_LETTER(fqdn[i]))
                  throw INVALID_DOMAIN_NAME();
              }
            }
          }
          // add character into part
          part += fqdn[i];
        }
        // last part cannot be empty
        if (part.empty()) throw INVALID_DOMAIN_NAME();
        // append last part
        domain.push_back(part);
        // enum domains has special rules
        if (checkEnumDomainSuffix(fqdn) && !checkEnumDomainName(domain))
          throw INVALID_DOMAIN_NAME();
      }
      /// interface method implementation
      bool checkEnumDomainName(const DomainName& domain) const
      {
        // must have suffix e164.org
        if (domain.size()<=2) return false;
        // every part of domain name except of suffix must be one digit
        for (unsigned i=0; i<domain.size()-2; i++)
          if (domain[i].length() != 1 || !IS_NUMBER(domain[i][0]))
            return false;
        return true;
      }
      /// check if suffix is e164.arpa
      bool checkEnumDomainSuffix(const std::string& fqdn) const
      {
        const std::string& esuf = getEnumZoneString();
        // check if substring 'esuf' found from right
        // is last substring in fqdn
        std::string::size_type i = fqdn.rfind(esuf);
        return  i != std::string::npos && i + esuf.size() == fqdn.size();
      }
      /// interface method implementation
      std::string makeEnumDomain(const std::string& number)
        const throw (NOT_A_NUMBER)
      {
        std::string result;
        unsigned l = number.size();
        if (!l) throw NOT_A_NUMBER();
        // where to stop in backward processing
        unsigned last = 0;
        if (!number.compare(0,2,"00")) last = 2;
        else if (number[0] == '+') last = 1;
        // process string
        for (unsigned i=l-1; i>last; i--) {
          if (!IS_NUMBER(number[i])) throw NOT_A_NUMBER();
          result += number[i];
          result += '.';
        }
        if (!IS_NUMBER(number[last])) throw NOT_A_NUMBER();
        result += number[last];
        // append default country code if short
        if (!last) {
          result += '.';
          result += getDefaultEnumSuffix();
        }
        // append enum domain zone
        result += '.';
        result += getEnumZoneString();
        return result;
      }
      const std::string& getDefaultEnumSuffix() const
      {
        return defaultEnumSuffix;
      }
      const std::string& getDefaultDomainSuffix() const
      {
        return defaultDomainSuffix;
      }
      const std::string& getEnumZoneString() const
      {
        return enumZoneString;
      }
      const Zone* findApplicableZone(const std::string& domain_fqdn) const
      {
        // nonconst casting for lazy initialization
        if (!loaded) ((Fred::Zone::ManagerImpl *)this)->load();
        return zoneList.findApplicableZoneByDomainFqdn(domain_fqdn);
      }

      virtual bool checkTLD(const DomainName& domain) const
      {
          try
          {
          	Database::Connection conn = Database::Manager::acquire();

            if (domain.size() < 1) return false;
            std::stringstream sql;
            sql << "SELECT COUNT(*) FROM enum_tlds "
                << "WHERE LOWER(tld)=LOWER('" << conn.escape(*domain.rbegin()) << "')";

  			Database::Result res = conn.exec(sql.str());
  			if((res.size() > 0)&&(res[0].size() > 0))
  			{
  				unsigned count = res[0][0];
  				bool ret = count > 0;
  				return ret;
  			}
  			else
  			{
  				return false;
  			}
          }//try
          catch (...)
          {
              LOGGER(PACKAGE).error("checkTLD: an error has occured");
              return false;
          }//catch (...)
      }//checkTLD

      /// add zone with zone_soa record
      virtual void addZone(
              const std::string& fqdn,
              int ex_period_min,
              int ex_period_max,
              int ttl,
              const std::string &hostmaster,
              int refresh,
              int update_retr,
              int expiry,
              int minimum,
              const std::string &ns_fqdn)
		throw (SQL_ERROR, ALREADY_EXISTS)
      {
        try
        {
        	Database::Connection conn = Database::Manager::acquire();

        	std::stringstream sql;
			sql << "SELECT COUNT(*) FROM zone WHERE fqdn='" << conn.escape(fqdn) << "'";

			Database::Result res = conn.exec(sql.str());

			unsigned count = res[0][0];

			if (count != 0)
			{
				LOGGER(PACKAGE).notice("Zone already exists.");
				throw ALREADY_EXISTS();
			}

			unsigned dots = 1;
			bool enumZone = checkEnumDomainSuffix(fqdn);
			if (enumZone) dots = 9;

			Database::Transaction tx(conn);

			ModelZoneSoa zn;
			zn.setFqdn(fqdn);
			zn.setExPeriodMax(ex_period_max);
			zn.setExPeriodMin(ex_period_min);
			zn.setValPeriod(enumZone ? 6 : 0);
			zn.setDotsMax(static_cast<int>(dots));
			zn.setEnumZone(enumZone);
			zn.setTtl(ttl);
			// zn.setSerial() is null
			zn.setHostmaster(hostmaster);
			zn.setRefresh(refresh);
			zn.setUpdateRetr(update_retr);
			zn.setExpiry(expiry);
			zn.setMinimum(minimum);
			zn.setNsFqdn(ns_fqdn);
			zn.insert();

			tx.commit();

        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("addZone: an error has occured");
            throw SQL_ERROR();
        }//catch (...)
      }//addZone

      /// add only zone record
      virtual void addOnlyZoneRecord(
              const std::string& fqdn,
              int ex_period_min=12,
              int ex_period_max=120)
        throw (SQL_ERROR, ALREADY_EXISTS)
		{
		try
		{
			Database::Connection conn = Database::Manager::acquire();

			std::stringstream sql;
			sql << "SELECT COUNT(*) FROM zone WHERE fqdn='" << conn.escape(fqdn) << "'";

			Database::Result res = conn.exec(sql.str());

			unsigned count = res[0][0];

			if (count != 0)
			{
				LOGGER(PACKAGE).notice("Zone already exists.");
				throw ALREADY_EXISTS();
			}

			unsigned dots = 1;
			bool enumZone = checkEnumDomainSuffix(fqdn);
			if (enumZone) dots = 9;

			Database::Transaction tx(conn);

			ModelZone zn;
			zn.setFqdn(fqdn);
			zn.setExPeriodMax(ex_period_max);
			zn.setExPeriodMin(ex_period_min);
			zn.setValPeriod(enumZone ? 6 : 0);
			zn.setDotsMax(static_cast<int>(dots));
			zn.setEnumZone(enumZone);
			zn.insert();

			tx.commit();

		}//try
		catch (...)
		{
			LOGGER(PACKAGE).error("addOnlyZoneRecord: an error has occured");
			throw SQL_ERROR();
		}//catch (...)
		}//addOnlyZoneRecord

      /// add only zone_soa record identified by fqdn
      virtual void addOnlyZoneSoaRecordByFqdn(
              const std::string& fqdn,
              int ttl=18000,
              const std::string &hostmaster="hostmaster@localhost",
              int refresh=10600,
              int update_retr=3600,
              int expiry=1209600,
              int minimum=7200,
              const std::string &ns_fqdn="localhost")
		throw (SQL_ERROR, ALREADY_EXISTS, NOT_FOUND)
		{
			try
			{
				Database::Connection conn = Database::Manager::acquire();

				std::stringstream sql_zone;
				sql_zone << "SELECT id FROM zone WHERE fqdn='" << conn.escape(fqdn) << "'";
				Database::Result res_zone = conn.exec(sql_zone.str());
				if (res_zone.size() < 1)
				{
					LOGGER(PACKAGE).notice("Zone not found.");
					throw NOT_FOUND();//zone not found
				}
				const TID id = res_zone[0][0];

				std::stringstream sql_zone_soa;
				sql_zone_soa << "SELECT COUNT(*) FROM zone_soa WHERE zone = " << id ;
				Database::Result res_zone_soa = conn.exec(sql_zone_soa.str());
				if (res_zone_soa.size() != 1)
				{
					LOGGER(PACKAGE).error("addOnlyZoneSoaRecordByFqdn: an error has occured");
					throw SQL_ERROR();
				}

				if (static_cast<unsigned int>(res_zone_soa[0][0]) == 1)
				{
					LOGGER(PACKAGE).notice("Zone already exists.");
					throw ALREADY_EXISTS();//zone already exists
				}

				Database::Transaction tx(conn);

				//ModelZone zn;
				//zn.setId(id);

				ModelZoneSoa zsa;
                zsa.setId(id);
				zsa.setTtl(ttl);
				zsa.setHostmaster(hostmaster);
				//zsa.setSerial() is null
				zsa.setRefresh(refresh);
				zsa.setUpdateRetr(update_retr);
				zsa.setExpiry(expiry);
				zsa.setMinimum(minimum);
				zsa.setNsFqdn(ns_fqdn);
				zsa.insert();

				tx.commit();

			}//try
			catch (...)
			{
				LOGGER(PACKAGE).error("addOnlyZoneSoaRecordByFqdn: an error has occured");
				throw SQL_ERROR();
			}//catch (...)
		}//addOnlyZoneSoaRecordByFqdn

      /// update zone and zone_soa record identified by fqdn
      virtual void updateZoneByFqdn(
              const std::string& fqdn,
              int ex_period_min,
              int ex_period_max,
              int ttl,
              const std::string &hostmaster,
              int refresh,
              int update_retr,
              int expiry,
              int minimum,
              const std::string &ns_fqdn)
		throw (SQL_ERROR, NOT_FOUND)
      {
        try
        {
        	Database::Connection conn = Database::Manager::acquire();

        	std::stringstream sql_zone;
			sql_zone << "SELECT id FROM zone WHERE fqdn='" << conn.escape(fqdn) << "'";
			Database::Result res_zone = conn.exec(sql_zone.str());
			if (res_zone.size() < 1)
				throw NOT_FOUND();//zone not found
			const TID id = res_zone[0][0];

        	std::stringstream sql_zone_soa;
			sql_zone_soa << "SELECT COUNT(*) FROM zone_soa WHERE zone = " << id ;
			Database::Result res_zone_soa = conn.exec(sql_zone_soa.str());
			if (res_zone_soa.size() < 1)
				throw NOT_FOUND();//zone not found

			updateZoneById
			(
				id,
				fqdn,
				ex_period_min,
				ex_period_max,
				ttl,
				hostmaster,
				refresh,
				update_retr,
				expiry,
				minimum,
				ns_fqdn
			);

        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("updateZoneByFqdn: an error has occured");
            throw SQL_ERROR();
        }//catch (...)
      }//updateZoneByFqdn

      /// update zone and zone_soa record identified by id
      virtual void updateZoneById(
    		  const unsigned long long id,
              const std::string& fqdn,
              int ex_period_min,
              int ex_period_max,
              int ttl,
              const std::string &hostmaster,
              int refresh,
              int update_retr,
              int expiry,
              int minimum,
              const std::string &ns_fqdn)
		throw (SQL_ERROR)
      {
        try
        {
        	Database::Connection conn = Database::Manager::acquire();

			unsigned dots = 1;
			bool enumZone = checkEnumDomainSuffix(fqdn);
			if (enumZone) dots = 9;

			Database::Transaction tx(conn);

			ModelZoneSoa zn;
			zn.setId(id);
			zn.setFqdn(fqdn);
			zn.setExPeriodMax(ex_period_max);
			zn.setExPeriodMin(ex_period_min);
			zn.setValPeriod(enumZone ? 6 : 0);
			zn.setDotsMax(static_cast<int>(dots));
			zn.setEnumZone(enumZone);
			zn.setTtl(ttl);
			zn.setHostmaster(hostmaster);
			zn.setRefresh(refresh);
			zn.setUpdateRetr(update_retr);
			zn.setExpiry(expiry);
			zn.setMinimum(minimum);
			zn.setNsFqdn(ns_fqdn);
			//zn.setSerial() is null
			zn.update();

			tx.commit();

        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("updateZoneById: an error has occured");
            throw SQL_ERROR();
        }//catch (...)
      }//updateZoneById


      virtual void addZoneNs(
              const std::string &zone,
              const std::string &fqdn,
              const std::string &addr)
          throw (SQL_ERROR,NOT_FOUND)
      {
          try
          {
        	Database::Connection conn = Database::Manager::acquire();

  			std::stringstream sql_zone;
  			sql_zone << "SELECT id FROM zone WHERE fqdn='" << conn.escape(zone) << "'";
  			Database::Result res_zone = conn.exec(sql_zone.str());
  			if (res_zone.size() < 1)
  				throw NOT_FOUND();//zone not found
  			const TID zone_id = res_zone[0][0];

  			  ModelZoneNs zn;
  			  zn.setZoneId(zone_id);
  			  zn.setFqdn(fqdn);
  			  zn.setAddrs(std::string("{")+addr+"}");
  			  zn.insert();

          }//try
          catch (...)
          {
              LOGGER(PACKAGE).error("addZoneNs: an error has occured");
              throw SQL_ERROR();
          }//catch (...)

      }//addZoneNs

      virtual void updateZoneNsById(
    		  const unsigned long long id,
              const std::string &zone,
              const std::string &fqdn,
              const std::string &addr)
          throw (SQL_ERROR,NOT_FOUND)
	{
		try
		{
		  Database::Connection conn = Database::Manager::acquire();

			std::stringstream sql_zone;
			sql_zone << "SELECT id FROM zone WHERE fqdn='" << conn.escape(zone) << "'";
			Database::Result res_zone = conn.exec(sql_zone.str());
			if (res_zone.size() < 1)
				throw NOT_FOUND();//zone not found
			const TID zone_id = res_zone[0][0];

			  ModelZoneNs zn;
			  zn.setId(id);
			  zn.setZoneId(zone_id);
			  zn.setFqdn(fqdn);
			  zn.setAddrs(std::string("{")+addr+"}");
			  zn.update();

		}//try
		catch (...)
		{
			LOGGER(PACKAGE).error("updateZoneNsById: an error has occured");
			throw SQL_ERROR();
		}//catch (...)

	}//updateZoneNsById


      virtual void addPrice(
              int zoneId,
              const std::string& operation,
              const Database::DateTime &validFrom,
              const Database::DateTime &validTo,
              const Money &price,
              int period
              , const bool enable_postpaid_operation)
      {
            try
            {
                Database::Connection conn = Database::Manager::acquire();

                Database::Result res_op = conn.exec_params(
                        "SELECT id FROM enum_operation WHERE operation=$1::text"
                        , Database::query_param_list(operation));
                if(res_op.size() == 0) throw std::runtime_error("addPrice: operation not found");
                int operationId = res_op[0][0];

            	ModelPriceList pl;
            	pl.setZoneId(zoneId);
            	pl.setOperationId(operationId);
            	if(!validFrom.get().is_not_a_date_time())
            	    pl.setValidFrom(validFrom);
            	if(!validTo.get().is_not_a_date_time())
            	    pl.setValidTo(validTo);
            	pl.setPrice(price.get_string());
            	pl.setQuantity(period);
            	pl.setEnablePostpaidOperation(enable_postpaid_operation);
            	pl.insert();

            }//try
            catch (...)
            {
                LOGGER(PACKAGE).error("addPrice: an error has occured");
                throw;
            }//catch (...)

      }//addPrice

      virtual void addPrice(
              const std::string &zone,
              const std::string& operation,
              const Database::DateTime &validFrom,
              const Database::DateTime &validTo,
              const Decimal &price,
              int period
              , const bool enable_postpaid_operation)
      {
          try
          {
          	Database::Connection conn = Database::Manager::acquire();

  			Database::Result res = conn.exec_params(
  			        "SELECT id FROM zone WHERE fqdn=$1::text"
  			        , Database::query_param_list(zone));
  			if(res.size() == 0) throw std::runtime_error("addPrice: zone not found");
  			int zoneId = res[0][0];

  			addPrice(zoneId, operation, validFrom, validTo, price, period, enable_postpaid_operation);
          }//try
          catch (...)
          {
              LOGGER(PACKAGE).error("addPrice: an error has occured");
              throw;
          }//catch (...)
      }//addPrice

      virtual std::string encodeIDN(const std::string& fqdn) const
      {
        std::string result;
        char *p;
        idna_to_ascii_8z(fqdn.c_str(), &p, 0);
        result = p ? p : fqdn;
        if (p) free(p);
        return result;
      }

      virtual std::string decodeIDN(const std::string& fqdn) const
      {
        std::string result;
        char *p;
        idna_to_unicode_8z8z(fqdn.c_str(), &p, 0);
        result = p ? p : fqdn;
        if (p) free(p);
        return result;
      }
/*
      virtual ZoneList *getList()
      {
        return &zoneList;
      }
*/
      ///list factory
        virtual ZoneListPtr createList()
        {
            return ZoneListPtr(new ZoneListImpl());
        }

    };//class ManagerImpl

    Manager::ZoneManagerPtr Manager::create()
    {
      TRACE("[CALL] Fred::Zone::Manager::create()");
      return Manager::ZoneManagerPtr(new ManagerImpl);
    }
  };
};

