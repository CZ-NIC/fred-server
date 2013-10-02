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
 *  common contact info data
 */

#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/contact/info_contact_data.h"

namespace Fred
{

    InfoContactData::InfoContactData()
    : crhistoryid(0)
    , historyid(0)
    , id(0)
    , print_diff_(false)
    {}

    bool InfoContactData::operator==(const InfoContactData& rhs) const
    {
        if(print_diff_) std::cout << "\nInfoContactData::operator== print_diff\n" << std::endl;

        bool result_roid = (roid.compare(rhs.roid) == 0);
        if(print_diff_ && !result_roid) std::cout << "roid: " << roid << " != "<< rhs.roid << std::endl;

        bool result_handle = (boost::algorithm::to_upper_copy(handle).compare(boost::algorithm::to_upper_copy(rhs.handle)) == 0);
        if(print_diff_ && !result_handle) std::cout << "handle: " << handle << " != "<< rhs.handle << std::endl;

        bool result_sponsoring_registrar_handle = (boost::algorithm::to_upper_copy(sponsoring_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.sponsoring_registrar_handle)) == 0);
        if(print_diff_ && !result_sponsoring_registrar_handle) std::cout << "sponsoring_registrar_handle: " << sponsoring_registrar_handle << " != "<< rhs.sponsoring_registrar_handle << std::endl;

        bool result_create_registrar_handle = (boost::algorithm::to_upper_copy(create_registrar_handle).compare(boost::algorithm::to_upper_copy(rhs.create_registrar_handle)) == 0);
        if(print_diff_ && !result_create_registrar_handle) std::cout << "create_registrar_handle: " << create_registrar_handle << " != "<< rhs.create_registrar_handle << std::endl;

        bool result_creation_time = (creation_time == rhs.creation_time);
        if(print_diff_ && !result_creation_time) std::cout << "creation_time: " << creation_time << " != "<< rhs.creation_time << std::endl;

        bool result_authinfopw = (authinfopw.compare(rhs.authinfopw) == 0);
        if(print_diff_ && !result_authinfopw) std::cout << "authinfopw: " << authinfopw << " != "<< rhs.authinfopw << std::endl;

        bool result_historyid = (historyid == rhs.historyid);
        if(print_diff_ && !result_historyid) std::cout << "historyid: " << historyid << " != "<< rhs.historyid << std::endl;

        bool result_crhistoryid = (crhistoryid == rhs.crhistoryid);
        if(print_diff_ && !result_crhistoryid) std::cout << "crhistoryid: " << crhistoryid << " != "<< rhs.crhistoryid << std::endl;

        bool result_update_registrar_handle = (update_registrar_handle.isnull() == rhs.update_registrar_handle.isnull());
        if(!update_registrar_handle.isnull() && !rhs.update_registrar_handle.isnull())
        {
            result_update_registrar_handle = (boost::algorithm::to_upper_copy(std::string(update_registrar_handle))
            .compare(boost::algorithm::to_upper_copy(std::string(rhs.update_registrar_handle))) == 0);
        }
        if(print_diff_ && !result_update_registrar_handle) std::cout << "update_registrar_handle: " << update_registrar_handle.print_quoted() << " != "<< rhs.update_registrar_handle.print_quoted() << std::endl;

        bool result_update_time = (update_time.isnull() == rhs.update_time.isnull());
        if(!update_time.isnull() && !rhs.update_time.isnull())
        {
            result_update_time = (boost::posix_time::ptime(update_time) == boost::posix_time::ptime(rhs.update_time));
        }
        if(print_diff_ && !result_update_time) std::cout << "update_time: " << update_time.print_quoted() << " != "<< rhs.update_time.print_quoted() << std::endl;

        bool result_transfer_time = (transfer_time.isnull() == rhs.transfer_time.isnull());
        if(!transfer_time.isnull() && !rhs.transfer_time.isnull())
        {
            result_transfer_time = (boost::posix_time::ptime(transfer_time) == boost::posix_time::ptime(rhs.transfer_time));
        }
        if(print_diff_ && !result_transfer_time) std::cout << "transfer_time: " << transfer_time.print_quoted() << " != "<< rhs.transfer_time.print_quoted() << std::endl;

