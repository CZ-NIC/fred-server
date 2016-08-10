/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 *  @file
 *  header of contact disclose items
 */

#ifndef DISCLOSE_H_6CCA57EF67A8AD5CE22CBA54280EAEE7//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define DISCLOSE_H_6CCA57EF67A8AD5CE22CBA54280EAEE7

#include <set>
#include <stdexcept>

namespace Epp {

class ContactDisclose
{
public:
    struct Flag
    {
        enum Enum
        {
            hide,
            disclose,
        };
    };
    explicit ContactDisclose(Flag::Enum _meaning):meaning_(_meaning) { }
    bool does_present_item_mean_to_disclose()const
    {
        switch (meaning_)
        {
            case Flag::hide:     return false;
            case Flag::disclose: return true;
        }
        throw std::runtime_error("invalid value of Epp::ContactDisclose::Flag::Enum");
    }
    bool does_present_item_mean_to_hide()const
    {
        return !this->does_present_item_mean_to_disclose();
    }
    struct Item
    {
        enum Enum
        {
            name,
            organization,
            address,
            telephone,
            fax,
            email,
            vat,
            ident,
            notify_email,
        };
    };
    template< Item::Enum ITEM >
    bool presents()const
    {
        const bool item_found = items_.find(ITEM) != items_.end();
        return item_found;
    }
    template< Item::Enum ITEM >
    ContactDisclose& add()
    {
        items_.insert(ITEM);
        return *this;
    }
private:
    const Flag::Enum meaning_;
    std::set< Item::Enum > items_;
};

}//namespace Epp

#endif//DISCLOSE_H_6CCA57EF67A8AD5CE22CBA54280EAEE7
