#include "tests/interfaces/whois/fixture_common.h"
#include "src/whois/registrar_group.h"

#include "src/fredlib/db_settings.h"
#include "util/random_data_generator.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_registrar_groups)

struct get_registrar_groups_fixture
: whois_impl_instance_fixture
{
    unsigned long long group_number;
    std::map<std::string, std::vector<std::string> > fixture_groups;

    get_registrar_groups_fixture()
    {
        Fred::OperationContextCreator ctx;
        fixture_groups = Whois::get_registrar_groups(ctx);
        group_number = fixture_groups.size();
        for(int i = 0; i < 5; ++i) //XXX
        {
            std::string name("test-group");
            RandomDataGenerator rdg;
            name += rdg.xnumstring(6);
            std::vector<std::string> registrars;
            Database::Result group = ctx.get_conn().exec_params(
                    "INSERT INTO registrar_group (short_name) "
                    "VALUES ($1::text) RETURNING ID ",
                    Database::query_param_list(name));
            for (int y = 0; y < rdg.xint() % 2 + 1; ++y)
            {
                const Fred::InfoRegistrarData& data = Test::registrar::make(ctx);
                ctx.get_conn().exec_params(
                        "INSERT INTO registrar_group_map (registrar_id, registrar_group_id, member_from) "
                        "VALUES ($1::bigint, $2::bigint, now()::date) ",
                        Database::query_param_list(data.id)(static_cast<unsigned long long>(group[0][0])));
                registrars.push_back(data.handle);
            }
            fixture_groups[name] = registrars;
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_registrar_groups, get_registrar_groups_fixture)
{
    std::vector<Registry::WhoisImpl::RegistrarGroup> group_vec = impl.get_registrar_groups();
    BOOST_CHECK(group_vec.size() == fixture_groups.size());
    std::map<std::string, std::vector<std::string> >::iterator f_gr_it;
    BOOST_FOREACH(const Registry::WhoisImpl::RegistrarGroup& it, group_vec)
    {
        f_gr_it = fixture_groups.find(it.name);
        BOOST_REQUIRE(f_gr_it != fixture_groups.end());
        const std::vector<std::string>& found = f_gr_it->second;
        BOOST_REQUIRE(found.size() == it.members.size());
        BOOST_FOREACH(const std::string& reg, it.members)
        {
            bool is_in_other = false;
            BOOST_FOREACH(const std::string& f_reg, found)
            {
                if (reg == f_reg)
                {
                    is_in_other = true;
                }
            }
            BOOST_CHECK(is_in_other);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_registrar_groups
BOOST_AUTO_TEST_SUITE_END()//TestWhois
