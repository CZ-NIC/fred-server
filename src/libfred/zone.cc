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

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <ctype.h>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "src/libfred/opcontext.hh"
#include "src/libfred/types.hh"
#include "src/libfred/zone/create_zone.hh"
#include "src/libfred/zone.hh"
#include "src/libfred/zone/update_zone.hh"
#include "src/libfred/zone/zone_ns/create_zone_ns.hh"
#include "src/libfred/zone/zone_ns/update_zone_ns.hh"
#include "src/libfred/zone/zone_soa/create_zone_soa.hh"
#include "src/libfred/zone/zone_soa/update_zone_soa.hh"
#include "src/util/log/context.hh"
#include "src/util/log/logger.hh"
#include "src/util/types/money.hh"


#define RANGE(x) x.begin(),x.end()
#define IS_NUMBER(x) (x >= '0' && x <= '9')
#define IS_LETTER(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))

namespace LibFred
{
  namespace Zone
  {

   class ZoneNsImpl
       : virtual public ZoneNs
    {
    public:
           ZoneNsImpl()
           {
           }

           ZoneNsImpl(TID _id,
                   TID _zone_id,
                   const std::string& _fqdn,
                   const std::string& _addrs)
               : id_(_id),
                 zone_id_(_zone_id),
                 fqdn_(_fqdn),
                 addrs_(_addrs)
           {
           }

           const TID &getId() const
           {
               return id_;
           }

           const TID &getZoneId() const
           {
               return zone_id_;
           }

           const std::string &getFqdn() const
           {
               return fqdn_;
           }

           const std::string &getAddrs() const
           {
               return addrs_;
           }

           void setId(const TID _id)
           {
               id_ = _id;
           }

           void setZoneId(const TID _zone_id)
           {
               zone_id_ = _zone_id;
           }

           void setFqdn(const std::string& _fqdn)
           {
               fqdn_ = _fqdn;
           }

           void setAddrs(const std::string& _addrs)
           {
               addrs_ = _addrs;
           }

           bool operator==(const TID _id) const
           {
             return getId() == _id;
           }

    private:
        TID id_;
        TID zone_id_;
        std::string fqdn_;
        std::string addrs_;

    };//class ZoneNsImpl


