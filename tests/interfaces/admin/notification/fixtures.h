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

#ifndef TESTS_INTERFACES_ADMIN_NOTIFICATION_FIXTURES_H
#define TESTS_INTERFACES_ADMIN_NOTIFICATION_FIXTURES_H

#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/opcontext.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "util/random_data_generator.h"

#include <map>
#include <set>
#include <boost/test/unit_test.hpp>

struct HasExpiredDomain : virtual Test::Fixture::instantiate_db_template {
    Fred::InfoDomainData domain;
    HasExpiredDomain() {
        Fred::OperationContextCreator ctx;
        domain = Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                        .make(Test::registrar::make(ctx).handle,
                              Test::contact::make(ctx).handle,
                              "expired-domain.cz")
                        .set_expiration_date(boost::gregorian::day_clock::local_day() - boost::gregorian::date_duration(1)),
                 ctx);
        Fred::PerformObjectStateRequest(domain.id).exec(ctx);
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

#endif
