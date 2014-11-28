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
 *  @file
 * contact info data diff
 */

#include <algorithm>
#include <string>

#include <boost/algorithm/string.hpp>

#include "util/util.h"
#include "util/is_equal_optional_nullable.h"
#include "info_contact_diff.h"

namespace Fred
{
    InfoContactDiff::InfoContactDiff()
    {}

    std::set<std::string> InfoContactDiff::changed_fields() const
    {
        std::set<std::string> fields;
        if(crhistoryid.isset()) fields.insert("crhistoryid");
        if(historyid.isset()) fields.insert("historyid");
        if(delete_time.isset()) fields.insert("delete_time");
        if(handle.isset()) fields.insert("handle");
        if(roid.isset()) fields.insert("roid");
        if(sponsoring_registrar_handle.isset()) fields.insert("sponsoring_registrar_handle");
        if(create_registrar_handle.isset()) fields.insert("create_registrar_handle");
        if(update_registrar_handle.isset()) fields.insert("update_registrar_handle");
        if(creation_time.isset()) fields.insert("creation_time");
        if(update_time.isset()) fields.insert("update_time");
        if(transfer_time.isset()) fields.insert("transfer_time");
        if(authinfopw.isset()) fields.insert("authinfopw");
        if(name.isset()) fields.insert("name");
        if(organization.isset()) fields.insert("organization");
        if(place.isset()) fields.insert("place");
        if(telephone.isset()) fields.insert("telephone");
        if(fax.isset()) fields.insert("fax");
        if(email.isset()) fields.insert("email");
        if(notifyemail.isset()) fields.insert("notifyemail");
        if(vat.isset()) fields.insert("vat");
        if(ssntype.isset()) fields.insert("ssntype");
        if(ssn.isset()) fields.insert("ssn");
        if(disclosename.isset()) fields.insert("disclosename");
        if(discloseorganization.isset()) fields.insert("discloseorganization");
        if(discloseaddress.isset()) fields.insert("discloseaddress");
        if(disclosetelephone.isset()) fields.insert("disclosetelephone");
        if(disclosefax.isset()) fields.insert("disclosefax");
        if(discloseemail.isset()) fields.insert("discloseemail");
        if(disclosevat.isset()) fields.insert("disclosevat");
        if(discloseident.isset()) fields.insert("discloseident");
        if(disclosenotifyemail.isset()) fields.insert("disclosenotifyemail");
        if(id.isset()) fields.insert("id");
        if(addresses.isset()) fields.insert("addresses");

        return  fields;
    }

    // náhrada za nefunkční DiffMemeber<Fred::ContactAddressList>::Type::print_quoted()
    template < class T1, class T2 > std::string format_optional_pair(const Optional< std::pair< T1, T2 > > &_in)
    {
        if (!_in.isset()) {
            return "[N/A]";
        }
        std::ostringstream out;
        out << "'first: " << _in.get_value().first << " second: " << _in.get_value().second << "'";
        return out.str();
    }