        bool result_delete_time = (delete_time.isnull() == rhs.delete_time.isnull());
        if(!delete_time.isnull() && !rhs.delete_time.isnull())
        {
            result_delete_time = (boost::posix_time::ptime(delete_time) == boost::posix_time::ptime(rhs.delete_time));
        }
        if(print_diff_ && !result_delete_time) std::cout << "delete_time: " << delete_time.print_quoted() << " != "<< rhs.delete_time.print_quoted() << std::endl;

        bool result_name = (name.isnull() == rhs.name.isnull());
        if(!name.isnull() && !rhs.name.isnull())
        {
            result_name = (static_cast<std::string>(name).compare(static_cast<std::string>(rhs.name)) == 0);
        }
        if(print_diff_ && !result_name) std::cout << "name: " << name.print_quoted() << " != "<< rhs.name.print_quoted() << std::endl;

        bool result_organization = (organization.isnull() == rhs.organization.isnull());
        if(!organization.isnull() && !rhs.organization.isnull())
        {
            result_organization = (static_cast<std::string>(organization).compare(static_cast<std::string>(rhs.organization)) == 0);
        }
        if(print_diff_ && !result_organization) std::cout << "organization: " << organization.print_quoted() << " != "<< rhs.organization.print_quoted() << std::endl;

        bool result_street1 = (street1.isnull() == rhs.street1.isnull());
        if(!street1.isnull() && !rhs.street1.isnull())
        {
            result_street1 = (static_cast<std::string>(street1).compare(static_cast<std::string>(rhs.street1)) == 0);
        }
        if(print_diff_ && !result_street1) std::cout << "street1: " << street1.print_quoted() << " != "<< rhs.street1.print_quoted() << std::endl;

        bool result_street2 = (street2.isnull() == rhs.street2.isnull());
        if(!street2.isnull() && !rhs.street2.isnull())
        {
            result_street2 = (static_cast<std::string>(street2).compare(static_cast<std::string>(rhs.street2)) == 0);
        }
        if(print_diff_ && !result_street2) std::cout << "street2: " << street2.print_quoted() << " != "<< rhs.street2.print_quoted() << std::endl;

        bool result_street3 = (street3.isnull() == rhs.street3.isnull());
        if(!street3.isnull() && !rhs.street3.isnull())
        {
            result_street3 = (static_cast<std::string>(street3).compare(static_cast<std::string>(rhs.street3)) == 0);
        }
        if(print_diff_ && !result_street3) std::cout << "street3: " << street3.print_quoted() << " != "<< rhs.street3.print_quoted() << std::endl;

        bool result_city = (city.isnull() == rhs.city.isnull());
        if(!city.isnull() && !rhs.city.isnull())
        {
            result_city = (static_cast<std::string>(city).compare(static_cast<std::string>(rhs.city)) == 0);
        }
        if(print_diff_ && !result_city) std::cout << "city: " << city.print_quoted() << " != "<< rhs.city.print_quoted() << std::endl;

        bool result_stateorprovince = (stateorprovince.isnull() == rhs.stateorprovince.isnull());
        if(!stateorprovince.isnull() && !rhs.stateorprovince.isnull())
        {
            result_stateorprovince = (static_cast<std::string>(stateorprovince).compare(static_cast<std::string>(rhs.stateorprovince)) == 0);
        }
        if(print_diff_ && !result_stateorprovince) std::cout << "stateorprovince: " << stateorprovince.print_quoted() << " != "<< rhs.stateorprovince.print_quoted() << std::endl;

        bool result_postalcode = (postalcode.isnull() == rhs.postalcode.isnull());
        if(!postalcode.isnull() && !rhs.postalcode.isnull())
        {
            result_postalcode = (static_cast<std::string>(postalcode).compare(static_cast<std::string>(rhs.postalcode)) == 0);
        }
        if(print_diff_ && !result_postalcode) std::cout << "postalcode: " << postalcode.print_quoted() << " != "<< rhs.postalcode.print_quoted() << std::endl;

