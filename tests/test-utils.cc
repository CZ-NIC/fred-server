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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/domain/create_domain.h"
#include "fredlib/opexception.h"
#include "util/util.h"

#include "fredlib/contact_verification/contact.h"
#include "fredlib/object_states.h"
#include "contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestUtils)

const std::string server_name = "test-utils";

///test callback
void print_str(const char* str)
{
    printf("\nstr: %s\n",str);
}

///test callback
void print_3str(const char* str1, const char* str2, const char* str3)
{
    printf("\nstr: %s - %s - %s\n", str1, str2, str3);
}

//check callback input data
class CheckCallback
{
    Fred::ConstArr valid_params_;
    bool param_ok_;

public:
    CheckCallback(const Fred::ConstArr& valid_params)
    : valid_params_(valid_params)
    , param_ok_(true)
    {}

    //checking callback called for all params
    void operator()(const char* param, const char* value)
    {
        bool param_ok = false;

        for(int i =0; i < valid_params_.size();++i)
        {
            if(std::string(param).compare(std::string(valid_params_[i])) ==0) param_ok = true;
        }

        //save
        param_ok_ = param_ok;
    }

    bool params_ok()
    {
        return param_ok_;
    }
};


//get callback - store input data
class GetCallback
{
std::vector<std::string> data_;

public:

    //callback
    void operator()(const char* param, const char* value)
    {
        //BOOST_MESSAGE(std::string("get_cbk: ")+ " : " + param + " : " + value);
        data_.push_back(std::string(param) + " : " + value);
        //BOOST_MESSAGE(data_.size());
    }

    std::vector<std::string> get()
    {
        return data_;
    }
};

/**
 * test FixedString
 * test FixedString construction and methods calls with precreated data
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(fixed_string)
{
    {
        Fred::FixedString<3> fs3;
        fs3.push_front("11");
        fs3.push_front("22");
        BOOST_CHECK(std::string(fs3.data) == "211");
    }

    {
        Fred::FixedString<3> fs3;
        fs3.push_back("11");
        fs3.push_back("22");
        BOOST_CHECK(std::string(fs3.data) == "112");
    }
}

/**
 * test exception callback
 * create and throw exception, catch exception get parameters and check number of parameters
 */
BOOST_AUTO_TEST_CASE(exception_params_callback)
{
    try
    {
        std::string fqdn_("fred.cz");
        std::string registrar_("REG-FRED_A");
        std::string errmsg("test exception ||");
        errmsg += std::string(" not found:fqdn: ")+boost::replace_all_copy(fqdn_,"|", "[pipe]")+" |";
        errmsg += std::string(" not found:registrar: ")+boost::replace_all_copy(registrar_,"|", "[pipe]")+" |";

        throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str());
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        GetCallback a;
        ex.callback_exception_params(boost::ref(a));
        BOOST_CHECK((a.get().size()) == 2);
    }

}

/**
 * test exception with quoted data
 * create and throw exception with quoted data, check params with callback
 */
BOOST_AUTO_TEST_CASE(test_quote_pipe_in_exception)
{
    try
    {
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test exception || not found:fqdn: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " | not found:registrant: test ";
        errmsg += " |";
        throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str());
    }
    catch(Fred::UpdateDomainException& ex)
    {
        CheckCallback check(ex.get_fail_param());
        ex.callback_exception_params(check);
        BOOST_CHECK(check.params_ok());
    }

    {//check Fred::OperationExceptionBase exception
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test exception || not found:fqdn: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " |";
        BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str())
        , Fred::OperationExceptionBase);
    }

    try
    {
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test exception || not found:fqdn: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " |";
        throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str());
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        CheckCallback check(ex.get_fail_param());
        ex.callback_exception_params(check);
        BOOST_CHECK(check.params_ok());
    }

    {//invalid exception data
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test error in exception params || not found:fqdn1: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " |";
        BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str())
        , Fred::UpdateDomainException::OperationErrorType);
        BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str())
        , Fred::OperationErrorBase);
    }

}

BOOST_AUTO_TEST_SUITE_END();//TestUtils
