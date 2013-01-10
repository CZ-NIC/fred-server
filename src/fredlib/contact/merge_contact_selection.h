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

#include <boost/function.hpp>

#include "fredlib/opcontext.h"
#include "util/log/log.h"

namespace Fred
{
    class MergeContactSelection
    {
    public:
        typedef boost::function<std::vector<std::string> (OperationContext& ctx
                , const std::vector<std::string>& contact_handle)> FilterFunctor;
    private:
        std::vector<std::string> contact_handle_;//contact handle vector
        std::vector<FilterFunctor> ff_;//filter functor vector
    public:
        MergeContactSelection(const std::vector<std::string>& contact_handle
                , const std::vector<FilterFunctor>& ff);
        std::string exec(OperationContext& ctx);
    };//class MergeContactSelection

    class FilterIdentifiedContact
    {
    public:
        FilterIdentifiedContact();
        std::vector<std::string> operator()(OperationContext& ctx, const std::vector<std::string>& contact_handle);
    };//class FilterIdentifiedContact

    class FilterConditionallyIdentifiedContact
    {
    public:
        FilterConditionallyIdentifiedContact();
        std::vector<std::string> operator()(OperationContext& ctx, const std::vector<std::string>& contact_handle);
    };//class FilterConditionallyIdentifiedContact

}//namespace Fred
#endif //MERGE_CONTACT_SELECTION_H_

