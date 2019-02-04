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
 *  @file fixtures.h
 *  <++>
 */

#ifndef FIXTURES_HH_68A62721C00840268108B7F7893798D1
#define FIXTURES_HH_68A62721C00840268108B7F7893798D1

#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "util/random_data_generator.hh"

#include <map>
#include <set>
#include <boost/test/unit_test.hpp>

namespace Test {
namespace Admin {
namespace Notification {

struct HasExpiredDomain : virtual instantiate_db_template {
    ::LibFred::InfoDomainData domain;
    HasExpiredDomain() {
        ::LibFred::OperationContextCreator ctx;
        domain = Test::exec(Test::CreateX_factory<::LibFred::CreateDomain>()
                        .make(Test::registrar::make(ctx).handle,
                              Test::contact::make(ctx).handle,
                              "expired-domain.cz")
                        .set_expiration_date(boost::gregorian::day_clock::local_day() - boost::gregorian::date_duration(1)),
                 ctx);
        ::LibFred::PerformObjectStateRequest(domain.id).exec(ctx);
        ctx.commit_transaction();
    }
};

struct HasInvalidEmail : HasExpiredDomain {
    std::map<unsigned long long, std::set<std::string> > domain_emails_map;
    HasInvalidEmail() {
        const std::string invalid_email = "invalid email";
        std::set<std::string> invalid_emails;
        invalid_emails.insert(invalid_email);
        domain_emails_map[domain.id] = invalid_emails;
    }
};

struct HasValidEmail : HasExpiredDomain {
    std::map<unsigned long long, std::set<std::string> > domain_emails_map;
    HasValidEmail() {
        const std::string valid_email = "valid.email@example.com";
        std::set<std::string> valid_emails;
        valid_emails.insert(valid_email);
        domain_emails_map[domain.id] = valid_emails;
    }
};

struct HasValidEmails : HasExpiredDomain {
    std::map<unsigned long long, std::set<std::string> > domain_emails_map;
    HasValidEmails() {
        const std::string valid_email = "valid.email@example.com";
        const std::string another_valid_email = "another.valid.email@example.com";
        std::set<std::string> valid_emails;
        valid_emails.insert(valid_email);
        valid_emails.insert(another_valid_email);
        domain_emails_map[domain.id] = valid_emails;
    }
};

struct HasSameEmails : HasExpiredDomain {
    std::map<unsigned long long, std::set<std::string> > domain_emails_map;
    HasSameEmails() {
        const std::string valid_email = "valid.email@example.com";
        const std::string same_valid_email = "valid.email@example.com";
        std::set<std::string> valid_emails;
        valid_emails.insert(valid_email);
        valid_emails.insert(same_valid_email);
        domain_emails_map[domain.id] = valid_emails;
    }
};

struct HasEmptyEmail : HasExpiredDomain {
    std::map<unsigned long long, std::set<std::string> > domain_emails_map;
    HasEmptyEmail() {
        const std::string valid_email = "valid.email@example.com";
        const std::string empty_email = "";
        std::set<std::string> empty_emails;
        empty_emails.insert(valid_email);
        empty_emails.insert(empty_email);
        domain_emails_map[domain.id] = empty_emails;
    }
};

struct HasNoEmails : HasValidEmails {
    std::map<unsigned long long, std::set<std::string> > none_domain_emails_map;
    HasNoEmails() {
        std::set<std::string> no_emails;
        none_domain_emails_map[domain.id] = no_emails;
    }
};

} // namespace Test::Admin::Notification
} // namespace Test::Admin
} // namespace Test

#endif
