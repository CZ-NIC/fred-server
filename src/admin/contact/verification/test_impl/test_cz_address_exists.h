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
 *  contact verification test for Czech postal address (defined in official set)
 */

#ifndef CONTACT_VERIFICATION_TEST_CZ_ADDRESS_53151278990_
#define CONTACT_VERIFICATION_TEST_CZ_ADDRESS_53151278990_

#include "src/admin/contact/verification/test_impl/test_interface.h"

#include <string>
#include <vector>
#include <utility>

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

namespace Admin
{
    class ContactVerificationTestCzAddress: public ContactVerificationTest {
        private:
            enum address_part {postal_code, city, street, house_number};
        public:
            typedef std::vector<std::pair<std::string, bool> > T_words_shortened;
            typedef std::pair<T_words_shortened, std::vector<std::string> > T_street_data;

        private:
            xmlDocPtr doc_;
            xmlXPathContextPtr xpathCtx_;

            const std::string street_delimiters_;
            const std::string street_shortened_word_signs_;
            const std::string city_delimiters_;
            const std::string city_shortened_word_signs_;

            bool is_address_valid(
                const ContactVerificationTestCzAddress::T_street_data& street,
                const ContactVerificationTestCzAddress::T_words_shortened& city,
                const string& postal_code) const;

            std::string diagnose_problem(
                const ContactVerificationTestCzAddress::T_street_data& street,
                const ContactVerificationTestCzAddress::T_words_shortened& city,
                const string& postal_code) const;

            std::vector<std::string> generate_xpath_queries(
                ContactVerificationTestCzAddress::T_street_data street,
                ContactVerificationTestCzAddress::T_words_shortened city,
                string postal_code,
                Optional<address_part> to_ommit = Optional<address_part>()) const;


        public:
            ContactVerificationTestCzAddress(const std::string& _mvcr_address_xml_filename);
            ~ContactVerificationTestCzAddress();

            virtual ContactVerificationTest::T_run_result run(long _history_id) const;
            virtual std::string get_name() const { return "cz_address_existence"; }
    };
}


#endif // #include guard end
