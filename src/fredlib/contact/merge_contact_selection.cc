/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @merge_contact_selection.cc
 *  selection of contact for merging
 */

#include <string>

#include <algorithm>
#include <functional>
#include <stdexcept>


#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

#include "fredlib/contact/merge_contact_selection.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "fredlib/object_states.h"

#include "util/util.h"

namespace Fred
{

    FACTORY_MODULE_INIT_DEFI(merge_contact_selection)

    MergeContactSelection::MergeContactSelection(const std::vector<std::string>& contact_handle
        , const std::vector<ContactSelectionFilterType>& filter)
    : contact_handle_(contact_handle)
    {
        for(std::vector<ContactSelectionFilterType>::const_iterator ci = filter.begin()
                ; ci !=  filter.end(); ++ci)
        {
            ContactSelectionFilterBase* filter = dynamic_cast<ContactSelectionFilterBase*>(
                                ContactSelectionFilterFactory::instance_ref().create(*ci));
            ff_.push_back(boost::shared_ptr<ContactSelectionFilterBase> (
                    filter));
        }
    }

    std::string MergeContactSelection::exec(OperationContext& ctx)
    {
        if(contact_handle_.empty())
        {
            throw std::logic_error("no contact handles");
        }
        for(std::vector<boost::shared_ptr<ContactSelectionFilterBase> >::iterator f = ff_.begin(); f != ff_.end(); ++f)
        {
            std::vector<std::string> current_filter_result;
            if(f->get() != 0) current_filter_result = f->get()->operator()(ctx,contact_handle_);
            if(current_filter_result.size() > 1) contact_handle_ = current_filter_result;
            if(current_filter_result.size() == 1) return current_filter_result[0];
            //if(current_filter_result.empty()) continue;//try next filter
        }

        if(contact_handle_.empty())
        {
            throw std::logic_error("no contact handles left");
        }
        else
        {
            throw std::logic_error("to many contact handles left");
        }
    }

