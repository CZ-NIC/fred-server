#include "tests/interfaces/whois/fixture_common.h"
#include "tests/setup/fixtures_utils.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(gdsd)//get_domain_status_descriptions)

struct object_status_descriptions_fixture
: whois_impl_instance_fixture
{
    typedef std::map<std::string, std::string> map_type;

    std::string test_lang;
    std::string object_name;
    map_type statuses;
    unsigned int status_number;

    object_status_descriptions_fixture()
    : test_lang("EN")
    {
        statuses["expired"] = "description of expired";
        statuses["unguarded"] = "description of unguarded";
        statuses["serverTransferProhibited"] = "description of serverTransferProhibited";
        Fred::OperationContext ctx;
        BOOST_FOREACH(map_type::value_type& p, statuses)
        {
            ctx.get_conn().exec_params(
                "INSERT INTO enum_object_states_desc "
                "VALUES ((SELECT id FROM enum_object_states WHERE name = $1::text),"
                " $2::text,"
                " $3::text)",
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
      status_number = 2;
      object_name = ("domain");
    }

    std::vector<Registry::WhoisImpl::ObjectStatusDesc> get_description(const std::string& lang)
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
      status_number = 1;
      object_name = ("contact");
    }

    std::vector<Registry::WhoisImpl::ObjectStatusDesc> get_description(const std::string& lang)
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
      status_number = 1;
      object_name = ("nsset");
    }

    std::vector<Registry::WhoisImpl::ObjectStatusDesc> get_description(const std::string& lang)
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
      status_number = 1;
      object_name = ("keyset");
    }

    std::vector<Registry::WhoisImpl::ObjectStatusDesc> get_description(const std::string& lang)
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

BOOST_FIXTURE_TEST_CASE_TEMPLATE(gdsd/*get_domain_status_descriptions*/, T, test_types, T)
{
    Fred::OperationContext ctx;
    std::vector<Fred::ObjectStateDescription> states =
                        Fred::GetObjectStateDescriptions(T::test_lang)
                        .set_object_type(T::object_name)
                        .set_external()
                        .exec(ctx);
    std::vector<Registry::WhoisImpl::ObjectStatusDesc> vec_osd = T::get_description(T::test_lang);
    BOOST_CHECK(states.size() == vec_osd.size());
    std::sort(states.begin(), states.end(), private_sort<Fred::ObjectStateDescription>);
    std::sort(vec_osd.begin(), vec_osd.end(), private_sort<Registry::WhoisImpl::ObjectStatusDesc>);
    std::vector<Fred::ObjectStateDescription>::iterator it;
    std::vector<Registry::WhoisImpl::ObjectStatusDesc>::iterator it2;
    for(it = states.begin(), it2 = vec_osd.begin(); it != states.end(); ++it, ++it2)
    {
        BOOST_CHECK(it->handle == it2->handle);
        BOOST_CHECK(it->description == it2->name);
    }
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(gdsdm/*get_domain_status_descriptions_missing*/, T, test_types, T)
{
    try
    {
        std::vector<Registry::WhoisImpl::ObjectStatusDesc> vec_osd = T::get_description("");
        BOOST_ERROR(std::string("this ") + T::object_name + " must not have a localization");
    }
    catch(const Registry::WhoisImpl::MissingLocalization& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(gdsdol/*get_domain_status_descriptions_other_lang*/, T, test_types, T)
{
    std::vector<Registry::WhoisImpl::ObjectStatusDesc> vec_osd = T::get_description("XX");
    BOOST_CHECK(T::status_number == vec_osd.size());
    for(std::vector<Registry::WhoisImpl::ObjectStatusDesc>::iterator it = vec_osd.begin(); it != vec_osd.end(); ++it)
    {
        //if not present - at() throws
        BOOST_CHECK(T::statuses.at(it->handle) == it->name);
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_domain_status_descriptions
BOOST_AUTO_TEST_SUITE_END()//TestWhois