        bool result_country = (country.isnull() == rhs.country.isnull());
        if(!country.isnull() && !rhs.country.isnull())
        {
            result_country = (static_cast<std::string>(country).compare(static_cast<std::string>(rhs.country)) == 0);
        }
        if(print_diff_ && !result_country) std::cout << "country: " << country.print_quoted() << " != "<< rhs.country.print_quoted() << std::endl;

        bool result_telephone = (telephone.isnull() == rhs.telephone.isnull());
        if(!telephone.isnull() && !rhs.telephone.isnull())
        {
            result_telephone = (static_cast<std::string>(telephone).compare(static_cast<std::string>(rhs.telephone)) == 0);
        }
        if(print_diff_ && !result_telephone) std::cout << "telephone: " << telephone.print_quoted() << " != "<< rhs.telephone.print_quoted() << std::endl;

        bool result_fax = (fax.isnull() == rhs.fax.isnull());
        if(!fax.isnull() && !rhs.fax.isnull())
        {
            result_fax = (static_cast<std::string>(fax).compare(static_cast<std::string>(rhs.fax)) == 0);
        }
        if(print_diff_ && !result_fax) std::cout << "fax: " << fax.print_quoted() << " != "<< rhs.fax.print_quoted() << std::endl;

        bool result_email = (email.isnull() == rhs.email.isnull());
        if(!email.isnull() && !rhs.email.isnull())
        {
            result_email = (static_cast<std::string>(email).compare(static_cast<std::string>(rhs.email)) == 0);
        }
        if(print_diff_ && !result_email) std::cout << "email: " << email.print_quoted() << " != "<< rhs.email.print_quoted() << std::endl;

        bool result_notifyemail = (notifyemail.isnull() == rhs.notifyemail.isnull());
        if(!notifyemail.isnull() && !rhs.notifyemail.isnull())
        {
            result_notifyemail = (static_cast<std::string>(notifyemail).compare(static_cast<std::string>(rhs.notifyemail)) == 0);
        }
        if(print_diff_ && !result_notifyemail) std::cout << "notifyemail: " << notifyemail.print_quoted() << " != "<< rhs.notifyemail.print_quoted() << std::endl;

        bool result_vat = (vat.isnull() == rhs.vat.isnull());
        if(!vat.isnull() && !rhs.vat.isnull())
        {
            result_vat = (static_cast<std::string>(vat).compare(static_cast<std::string>(rhs.vat)) == 0);
        }
        if(print_diff_ && !result_vat) std::cout << "vat: " << vat.print_quoted() << " != "<< rhs.vat.print_quoted() << std::endl;

        bool result_ssntype = (ssntype.isnull() == rhs.ssntype.isnull());
        if(!ssntype.isnull() && !rhs.ssntype.isnull())
        {
            result_ssntype = (static_cast<std::string>(ssntype).compare(static_cast<std::string>(rhs.ssntype)) == 0);
        }
        if(print_diff_ && !result_ssntype) std::cout << "ssntype: " << ssntype.print_quoted() << " != "<< rhs.ssntype.print_quoted() << std::endl;

        bool result_ssn = (ssn.isnull() == rhs.ssn.isnull());
        if(!ssn.isnull() && !rhs.ssn.isnull())
        {
            result_ssn = (static_cast<std::string>(ssn).compare(static_cast<std::string>(rhs.ssn)) == 0);
        }
        if(print_diff_ && !result_ssn) std::cout << "ssn: " << ssn.print_quoted() << " != "<< rhs.ssn.print_quoted() << std::endl;

        bool result_disclosename = (disclosename.isnull() == rhs.disclosename.isnull());
        if(!disclosename.isnull() && !rhs.disclosename.isnull())
        {
            result_disclosename = (static_cast<bool>(disclosename) == static_cast<bool>(rhs.disclosename));
        }
        if(print_diff_ && !result_disclosename) std::cout << "disclosename: " << disclosename.print_quoted() << " != "<< rhs.disclosename.print_quoted() << std::endl;