    std::string InfoContactDiff::to_string() const
    {
        return Util::format_data_structure("InfoContactDiff",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("crhistoryid", crhistoryid.print_quoted()))
        (std::make_pair("historyid", historyid.print_quoted()))
        (std::make_pair("delete_time", delete_time.print_quoted()))
        (std::make_pair("handle", handle.print_quoted()))
        (std::make_pair("roid", roid.print_quoted()))
        (std::make_pair("sponsoring_registrar_handle", sponsoring_registrar_handle.print_quoted()))
        (std::make_pair("create_registrar_handle", create_registrar_handle.print_quoted()))
        (std::make_pair("update_registrar_handle", update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time", creation_time.print_quoted()))
        (std::make_pair("update_time", update_time.print_quoted()))
        (std::make_pair("transfer_time", transfer_time.print_quoted()))
        (std::make_pair("authinfopw", authinfopw.print_quoted()))
        (std::make_pair("name", name.print_quoted()))
        (std::make_pair("organization", organization.print_quoted()))
        (std::make_pair("place", place.print_quoted()))
        (std::make_pair("telephone", telephone.print_quoted()))
        (std::make_pair("fax", fax.print_quoted()))
        (std::make_pair("email", email.print_quoted()))
        (std::make_pair("notifyemail", notifyemail.print_quoted()))
        (std::make_pair("vat", vat.print_quoted()))
        (std::make_pair("ssntype", ssntype.print_quoted()))
        (std::make_pair("ssn", ssn.print_quoted()))
        (std::make_pair("disclosename", disclosename.print_quoted()))
        (std::make_pair("discloseorganization", discloseorganization.print_quoted()))
        (std::make_pair("discloseaddress", discloseaddress.print_quoted()))
        (std::make_pair("disclosetelephone", disclosetelephone.print_quoted()))
        (std::make_pair("disclosefax", disclosefax.print_quoted()))
        (std::make_pair("discloseemail", discloseemail.print_quoted()))
        (std::make_pair("disclosevat", disclosevat.print_quoted()))
        (std::make_pair("discloseident", discloseident.print_quoted()))
        (std::make_pair("disclosenotifyemail", disclosenotifyemail.print_quoted()))
        (std::make_pair("id", id.print_quoted()))
        // z nějakého, mně záhadného, důvodu tady nefunguje addresses.print_quoted() !?
        // obešel jsem to tedy tímhle hackem
        (std::make_pair("addresses", format_optional_pair(addresses)))
        );//format_data_structure InfoContactDiff
    }

    bool InfoContactDiff::is_empty() const
    {
        return
            !( crhistoryid.isset()
            || historyid.isset()
            || delete_time.isset()
            || handle.isset()
            || roid.isset()
            || sponsoring_registrar_handle.isset()
            || create_registrar_handle.isset()
            || update_registrar_handle.isset()
            || creation_time.isset()
            || update_time.isset()
            || transfer_time.isset()
            || authinfopw.isset()
            || name.isset()
            || organization.isset()
            || place.isset()
            || telephone.isset()
            || fax.isset()
            || email.isset()
            || notifyemail.isset()
            || vat.isset()
            || ssntype.isset()
            || ssn.isset()
            || disclosename.isset()
            || discloseorganization.isset()
            || discloseaddress.isset()
            || disclosetelephone.isset()
            || disclosefax.isset()
            || discloseemail.isset()
            || disclosevat.isset()
            || discloseident.isset()
            || disclosenotifyemail.isset()
            || id.isset()
            || addresses.isset()
            );
    }

    namespace
    {
        bool operator==(const ContactAddressList&, const ContactAddressList&);
        bool operator!=(const ContactAddressList &a, const ContactAddressList &b) { return !(a == b); }
    }

    InfoContactDiff diff_contact_data(const InfoContactData& first, const InfoContactData& second)
    {
        Fred::InfoContactDiff diff;

        //differing data
        if(first.crhistoryid != second.crhistoryid)
        {
            diff.crhistoryid = std::make_pair(first.crhistoryid,second.crhistoryid);
        }

        if(first.historyid != second.historyid)
        {
            diff.historyid = std::make_pair(first.historyid,second.historyid);
        }

        if(!Util::is_equal(first.delete_time, second.delete_time))
        {
            diff.delete_time = std::make_pair(first.delete_time,second.delete_time);
        }

        if(boost::algorithm::to_upper_copy(first.handle)
            .compare(boost::algorithm::to_upper_copy(second.handle)) != 0)
        {
            diff.handle = std::make_pair(first.handle, second.handle);
        }

        if(first.roid.compare(second.roid) != 0)
        {
            diff.roid = std::make_pair(first.roid,second.roid);
        }

        if(boost::algorithm::to_upper_copy(first.sponsoring_registrar_handle)
            .compare(boost::algorithm::to_upper_copy(second.sponsoring_registrar_handle)) != 0)
        {
            diff.sponsoring_registrar_handle = std::make_pair(first.sponsoring_registrar_handle
                    ,second.sponsoring_registrar_handle);
        }

        if(boost::algorithm::to_upper_copy(first.create_registrar_handle)
        .compare(boost::algorithm::to_upper_copy(second.create_registrar_handle)) != 0)
        {
            diff.create_registrar_handle = std::make_pair(first.create_registrar_handle
                    ,second.create_registrar_handle);
        }

        if(!Util::is_equal_upper(first.update_registrar_handle, second.update_registrar_handle))
        {
            diff.update_registrar_handle = std::make_pair(first.update_registrar_handle
                    ,second.update_registrar_handle);
        }

        if(first.creation_time != second.creation_time)
        {
            diff.creation_time = std::make_pair(first.creation_time,second.creation_time);
        }

        if(!Util::is_equal(first.update_time, second.update_time))
        {
            diff.update_time = std::make_pair(first.update_time
                    ,second.update_time);
        }

        if(!Util::is_equal(first.transfer_time, second.transfer_time))
        {
            diff.transfer_time = std::make_pair(first.transfer_time
                    ,second.transfer_time);
        }

        if(first.authinfopw.compare(second.authinfopw) != 0)
        {
            diff.authinfopw = std::make_pair(first.authinfopw,second.authinfopw);
        }

        if(!Util::is_equal(first.name, second.name))
        {
            diff.name = std::make_pair(first.name,second.name);
        }

        if(!Util::is_equal(first.organization, second.organization))
        {
            diff.organization = std::make_pair(first.organization,second.organization);
        }

        if(!Util::is_equal(first.place, second.place))
        {
            diff.place = std::make_pair(first.place, second.place);
        }

        if(!Util::is_equal(first.telephone, second.telephone))
        {
            diff.telephone = std::make_pair(first.telephone,second.telephone);
        }

        if(!Util::is_equal(first.fax, second.fax))
        {
            diff.fax = std::make_pair(first.fax,second.fax);
        }

        if(!Util::is_equal(first.email, second.email))
        {
            diff.email = std::make_pair(first.email,second.email);
        }

        if(!Util::is_equal(first.notifyemail, second.notifyemail))
        {
            diff.notifyemail = std::make_pair(first.notifyemail,second.notifyemail);
        }

        if(!Util::is_equal(first.vat, second.vat))
        {
            diff.vat = std::make_pair(first.vat,second.vat);
        }

        if(!Util::is_equal(first.ssntype, second.ssntype))
        {
            diff.ssntype = std::make_pair(first.ssntype,second.ssntype);
        }

        if(!Util::is_equal(first.ssn, second.ssn))
        {
            diff.ssn = std::make_pair(first.ssn,second.ssn);
        }

        if(first.disclosename != second.disclosename)
        {
            diff.disclosename = std::make_pair(first.disclosename,second.disclosename);
        }

        if(first.discloseorganization != second.discloseorganization)
        {
            diff.discloseorganization = std::make_pair(first.discloseorganization,second.discloseorganization);
        }

        if(first.discloseaddress != second.discloseaddress)
        {
            diff.discloseaddress = std::make_pair(first.discloseaddress,second.discloseaddress);
        }

        if(first.disclosetelephone != second.disclosetelephone)
        {
            diff.disclosetelephone = std::make_pair(first.disclosetelephone,second.disclosetelephone);
        }

        if(first.disclosefax != second.disclosefax)
        {
            diff.disclosefax = std::make_pair(first.disclosefax,second.disclosefax);
        }

        if(first.discloseemail != second.discloseemail)
        {
            diff.discloseemail = std::make_pair(first.discloseemail,second.discloseemail);
        }

        if(first.disclosevat != second.disclosevat)
        {
            diff.disclosevat = std::make_pair(first.disclosevat,second.disclosevat);
        }

        if(first.discloseident != second.discloseident)
        {
            diff.discloseident = std::make_pair(first.discloseident,second.discloseident);
        }

        if(first.disclosenotifyemail != second.disclosenotifyemail)
        {
            diff.disclosenotifyemail = std::make_pair(first.disclosenotifyemail,second.disclosenotifyemail);
        }

        if(first.id != second.id)
        {
            diff.id = std::make_pair(first.id,second.id);
        }

        if(first.addresses != second.addresses)
        {
            diff.addresses = std::make_pair(first.addresses, second.addresses);
        }

        return diff;
    }


    namespace
    {
        bool operator==(const ContactAddressList &_a, const ContactAddressList &_b)
        {
            if (_a.size() != _b.size()) {
                return false;
            }
            ContactAddressList::const_iterator pa = _a.begin();
            ContactAddressList::const_iterator pb = _b.begin();
            while (true) {
                if (pa == _a.end()) {
                    return pb == _b.end();
                }
                if (pb == _b.end()) {
                    return false;
                }
                if ((pa->first != pb->first) ||
                    (pa->second != pb->second)) {
                    return false;
                }
                ++pa;
                ++pb;
            }
        }

    }

}//namespace Fred
