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

#include "fredlib/contact/merge_contact_selection.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "fredlib/object_states.h"

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
        FilterIdentifiedContact(){};
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
        FilterConditionallyIdentifiedContact(){};

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



}//namespace Fred