        bool result_discloseorganization = (discloseorganization.isnull() == rhs.discloseorganization.isnull());
        if(!discloseorganization.isnull() && !rhs.discloseorganization.isnull())
        {
            result_discloseorganization = (static_cast<bool>(discloseorganization) == static_cast<bool>(rhs.discloseorganization));
        }
        if(print_diff_ && !result_discloseorganization) std::cout << "discloseorganization: " << discloseorganization.print_quoted() << " != "<< rhs.discloseorganization.print_quoted() << std::endl;

        bool result_discloseaddress = (discloseaddress.isnull() == rhs.discloseaddress.isnull());
        if(!discloseaddress.isnull() && !rhs.discloseaddress.isnull())
        {
            result_discloseaddress = (static_cast<bool>(discloseaddress) == static_cast<bool>(rhs.discloseaddress));
        }
        if(print_diff_ && !result_discloseaddress) std::cout << "discloseaddress: " << discloseaddress.print_quoted() << " != "<< rhs.discloseaddress.print_quoted() << std::endl;

        bool result_disclosetelephone = (disclosetelephone.isnull() == rhs.disclosetelephone.isnull());
        if(!disclosetelephone.isnull() && !rhs.disclosetelephone.isnull())
        {
            result_disclosetelephone = (static_cast<bool>(disclosetelephone) == static_cast<bool>(rhs.disclosetelephone));
        }
        if(print_diff_ && !result_disclosetelephone) std::cout << "disclosetelephone: " << disclosetelephone.print_quoted() << " != "<< rhs.disclosetelephone.print_quoted() << std::endl;

        bool result_disclosefax = (disclosefax.isnull() == rhs.disclosefax.isnull());
        if(!disclosefax.isnull() && !rhs.disclosefax.isnull())
        {
            result_disclosefax = (static_cast<bool>(disclosefax) == static_cast<bool>(rhs.disclosefax));
        }
        if(print_diff_ && !result_disclosefax) std::cout << "disclosefax: " << disclosefax.print_quoted() << " != "<< rhs.disclosefax.print_quoted() << std::endl;

        bool result_discloseemail = (discloseemail.isnull() == rhs.discloseemail.isnull());
        if(!discloseemail.isnull() && !rhs.discloseemail.isnull())
        {
            result_discloseemail = (static_cast<bool>(discloseemail) == static_cast<bool>(rhs.discloseemail));
        }
        if(print_diff_ && !result_discloseemail) std::cout << "discloseemail: " << discloseemail.print_quoted() << " != "<< rhs.discloseemail.print_quoted() << std::endl;

        bool result_disclosevat = (disclosevat.isnull() == rhs.disclosevat.isnull());
        if(!disclosevat.isnull() && !rhs.disclosevat.isnull())
        {
            result_disclosevat = (static_cast<bool>(disclosevat) == static_cast<bool>(rhs.disclosevat));
        }
        if(print_diff_ && !result_disclosevat) std::cout << "disclosevat: " << disclosevat.print_quoted() << " != "<< rhs.disclosevat.print_quoted() << std::endl;

        bool result_discloseident = (discloseident.isnull() == rhs.discloseident.isnull());
        if(!discloseident.isnull() && !rhs.discloseident.isnull())
        {
            result_discloseident = (static_cast<bool>(discloseident) == static_cast<bool>(rhs.discloseident));
        }
        if(print_diff_ && !result_discloseident) std::cout << "discloseident: " << discloseident.print_quoted() << " != "<< rhs.discloseident.print_quoted() << std::endl;

        bool result_disclosenotifyemail = (disclosenotifyemail.isnull() == rhs.disclosenotifyemail.isnull());
        if(!disclosenotifyemail.isnull() && !rhs.disclosenotifyemail.isnull())
        {
            result_disclosenotifyemail = (static_cast<bool>(disclosenotifyemail) == static_cast<bool>(rhs.disclosenotifyemail));
        }
        if(print_diff_ && !result_disclosenotifyemail) std::cout << "disclosenotifyemail: " << disclosenotifyemail.print_quoted() << " != "<< rhs.disclosenotifyemail.print_quoted() << std::endl;