    class ZoneImpl : public LibFred::CommonObjectImplNew
                   , virtual public  Zone
    {
    private:
        TID id_;
        std::string fqdn_;
        int ex_period_min_;
        int ex_period_max_;
        int val_period_;
        int dots_max_;
        bool enum_zone_;
        int ttl_;
        std::string hostmaster_;
        int serial_;
        int refresh_;
        int update_retr_;
        int expiry_;
        int minimum_;
        std::string ns_fqdn_;

        typedef std::vector<std::shared_ptr<ZoneNsImpl> > ZoneNsList;
        typedef ZoneNsList::iterator ZoneNsListIter;
        ZoneNsList zone_ns_list; /// zone ns records

        void operator==(const ZoneImpl&){};

    public:
        ZoneImpl()
            : CommonObjectImplNew()
        {}

        ZoneImpl(
                TID _id,
                const std::string& _fqdn,
                const int _ex_period_min,
                const int _ex_period_max,
                const int _val_period,
                const int _dots_max,
                const bool _enum_zone,
                const int _ttl,
                const std::string& _hostmaster,
                const int _serial,
                const int _refresh,
                const int _update_retr,
                const int _expiry,
                const int _minimum,
                const std::string& _ns_fqdn)
            : CommonObjectImplNew(),
              id_(_id),
              fqdn_(_fqdn),
              ex_period_min_(_ex_period_min),
              ex_period_max_(_ex_period_max),
              val_period_(_val_period),
              dots_max_(_dots_max),
              enum_zone_(_enum_zone),

              ttl_(_ttl),
              hostmaster_(_hostmaster),
              serial_(_serial),
              refresh_(_refresh),
              update_retr_(_update_retr),
              expiry_(_expiry),
              minimum_(_minimum),
              ns_fqdn_(_ns_fqdn)
        {
        }

        ~ZoneImpl()
        {}

      /// compare if domain belongs to this zone (according to suffix)
      bool isDomainApplicable(const std::string& fqdn) const
      {
          typedef std::string string;
          // make local copies for normalization (to lower case)
          string zone_fqdn (getFqdn());
          string domain_fqdn (fqdn);

          boost::to_lower(domain_fqdn);
          boost::to_lower(zone_fqdn);

          int domain_length = domain_fqdn.length();
          int zone_length = zone_fqdn.length();

          if (domain_length <= zone_length) {
              return false;
          }

          // substr starts one char before zone length (must include the dot)
          if ( domain_fqdn.substr(domain_length - zone_length - 1)
              ==
              string(".") + zone_fqdn
          ) {
              return true;
          } else {
              return false;
          }
      }

      /// max. number of labels in fqdn
      virtual unsigned getMaxLevel() const
      {
          const std::string& fqdn = getFqdn();
          int maxlevel = std::count(fqdn.begin(), fqdn.end(), '.')
          + 1 + getDotsMax();
          return  maxlevel;
      }

      /// Create new ZoneNs record
      virtual ZoneNs* newZoneNs()
      {
          std::shared_ptr<ZoneNsImpl> new_ZoneNs ( new ZoneNsImpl());
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

      //Zone
      const TID &getId() const
      {
          return id_;
      }

      void setId(const TID _id)
      {
          id_ = _id;
      }

      const std::string &getFqdn() const
      {
          return fqdn_;
      }

      void setFqdn(const std::string& _fqdn)
      {
          fqdn_ = _fqdn;
      }

      const int &getExPeriodMin() const
      {
          return ex_period_min_;
      }

      void setExPeriodMin(const int _ex_period_min)
      {
          ex_period_min_ = _ex_period_min;
      }

      const int &getExPeriodMax() const
      {
          return ex_period_max_;
      }

      void setExPeriodMax(const int _ex_period_max)
      {
          ex_period_max_ = _ex_period_max;
      }

      const int &getValPeriod() const
      {
          return val_period_;
      }

      void setValPeriod(const int _val_period)
      {
          val_period_ = _val_period;
      }

      const int &getDotsMax() const
      {
          return dots_max_;
      }

      void setDotsMax(const int _dots_max)
      {
          dots_max_ = _dots_max;
      }

      const bool &getEnumZone() const
      {
          return enum_zone_;
      }

      const bool &isEnumZone() const
      {
          return getEnumZone();
      }

      void setEnumZone(const bool _enum_zone)
      {
          enum_zone_ = _enum_zone;
      }

      //ZoneSoa
      const int &getTtl() const
      {
          return ttl_;
      }

      const std::string &getHostmaster() const
      {
          return hostmaster_;
      }

      const int &getSerial() const
      {
          return serial_;
      }

      const int &getRefresh() const
      {
          return refresh_;
      }

      const int &getUpdateRetr() const
      {
          return update_retr_;
      }

      const int &getExpiry() const
      {
          return expiry_;
      }

      const int &getMinimum() const
      {
          return minimum_;
      }

      const std::string &getNsFqdn() const
      {
          return ns_fqdn_;
      }

      void setTtl(const int _ttl)
      {
          ttl_ = _ttl;
      }

      void setHostmaster(const std::string& _hostmaster)
      {
          hostmaster_ = _hostmaster;
      }

      void setSerial(const int _serial)
      {
          serial_ = _serial;
      }

      void setRefresh(const int _refresh)
      {
          refresh_ = _refresh;
      }

      void setUpdateRetr(const int _update_retr)
      {
          update_retr_ = _update_retr;
      }

      void setExpiry(const int _expiry)
      {
          expiry_ = _expiry;
      }

      void setMinimum(const int _minimum)
      {
          minimum_ = _minimum;
      }

      void setNsFqdn(const std::string& _ns_fqdn)
      {
          ns_fqdn_ = _ns_fqdn;
      }

      void putZoneNs(TID _id,
                      TID _zoneid,
                  const std::string& fqdn,
                  const std::string& addrs)
      {
        zone_ns_list.push_back(std::make_shared<ZoneNsImpl>(_id,_zoneid,fqdn,addrs));
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
		: public LibFred::CommonListImplNew
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

			  virtual void reload()
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
		          std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
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

		      virtual LibFred::Zone::Zone* findId(Database::ID id) const
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
      ) const
      {
          // ! the last asterisk means only that last label has {0,n} chars with 0 enabling fqdn to end with dot
          //                                    (somelabel.)*(label )
          //                                    |          | |      |
          static const boost::regex fqdn_regex("([^\\.]+\\.)*[^\\.]*");
          static const boost::regex label_regex("[a-z0-9]|[a-z0-9][-a-z0-9]{0,61}[a-z0-9]", boost::regex::icase);
          static const boost::regex punycode_label_regex("xn--[a-z0-9]+(-[a-z0-9]+)*", boost::regex::icase);

          const unsigned fqdn_without_root_dot_lenght = 253;
          const unsigned label_without_dot_lenght = 63;
          const unsigned min_labels_count = 1;

          try {
                // hacking and slashing through string value-space eliminating one invalid fqdn branch after another
                // ... TODO: reversed, more defensive approach would be safer

                if(!boost::regex_match(fqdn, fqdn_regex)) {
                    throw INVALID_DOMAIN_NAME();
                }
                // if there had been ".." at the end of fqdn previous fqdn_regex would have already killed it
                const std::string fqdn_without_root_dot(boost::trim_right_copy_if(fqdn, boost::is_any_of(".")));
                if( fqdn_without_root_dot.size() > fqdn_without_root_dot_lenght ) {
                    throw INVALID_DOMAIN_NAME();
                }

                std::vector<std::string> labels;
                // although ".." would have been found by "fqdn_regex" not using boost::token_compress_on just to be sure
                boost::split(labels, fqdn_without_root_dot, boost::is_any_of(".") /* boost::token_compress_OFF */);

                if( labels.size() < min_labels_count ) {
                    throw INVALID_DOMAIN_NAME();
                }

                // enum (or enum resembling) domains
                if ( checkEnumDomainSuffix(fqdn) ){
                    if( !checkEnumDomainName(labels)) {
                        throw INVALID_DOMAIN_NAME();
                    }

                // non-enum domains
                } else {
                    BOOST_FOREACH(const std::string& label, labels) {
                        if( label.length() > label_without_dot_lenght ) {
                            throw INVALID_DOMAIN_NAME();
                        }
                        if(!boost::regex_match(label, label_regex)) {
                            throw INVALID_DOMAIN_NAME();
                        }
                        if(label.find("--") != std::string::npos) {
                            if(!allowIDN) {
                                throw INVALID_DOMAIN_NAME();
                            } else if(!boost::regex_match(label, punycode_label_regex)) {
                                throw INVALID_DOMAIN_NAME();
                            } else if(!is_valid_punycode(label)) {
                                throw INVALID_DOMAIN_NAME();
                            }
                        }
                    }
                }

                domain = labels;
          } catch(...) {
              throw INVALID_DOMAIN_NAME();
          }
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
        const
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
        if (!loaded) ((LibFred::Zone::ManagerImpl *)this)->load();
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

      void addZone(
              const std::string& _fqdn,
              int _ex_period_min,
              int _ex_period_max,
              int _ttl,
              const std::string& _hostmaster,
              int _refresh,
              int _update_retr,
              int _expiry,
              int _minimum,
              const std::string& _ns_fqdn) final override
      {
        try
        {
            LibFred::OperationContextCreator ctx;

            LibFred::Zone::CreateZone(_fqdn, _ex_period_min, _ex_period_max).exec(ctx);

            LibFred::Zone::CreateZoneSoa(_fqdn)
                    .set_ttl(_ttl)
                    .set_hostmaster(_hostmaster)
                    .set_refresh(_refresh)
                    .set_update_retr(_update_retr)
                    .set_expiry(_expiry)
                    .set_minimum(_minimum)
                    .set_ns_fqdn(_ns_fqdn)
                    .exec(ctx);

            ctx.commit_transaction();
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("addZone: an error has occured");
            throw SQL_ERROR();
        }
      }

      void addZoneNs(
              const std::string& _zone_fqdn,
              const std::string& _nameserver_fqdn,
              const std::string& _nameserver_ip_addresses) final override
      {
          try
          {
              OperationContextCreator ctx;

              if (!_nameserver_ip_addresses.empty())
              {
                  std::vector<std::string> addrs;
                  boost::split(addrs, _nameserver_ip_addresses, boost::is_any_of(","));

                  std::vector<boost::asio::ip::address> ip_addrs;
                  std::for_each(addrs.begin(),
                          addrs.end(),
                          [&ip_addrs](const std::string& s)
                                { ip_addrs.push_back(boost::asio::ip::address::from_string(s)); });

                  CreateZoneNs(_zone_fqdn)
                          .set_nameserver_fqdn(_nameserver_fqdn)
                          .set_nameserver_ip_addresses(ip_addrs)
                          .exec(ctx);
              }
              else
              {
                  CreateZoneNs(_zone_fqdn)
                          .set_nameserver_fqdn(_nameserver_fqdn)
                          .exec(ctx);
              }
              ctx.commit_transaction();
          }
          catch (...)
          {
              LOGGER(PACKAGE).error("addZoneNs: an error has occured");
              throw;
          }
      }

    virtual void addPrice(
            int zone_id,
            const std::string& operation,
            const Database::DateTime &valid_from,
            const Database::DateTime &valid_to,
            const Money &price,
            int quantity,
            bool postpaid_operation_enabled)
    {
        struct AddPriceFailure:std::runtime_error { AddPriceFailure(const std::string &msg):std::runtime_error(msg) { } };
        try
        {
            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);

            const Database::Result res_op = conn.exec_params(
                    "SELECT id FROM enum_operation WHERE operation=$1::TEXT", Database::query_param_list(operation));
            if (res_op.size() != 1)
            {
                throw AddPriceFailure("operation not found");
            }
            const unsigned operation_id = static_cast< unsigned >(res_op[0][0]);

            const Database::Result update_result = conn.exec_params(
                "UPDATE price_list SET valid_to=$1::TIMESTAMP "
                "WHERE zone_id=$2::INTEGER AND "
                      "operation_id=$3::INTEGER AND "
                      "valid_from<$1::TIMESTAMP AND ($1::TIMESTAMP<valid_to OR valid_to IS NULL) "
                "RETURNING 0",//each updated row represents one row in result
                Database::query_param_list(valid_from.iso_str())//$1::TIMESTAMP
                                          (zone_id)             //$2::INTEGER
                                          (operation_id));      //$3::INTEGER
            if (1 < update_result.size())
            {
                throw AddPriceFailure("too many rows updated");
            }

            const Database::Result insert_result = conn.exec_params(
                "INSERT INTO price_list (zone_id,operation_id,valid_from,valid_to,"
                                        "price,quantity,enable_postpaid_operation) "
                "VALUES($1::INTEGER,$2::INTEGER,$3::TIMESTAMP,$4::TIMESTAMP,"
                       "$5::NUMERIC(10,2),$6::INTEGER,$7::BOOLEAN) "
                    "RETURNING 0",//each updated row represents one row in result
                Database::query_param_list(zone_id)                          //$1::INTEGER
                                          (operation_id)                     //$2::INTEGER
                                          (valid_from.iso_str())             //$3::TIMESTAMP
                                          (valid_to.get().is_not_a_date_time()
                                           ? Database::QPNull
                                           : valid_to.iso_str())             //$4::TIMESTAMP
                                          (price)                            //$5::NUMERIC(10,2)
                                          (quantity)                         //$6::INTEGER
                                          (postpaid_operation_enabled));     //$7::BOOLEAN
            if (insert_result.size() != 1)
            {
                throw AddPriceFailure("INSERT failure");
            }

            tx.commit();
        }
        catch (const AddPriceFailure &e)
        {
            LOGGER(PACKAGE).error(std::string("addPrice: ") + e.what());
            throw;
        }
        catch (const std::exception &e)
        {
            LOGGER(PACKAGE).error(std::string("addPrice: ") + e.what());
            throw;
        }
        catch (...)
        {
            LOGGER(PACKAGE).error("addPrice: an error has occurred");
            throw;
        }

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

      virtual bool is_valid_punycode(const std::string& fqdn) const {
          // TODO - prepared for proper check, current implementation is not strict enough

          std::string dev_null;
          try {
              dev_null = punycode_to_utf8(fqdn);
              utf8_to_punycode(dev_null);
          } catch (const idn_conversion_fail& ) {
              return false;
          }

          return true;
      }

      virtual std::string utf8_to_punycode(const std::string& fqdn) const
      {
          char *p = 0;
          std::shared_ptr<char> release_p;

          bool convert_result = (idna_to_ascii_8z(fqdn.c_str(), &p, 0) == IDNA_SUCCESS);
          release_p = std::shared_ptr<char>(p, free);

          if(convert_result) {
              std::string result( p );
              return result;
          } else {
              throw idn_conversion_fail();
          }
      }

      virtual std::string punycode_to_utf8(const std::string& fqdn) const
      {
          char *p = 0;
          std::shared_ptr<char> release_p;

          bool convert_result = (idna_to_unicode_8z8z(fqdn.c_str(), &p, 0) == IDNA_SUCCESS);
          release_p = std::shared_ptr<char>(p, free);

          if(convert_result) {
              std::string result( p );
              return result;
          } else {
              throw idn_conversion_fail();
          }
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
      TRACE("[CALL] LibFred::Zone::Manager::create()");
      return Manager::ZoneManagerPtr(new ManagerImpl);
    }
  };
};