    class FilterIdentifiedContact
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterIdentifiedContact>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {

            std::vector<std::string> filtered;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                Database::Result contact_check = ctx.get_conn().exec_params(
                "SELECT oreg.name FROM contact c JOIN object_registry oreg ON c.id = oreg.id "
                " JOIN object_state os ON os.object_id = oreg.id AND os.valid_to IS NULL "
                " JOIN enum_object_states eos ON eos.id = os.state_id "
                " WHERE eos.name = $1::text AND oreg.name = $2::text"
                , Database::query_param_list (Fred::ObjectState::IDENTIFIED_CONTACT)(*i));

                if(contact_check.size() == 1) filtered.push_back(*i);
            }
            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_IDENTIFIED_CONTACT;
        }
    };//class FilterIdentifiedContact

    class FilterConditionallyIdentifiedContact
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterConditionallyIdentifiedContact>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {
            std::vector<std::string> filtered;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                Database::Result contact_check = ctx.get_conn().exec_params(
                "SELECT oreg.name FROM contact c JOIN object_registry oreg ON c.id = oreg.id "
                " JOIN object_state os ON os.object_id = oreg.id AND os.valid_to IS NULL "
                " JOIN enum_object_states eos ON eos.id = os.state_id "
                " WHERE eos.name = $1::text AND oreg.name = $2::text"
                , Database::query_param_list (Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT)(*i));

                if(contact_check.size() == 1) filtered.push_back(*i);
            }
            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT;
        }
    };//class FilterConditionallyIdentifiedContact

    class FilterHandleMojeIDSyntax
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterHandleMojeIDSyntax>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {
            boost::regex mojeid_handle_syntax("[a-zA-Z0-9_:.-]{1,63}");//unable to use private constant in contact.cc
            std::vector<std::string> filtered;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                try
                {
                    if(boost::regex_match(*i,mojeid_handle_syntax))
                    {
                        filtered.push_back(*i);
                    }
                }
                catch(std::exception& ex)
                {
                    //report regex_match failure
                    std::string errmsg("FilterHandleMojeIDSyntax: ");
                    errmsg += ex.what();
                    ctx.get_log().error(errmsg);
                }
            }
            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_HANDLE_MOJEID_SYNTAX;
        }
    };//class FilterHandleMojeIDSyntax

    class FilterMaxDomainsBound
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterMaxDomainsBound>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {
            std::vector<std::string> filtered;

            if(contact_handle.empty()) return filtered;

            std::string query_begin("SELECT cc.handle, (cc.domain_registrant_count + cc.domain_admin_count) AS all_domains_count FROM ( "
            " SELECT current_contact.handle "
            " , (SELECT count(*) FROM object_registry oreg JOIN domain d ON oreg.id = d.id WHERE d.registrant = current_contact.id) AS domain_registrant_count "
            " , (SELECT count(*) FROM object_registry oreg JOIN domain d ON oreg.id = d.id JOIN domain_contact_map dcm ON dcm.domainid = d.id and dcm.role = 1 "
            "    WHERE dcm.contactid  = current_contact.id) AS domain_admin_count "
            " FROM (SELECT oreg.name AS handle, c.id AS id FROM contact c JOIN object_registry oreg ON c.id = oreg.id ");

            Util::HeadSeparator where_or(" WHERE "," OR ");

            std::string query_end("     ) AS current_contact "
            " ) cc "
            " ORDER BY all_domains_count DESC ");

            Database::QueryParams params;//query params
            std::stringstream sql;
            sql << query_begin;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                params.push_back(*i);
                sql << where_or.get() << "UPPER(oreg.name) = UPPER($" << params.size() << "::text) ";
            }
            sql << query_end;

            Database::Result contact_domains = ctx.get_conn().exec_params(sql.str(), params);

            for(Database::Result::size_type i = 0 ; i < contact_domains.size(); ++i)
            {
                //if it is first contact with maximum of domains bound or another contact with the same maximum number of domains bound
                if((i == 0) || (std::string(contact_domains[0][1]).compare(std::string(contact_domains[i][1])) == 0 ))
                {
                    filtered.push_back(std::string(contact_domains[i][0]));
                }
                else
                {//ignore others
                    break;
                }
            }

            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_MAX_DOMAINS_BOUND;
        }
    };//class FilterMaxDomainsBound

    class FilterMaxObjectsBound
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterMaxObjectsBound>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {
            std::vector<std::string> filtered;

            if(contact_handle.empty()) return filtered;

            std::string query_begin("SELECT cc.handle, (cc.domain_registrant_count + cc.domain_admin_count + cc.nsset_tech_count + cc.keyset_tech_count) AS all_domains_count FROM ( "
            " SELECT current_contact.handle "
            " , (SELECT count(*) FROM object_registry oreg JOIN domain d ON oreg.id = d.id WHERE d.registrant = current_contact.id) AS domain_registrant_count "
            " , (SELECT count(*) FROM object_registry oreg JOIN domain d ON oreg.id = d.id JOIN domain_contact_map dcm ON dcm.domainid = d.id and dcm.role = 1 "
            "    WHERE dcm.contactid  = current_contact.id) AS domain_admin_count "
            " , (SELECT count(*) FROM object_registry oreg JOIN nsset n ON oreg.id = n.id JOIN nsset_contact_map ncm ON ncm.nssetid = n.id "
            "    WHERE ncm.contactid  = current_contact.id) AS nsset_tech_count "
            " , (SELECT count(*) FROM object_registry oreg JOIN keyset k ON oreg.id = k.id JOIN keyset_contact_map kcm ON kcm.keysetid = k.id "
            "    WHERE kcm.contactid  = current_contact.id) AS keyset_tech_count "
            " FROM (SELECT oreg.name AS handle, c.id AS id FROM contact c JOIN object_registry oreg ON c.id = oreg.id ");

            Util::HeadSeparator where_or(" WHERE "," OR ");

            std::string query_end("     ) AS current_contact "
            " ) cc "
            " ORDER BY all_domains_count DESC ");

            Database::QueryParams params;//query params
            std::stringstream sql;
            sql << query_begin;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                params.push_back(*i);
                sql << where_or.get() << "UPPER(oreg.name) = UPPER($" << params.size() << "::text) ";
            }
            sql << query_end;

            Database::Result contact_objects = ctx.get_conn().exec_params(sql.str(), params);

            for(Database::Result::size_type i = 0 ; i < contact_objects.size(); ++i)
            {
                //if it is first contact with maximum of objects bound or another contact with the same maximum number of objects bound
                if((i == 0) || (std::string(contact_objects[0][1]).compare(std::string(contact_objects[i][1])) == 0 ))
                {
                    filtered.push_back(std::string(contact_objects[i][0]));
                }
                else
                {//ignore others
                    break;
                }
            }

            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_MAX_OBJECTS_BOUND;
        }
    };//class FilterMaxObjectsBound

    class FilterRecentlyUpdated
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterRecentlyUpdated>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {
            std::vector<std::string> filtered;

            if(contact_handle.empty()) return filtered;

            std::string query_begin("SELECT oreg.name, o.update FROM object o "
                    " JOIN object_registry oreg ON o.id = oreg.id AND o.update IS NOT NULL "
                    " JOIN contact c ON c.id = oreg.id ");

            Util::HeadSeparator where_or(" WHERE "," OR ");

            std::string query_end(" ORDER BY o.update DESC ");

            Database::QueryParams params;//query params
            std::stringstream sql;
            sql << query_begin;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                params.push_back(*i);
                sql << where_or.get() << "UPPER(oreg.name) = UPPER($" << params.size() << "::text) ";
            }
            sql << query_end;

            Database::Result contact_updated = ctx.get_conn().exec_params(sql.str(), params);

            for(Database::Result::size_type i = 0 ; i < contact_updated.size(); ++i)
            {
                //if it is first contact with most recent update timestamp or another contact with the same update timestamp
                if((i == 0) || (std::string(contact_updated[0][1]).compare(std::string(contact_updated[i][1])) == 0 ))
                {
                    filtered.push_back(std::string(contact_updated[i][0]));
                }
                else
                {//ignore others
                    break;
                }
            }

            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_RECENTLY_UPDATED;
        }
    };//class FilterRecentlyUpdated

    class FilterNotRegCzNic
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterNotRegCzNic>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {
            std::vector<std::string> filtered;

            std::string query(
                "SELECT oreg.name FROM object_registry oreg "
                " JOIN object o ON oreg.id = o.id "
                " JOIN registrar r ON r.id = o.clid AND UPPER(r.handle) != UPPER('REG-CZNIC') "
                " JOIN contact c ON c.id = oreg.id ");

            Util::HeadSeparator where_or(" WHERE "," OR ");

            Database::QueryParams params;//query params
            std::stringstream sql;
            sql << query;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                params.push_back(*i);
                sql << where_or.get() << "UPPER(oreg.name) = UPPER($" << params.size() << "::text) ";
            }

            Database::Result contact_not_regcznic = ctx.get_conn().exec_params(sql.str(), params);

            for(Database::Result::size_type i = 0 ; i < contact_not_regcznic.size(); ++i)
            {
                    filtered.push_back(std::string(contact_not_regcznic[i][0]));
            }

            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_NOT_REGCZNIC;
        }
    };//class FilterNotRegCzNic

    class FilterRecentlyCreated
    : public ContactSelectionFilterBase
    , public Util::FactoryAutoRegister<ContactSelectionFilterBase, FilterRecentlyCreated>
    {
    public:
        std::vector<std::string> operator()(OperationContext& ctx
                , const std::vector<std::string>& contact_handle)
        {
            std::vector<std::string> filtered;

            if(contact_handle.empty()) return filtered;

            std::string query_begin("SELECT oreg.name, oreg.crdate "
                    " FROM object_registry oreg "
                    " JOIN contact c ON c.id = oreg.id ");

            Util::HeadSeparator where_or(" WHERE "," OR ");

            std::string query_end(" ORDER BY oreg.crdate DESC ");

            Database::QueryParams params;//query params
            std::stringstream sql;
            sql << query_begin;
            for(std::vector<std::string>::const_iterator i = contact_handle.begin(); i != contact_handle.end() ; ++i)
            {
                params.push_back(*i);
                sql << where_or.get() << "UPPER(oreg.name) = UPPER($" << params.size() << "::text) ";
            }
            sql << query_end;

            Database::Result contact_created = ctx.get_conn().exec_params(sql.str(), params);

            for(Database::Result::size_type i = 0 ; i < contact_created.size(); ++i)
            {
                //if it is first contact with most recent create timestamp or another contact with the same create timestamp
                if((i == 0) || (std::string(contact_created[0][1]).compare(std::string(contact_created[i][1])) == 0 ))
                {
                    filtered.push_back(std::string(contact_created[i][0]));
                }
                else
                {//ignore others
                    break;
                }
            }

            return filtered;
        }

        static ContactSelectionFilterType registration_name()
        {
            return MCS_FILTER_RECENTLY_CREATED;
        }
    };//class FilterRecentlyCreated


}//namespace Fred