        bool result_id = (id == rhs.id);
        if(print_diff_ && !result_id) std::cout << "id: " << id << " != "<< rhs.id << std::endl;


        return  result_roid
                && result_handle
                && result_sponsoring_registrar_handle
                && result_create_registrar_handle
                && result_creation_time
                && result_authinfopw
                && result_historyid
                && result_crhistoryid
                && result_update_registrar_handle
                && result_update_time
                && result_transfer_time
                && result_delete_time
                && result_name
                && result_organization
                && result_street1
                && result_street2
                && result_street3
                && result_city
                && result_stateorprovince
                && result_postalcode
                && result_country
                && result_telephone
                && result_fax
                && result_email
                && result_notifyemail
                && result_vat
                && result_ssntype
                && result_ssn
                && result_disclosename
                && result_discloseorganization
                && result_discloseaddress
                && result_disclosetelephone
                && result_disclosefax
                && result_discloseemail
                && result_disclosevat
                && result_discloseident
                && result_disclosenotifyemail
                && result_id
                ;
    }

    bool InfoContactData::operator!=(const InfoContactData& rhs) const
    {
        return !this->operator ==(rhs);
    }

    void InfoContactData::set_diff_print(bool print_diff)
    {
        print_diff_ = print_diff;
    }

    std::string InfoContactData::to_string() const
    {
        return Util::format_data_structure("InfoContactData",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("crhistoryid",boost::lexical_cast<std::string>(crhistoryid)))
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid)))
        (std::make_pair("delete_time",delete_time.print_quoted()))
        (std::make_pair("handle",handle))
        (std::make_pair("roid",roid))
        (std::make_pair("sponsoring_registrar_handle",sponsoring_registrar_handle))
        (std::make_pair("create_registrar_handle",create_registrar_handle))
        (std::make_pair("update_registrar_handle",update_registrar_handle.print_quoted()))
        (std::make_pair("creation_time",boost::lexical_cast<std::string>(creation_time)))
        (std::make_pair("update_time",update_time.print_quoted()))
        (std::make_pair("transfer_time",transfer_time.print_quoted()))
        (std::make_pair("authinfopw",authinfopw))
        (std::make_pair("name",name.print_quoted()))
        (std::make_pair("organization",organization.print_quoted()))
        (std::make_pair("street1",street1.print_quoted()))
        (std::make_pair("street2",street2.print_quoted()))
        (std::make_pair("street3",street3.print_quoted()))
        (std::make_pair("city",city.print_quoted()))
        (std::make_pair("stateorprovince",stateorprovince.print_quoted()))
        (std::make_pair("postalcode",postalcode.print_quoted()))
        (std::make_pair("country",country.print_quoted()))
        (std::make_pair("telephone",telephone.print_quoted()))
        (std::make_pair("fax",fax.print_quoted()))
        (std::make_pair("email",email.print_quoted()))
        (std::make_pair("notifyemail_",notifyemail.print_quoted()))
        (std::make_pair("vat",vat.print_quoted()))
        (std::make_pair("ssntype",ssntype.print_quoted()))
        (std::make_pair("ssn",ssn.print_quoted()))
        (std::make_pair("disclosename",disclosename.print_quoted()))
        (std::make_pair("discloseorganization",discloseorganization.print_quoted()))
        (std::make_pair("discloseaddress",discloseaddress.print_quoted()))
        (std::make_pair("disclosetelephone",disclosetelephone.print_quoted()))
        (std::make_pair("disclosefax",disclosefax.print_quoted()))
        (std::make_pair("discloseemail",discloseemail.print_quoted()))
        (std::make_pair("disclosevat",disclosevat.print_quoted()))
        (std::make_pair("discloseident",discloseident.print_quoted()))
        (std::make_pair("disclosenotifyemail",disclosenotifyemail.print_quoted()))
        );
    }


}//namespace Fred


