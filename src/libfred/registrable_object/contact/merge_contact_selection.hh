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
 *  @merge_contact_selection.h
 *  selection of contact for merging
 */

#ifndef MERGE_CONTACT_SELECTION_H_
#define MERGE_CONTACT_SELECTION_H_

#include <string>
#include <vector>

#include "src/libfred/opexception.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/log/log.hh"
#include "src/util/factory.hh"
#include "src/util/factory_check.hh"
#include "src/util/printable.hh"

namespace LibFred
{
    FACTORY_MODULE_INIT_DECL(merge_contact_selection)

    typedef std::string ContactSelectionFilterType;

    const ContactSelectionFilterType MCS_FILTER_IDENTIFIED_CONTACT = "mcs_filter_identified_contact";
    const ContactSelectionFilterType MCS_FILTER_CONDITIONALLY_IDENTIFIED_CONTACT = "mcs_filter_conditionally_identified_contact";
    const ContactSelectionFilterType MCS_FILTER_HANDLE_MOJEID_SYNTAX = "mcs_filter_handle_mojeid_syntax";
    const ContactSelectionFilterType MCS_FILTER_MAX_DOMAINS_BOUND = "mcs_filter_max_domains_bound";
    const ContactSelectionFilterType MCS_FILTER_MAX_OBJECTS_BOUND = "mcs_filter_max_objects_bound";
    const ContactSelectionFilterType MCS_FILTER_RECENTLY_UPDATED = "mcs_filter_recently_updated";
    const ContactSelectionFilterType MCS_FILTER_NOT_REGCZNIC  = "mcs_filter_not_regcznic";
    const ContactSelectionFilterType MCS_FILTER_RECENTLY_CREATED = "mcs_filter_recently_created";

    class ContactSelectionFilterBase
    {
    public:
      virtual ~ContactSelectionFilterBase(){}
      virtual std::vector<std::string> operator()(OperationContext& ctx, const std::vector<std::string>& contact_handle) = 0;
    };


    struct MergeContactSelectionOutput : public Util::Printable
    {
        std::string handle;
        ContactSelectionFilterType filter;

        MergeContactSelectionOutput(
                const std::string &_handle,
                const ContactSelectionFilterType &_filter)
            : handle(_handle),
              filter(_filter)
        {}

        std::string to_string() const
        {
            return Util::format_data_structure("MergeContactSelectionOutput",
            Util::vector_of<std::pair<std::string,std::string> >
            (std::make_pair("handle",handle))
            (std::make_pair("filter",filter))
            );
        }
    };

    class MergeContactSelection : public Util::Printable
    {
        std::vector<std::string> contact_handle_;//contact handle vector
        std::vector<std::pair<std::string, std::shared_ptr<ContactSelectionFilterBase> > > ff_;//filter functor ptr vector
    public:

        struct NoContactHandles
        : virtual LibFred::OperationException
        {
            const char* what() const noexcept {return "no contact handles, nothing to process";}
        };
        struct NoContactHandlesLeft
        : virtual LibFred::OperationException
        {
            const char* what() const noexcept {return "no contact handles left, selection of contact with given rules failed";}
        };
        struct TooManyContactHandlesLeft
        : virtual LibFred::OperationException
        {
            const char* what() const noexcept {return "too many contact handles left, selection of contact with given rules failed";}
        };

        MergeContactSelection(const std::vector<std::string>& contact_handle
                , const std::vector<ContactSelectionFilterType>& filter);
        MergeContactSelectionOutput exec(OperationContext& ctx);

        std::string to_string() const;
    };//class MergeContactSelection


    typedef Util::Factory<ContactSelectionFilterBase, Util::ClassCreator<ContactSelectionFilterBase> > ContactSelectionFilterFactory;

} // namespace LibFred
#endif //MERGE_CONTACT_SELECTION_H_

