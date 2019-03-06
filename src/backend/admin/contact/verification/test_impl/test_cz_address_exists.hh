/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  contact verification test for Czech postal address (defined in official set)
 */

#ifndef TEST_CZ_ADDRESS_EXISTS_HH_49FD31BE47BB4477AFFADC2E1DD2F4B8
#define TEST_CZ_ADDRESS_EXISTS_HH_49FD31BE47BB4477AFFADC2E1DD2F4B8

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/tree.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/list_of.hpp>

#include <string>
#include <vector>
#include <utility>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

    FACTORY_MODULE_INIT_DECL(TestCzAddress_init)

    class TestCzAddress
    : public
        Test,
        test_auto_registration<TestCzAddress>
    {
        private:
            enum address_part {postal_code, city, street, house_number};
        public:
            typedef std::vector<std::pair<std::string, bool> > T_words_shortened;
            typedef std::pair<T_words_shortened, std::vector<std::string> > T_street_data;

        private:
            xmlDocPtr doc_;
            xmlXPathContextPtr xpathCtx_;

            static const std::string street_delimiters_;
            static const std::string street_shortened_word_signs_;
            static const std::string city_delimiters_;
            static const std::string city_shortened_word_signs_;

            bool is_address_valid(
                const TestCzAddress::T_street_data& street,
                const TestCzAddress::T_words_shortened& city,
                const std::string& postal_code) const;

            std::string diagnose_problem(
                const TestCzAddress::T_street_data& street,
                const TestCzAddress::T_words_shortened& city,
                const std::string& postal_code) const;

            std::vector<std::string> generate_xpath_queries(
                TestCzAddress::T_street_data street,
                TestCzAddress::T_words_shortened city,
                std::string postal_code,
                Optional<address_part> to_ommit = Optional<address_part>()) const;


        public:
            TestCzAddress& set_mvcr_address_xml_filename(const std::string& _mvcr_address_xml_filename);
            ~TestCzAddress();

            virtual TestRunResult run(unsigned long long _history_id) const;
            static std::string registration_name() { return "cz_address_existence"; }
    };

    template<> struct TestDataProvider<TestCzAddress>
    : TestDataProvider_common,
      _inheritTestRegName<TestCzAddress>
    {
        std::string street1_;
        std::string street2_;
        std::string street3_;
        std::string city_;
        std::string postalcode_;
        std::string country_;

        virtual void store_data(const LibFred::InfoContactOutput& _data) {
            street1_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street1);
            street2_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street2.get_value_or_default());
            street3_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street3.get_value_or_default());
            city_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().city);
            postalcode_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().postalcode);
            country_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().country);
        }

        virtual std::vector<std::string> get_string_data() const {
            return boost::assign::list_of
                (street1_)
                (city_)
                (postalcode_)
                (country_);
        }
    };
} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
