
BOOST_AUTO_TEST_SUITE(get_registrars)

struct get_my_registrar_list_fixture
: test_registrar_fixture
{
    std::map<std::string,Fred::InfoRegistrarOutput> registrar_info;
    int system_registars, total_registrars;

    get_my_registrar_list_fixture()
    : test_registrar_fixture(),//TODO remove
      system_registars(5),
      total_registrars(10)
    {
        Fred::OperationContext ctx;
        for(int i=0; i < total_registrars; ++i)
        {
            std::ostringstream test_handles;
            test_handles << test_registrar_handle << i;
            Fred::CreateRegistrar& cr = Fred::CreateRegistrar(test_handles.str())
                .set_name(std::string("TEST-REGISTRAR NAME")+xmark+boost::lexical_cast<std::string>(i))
                .set_street1(std::string("STR1")+xmark)
                .set_city("Praha")
                .set_postalcode("11150")
                .set_country("CZ");
            if(i > system_registars)
            {
                cr.set_system(true);
            }
            cr.exec(ctx);
            registrar_info[test_handles.str()] =
                    Fred::InfoRegistrarByHandle(test_handles.str())
                    .exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nonsystem_registrars, get_my_registrar_list_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoRegistrarOutput> reg_list_out =
            Fred::InfoRegistrarAllExceptSystem()
            .exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    BOOST_CHECK(reg_list_out.size() == registrar_info.size() - system_registars);
    for(unsigned int i=0; i < reg_list_out.size(); ++i)
    {
        //refactor
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.handle == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.handle);
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.id == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.id);
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.street1.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.street1.get_value());
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.city.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.city.get_value());
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.postalcode.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.postalcode.get_value());
        BOOST_CHECK(reg_list_out.at(i).info_registrar_data.country.get_value() == map_at(registrar_info, reg_list_out.at(i).info_registrar_data.handle).info_registrar_data.country.get_value());
        BOOST_CHECK(!reg_list_out.at(i).info_registrar_data.system.get_value());//neither of them to be system
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_registrars
