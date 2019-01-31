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

#include "src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/types.h>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

FACTORY_MODULE_INIT_DEFI(TestEmailExistsForManagedZones_init)

/**
 * @returns test result for this email only - either OK, FAIL or SKIPPED
 */
static std::string check_email(
        LibFred::OperationContext& ctx,
        std::string email,
        const std::set<std::string>& managed_zones)
{
    boost::trim(email);

    std::string at_domain;
    {
        std::string::size_type position_of_atsign = email.find_first_of("@");
        if (position_of_atsign == std::string::npos)        // no '@'
        {
            return LibFred::ContactTestStatus::FAIL;
        }

        if (email.size() == position_of_atsign + 1)     // no domain part
        {
            return LibFred::ContactTestStatus::FAIL;
        }
        // else: it is guaranteed that char at(position_of_atsign + 1) exists

        // ... BTW: Now we know email is not empty
        if (*(email.end() - 1) == '.')
        {
            email.erase((email.end() - 1));
        }

        at_domain = email.substr(position_of_atsign);
    }

    std::vector<std::string::iterator> label_separator_positions;
    if (*at_domain.begin() == '.')
    {
        // just to simplify reasoning later
        return LibFred::ContactTestStatus::FAIL;
    }
    // guarantees that label_separator_positions.size() > 0 which is handy in iterator init in for(;;) below
    label_separator_positions.push_back(at_domain.begin());

    for (std::string::iterator it = at_domain.begin();
         it != at_domain.end();
         ++it
         )
    {
        if (*it == '.')
        {
            label_separator_positions.push_back(it);
        }
    }

    // important: going from longest subdomain to shortest (TLD)

    /* tricky detail: starting from begin() + 1 because email should look like:
     * alice@subdomain.zone
     * even if we would be managing myzone we can't (directly) say if
     * e-mail bob@myzone exists, we can only say so about foo@mydomain.myzone
     */for (std::vector<std::string::iterator>::const_iterator it = label_separator_positions.begin() + 1;
         it != label_separator_positions.end();
         ++it)
    {
        // try to find zone
        if (managed_zones.find(
                    std::string(
                            (*it) + 1,
                            at_domain.end())                  // emulation of "hostname.substr(it)"
                    ) == managed_zones.end()
            )
        {
            continue;
        }

        // ok, we are managing the zone, lets ...
        // ... try to find first level sub-domain in this zone
        if (
            ctx.get_conn().exec_params(
                    // clang-format off
                    "WITH normalized AS ( "
                        "SELECT "
                            "LOWER( "
                                "$1::text "
                            ") AS email_domain "
                    ") "
                    "SELECT 1 "
                        "FROM object_registry AS o_r "
                            "JOIN domain AS d USING(id),"
                            "normalized "
                        "WHERE o_r.erdate IS NULL "
                            "AND LOWER(o_r.name) = normalized.email_domain ",
                    // clang-format on
                    Database::query_param_list
                            //  (it - 1)      is vector iterator pointing to position of previous delimiter
                            //                (position is represented by different string iterator)
                            // *(it - 1)      is string iterator pointing to previous delimiter
                            // *(it - 1) + 1  is string iterator pointing to first characted after previous iterator
                            // Simple, huh?
                            (std::string(*(it - 1) + 1, at_domain.end()))
                    ).size() != 1)
        {
            return LibFred::ContactTestStatus::FAIL;
        }
        else
        {
            return LibFred::ContactTestStatus::OK;
        }
    }

    // ok no subdomain of hostname is a managed zones
    return LibFred::ContactTestStatus::SKIPPED;
}


static std::set<std::string> get_managed_zones(LibFred::OperationContext& ctx)
{
    Database::Result res_zones = ctx.get_conn().exec("SELECT fqdn FROM zone ");
    std::set<std::string> result;
    for (Database::Result::Iterator it = res_zones.begin();
         it != res_zones.end();
         ++it)
    {
        result.insert(static_cast<std::string>((*it)["fqdn"]));
    }

    return result;
}


Test::TestRunResult TestEmailExistsForManagedZones::run(unsigned long long _history_id) const
{

    TestDataProvider<TestEmailExistsForManagedZones> data;
    data.init_data(_history_id);

    boost::trim(data.email_);

    std::vector<std::string> emails;
    boost::algorithm::split(
            emails,
            data.email_,
            boost::is_any_of(","));

    // empty email is ok
    if (emails.size() == 0)
    {
        return TestRunResult(LibFred::ContactTestStatus::OK);
    }

    LibFred::OperationContextCreator ctx;

    std::set<std::string> managed_zones = get_managed_zones(ctx);

    std::vector<std::string> invalid_emails;
    unsigned int skipped_email_count = 0;

    for (std::vector<std::string>::const_iterator it = emails.begin();
         it != emails.end();
         ++it
         )
    {
        std::string result = check_email(
                ctx,
                *it,
                managed_zones);

        if (result == LibFred::ContactTestStatus::SKIPPED)
        {
            skipped_email_count++;
        }
        else if (result == LibFred::ContactTestStatus::FAIL)
        {
            invalid_emails.push_back(*it);
        }
    }

    if (skipped_email_count == emails.size() && emails.size() > 0)
    {
        return TestRunResult(
                LibFred::ContactTestStatus::SKIPPED,
                std::string("this test is intended for e-mails from managed zones only"));
    }
    else if (!invalid_emails.empty())
    {
        return TestRunResult(
                LibFred::ContactTestStatus::FAIL,
                "emails: " + boost::join(
                        invalid_emails,
                        ",") + " are invalid");
    }

    return TestRunResult(LibFred::ContactTestStatus::OK);
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
