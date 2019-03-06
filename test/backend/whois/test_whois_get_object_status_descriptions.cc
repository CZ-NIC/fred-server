/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/whois/fixture_common.hh"

#include "libfred/object_state/get_object_state_descriptions.hh"

#include <boost/foreach.hpp>
#include <boost/mpl/list.hpp>

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(gdsd)//get_domain_status_descriptions

struct object_status_descriptions_fixture
: whois_impl_instance_fixture
{
    typedef std::map<std::string, std::string> map_type;
    typedef Fred::Backend::Whois::ObjectStatusDesc StatusDesc;
    const std::string test_lang;
    std::string object_name;
    map_type statuses;
    unsigned int type_statuses_number;

    object_status_descriptions_fixture()
    : test_lang("EN")
    {
        LibFred::OperationContextCreator ctx;
        statuses["expired"] = "description of expired";
        statuses["unguarded"] = "description of unguarded";
        statuses["serverTransferProhibited"] = "description of serverTransferProhibited";
        BOOST_FOREACH(map_type::value_type& p, statuses)
        {
            ctx.get_conn().exec_params(
                "INSERT INTO enum_object_states_desc (state_id, lang, description) "
                "VALUES ((SELECT id "
                         "FROM enum_object_states "
                         "WHERE name = $1::text), "
                         "$2::text, "
                         "$3::text)",
                Database::query_param_list(p.first)("XX")(p.second)
            );
        }
        ctx.commit_transaction();
    }
};

struct domain_type
: object_status_descriptions_fixture
{
    domain_type()
    : object_status_descriptions_fixture()
    {
      type_statuses_number = 2;
      object_name = "domain";
    }

    std::vector<StatusDesc> get_description(const std::string& lang)
    {
        return impl.get_domain_status_descriptions(lang);
    }
};

struct contact_type
: object_status_descriptions_fixture
{
    contact_type()
    : object_status_descriptions_fixture()
    {
      type_statuses_number = 1;
      object_name = ("contact");
    }

    std::vector<StatusDesc> get_description(const std::string& lang)
    {
        return impl.get_contact_status_descriptions(lang);
    }
};

struct nsset_type
: object_status_descriptions_fixture
{
    nsset_type()
    : object_status_descriptions_fixture()
    {
      type_statuses_number = 1;
      object_name = ("nsset");
    }

    std::vector<StatusDesc> get_description(const std::string& lang)
    {
        return impl.get_nsset_status_descriptions(lang);
    }
};

struct keyset_type
: object_status_descriptions_fixture
{
    keyset_type()
    : object_status_descriptions_fixture()
    {
      type_statuses_number = 1;
      object_name = ("keyset");
    }

    std::vector<StatusDesc> get_description(const std::string& lang)
    {
        return impl.get_keyset_status_descriptions(lang);
    }
};

template <class T>
bool private_sort(T o1, T o2)
{
    return o1.handle < o2.handle;
}

typedef boost::mpl::list<domain_type, contact_type, nsset_type, keyset_type> test_types;

/*get_domain_status_descriptions*/
BOOST_FIXTURE_TEST_CASE_TEMPLATE(gdsd, T, test_types, T)
{
    LibFred::OperationContextCreator ctx;
    std::vector<::LibFred::ObjectStateDescription> states =
                        LibFred::GetObjectStateDescriptions(T::test_lang)
                            .set_object_type(T::object_name)
                            .set_external()
                            .exec(ctx);
    std::vector<typename T::StatusDesc> vec_osd = T::get_description(T::test_lang);
    BOOST_CHECK(states.size() == vec_osd.size());
    std::sort(states.begin(), states.end(), private_sort<::LibFred::ObjectStateDescription>);
    std::sort(vec_osd.begin(), vec_osd.end(), private_sort<typename T::StatusDesc>);
    for(unsigned int i = 0; i < states.size(); ++i)
    {
        BOOST_CHECK(states[i].handle == vec_osd[i].handle);
        BOOST_CHECK(states[i].description == vec_osd[i].name);
    }
}

/*get_domain_status_descriptions_missing*/
BOOST_FIXTURE_TEST_CASE_TEMPLATE(gdsdm, T, test_types, T)
{
    try
    {
        std::vector<typename T::StatusDesc> vec_osd = T::get_description("");
        BOOST_ERROR(std::string("this ") +
                    T::object_name +
                    " state must not have a localization");
    }
    catch(const Fred::Backend::Whois::MissingLocalization& ex)
    {
        BOOST_CHECK(true);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(ex));
    }
}

/*get_domain_status_descriptions_other_lang*/
BOOST_FIXTURE_TEST_CASE_TEMPLATE(gdsdol, T, test_types, T)
{
    std::vector<typename T::StatusDesc> vec_osd = T::get_description("XX");
    BOOST_CHECK(T::type_statuses_number == vec_osd.size());
    for(typename std::vector<typename T::StatusDesc>::iterator it = vec_osd.begin();
            it != vec_osd.end(); ++it)
    {
        //if not present - at() throws
        BOOST_CHECK(T::statuses.at(it->handle) == it->name);
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_domain_status_descriptions
BOOST_AUTO_TEST_SUITE_END()//TestWhois
