/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include <boost/test/unit_test.hpp>
#include <string>

#include "libfred/opcontext.hh"
#include "libfred/opexception.hh"

#include "util/random_data_generator.hh"
#include "test/setup/fixtures.hh"
#include "test/libfred/util.hh"

DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier, std::string);
DECLARE_EXCEPTION_DATA(testing_int_data, int);
DECLARE_EXCEPTION_DATA(testing_unsigned_int_data, unsigned int);

DECLARE_VECTOR_OF_EXCEPTION_DATA(contact1_handle, std::string);
DECLARE_VECTOR_OF_EXCEPTION_DATA(contact2_handle, std::string);
DECLARE_VECTOR_OF_EXCEPTION_DATA(contact3_handle, int);

BOOST_FIXTURE_TEST_SUITE(TestOperationException, Test::instantiate_db_template)

const std::string server_name = "test-opexception";

///exception instance for tests
template <class DERIVED_EXCEPTION> struct ExceptionTemplate1
: ExceptionData_unknown_contact_handle<DERIVED_EXCEPTION>
, ExceptionData_unknown_registrar_handle<DERIVED_EXCEPTION>
{};

struct TestException
: virtual ::LibFred::OperationException
, ExceptionTemplate1<TestException>
//, ExceptionData_unknown_contact_handle<TestException>
//, ExceptionData_unknown_registrar_handle<TestException>
, ExceptionData_testing_int_data<TestException>
, ExceptionData_testing_unsigned_int_data<TestException>
, ExceptionData_vector_of_contact1_handle<TestException>
, ExceptionData_vector_of_contact2_handle<TestException>
, ExceptionData_vector_of_contact3_handle<TestException>
{};


BOOST_AUTO_TEST_CASE(throwTestExceptionCallback)
{
    try
    {
        //instance of the client's Exception
        TestException ex;

        //callback interface
        boost::function<void (const std::string& unknown_registrar_handle)> f;

        //exception setter assignment into callback parameter
        f = boost::bind(&TestException::set_unknown_registrar_handle,&ex,_1);

        //implementation callback call
        f("test_registrar");

        //client checking exception instance
        if(ex.throw_me())
        {
            BOOST_THROW_EXCEPTION(ex);
        }
    }
    catch(const TestException& ex)
    {
        BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));
        BOOST_CHECK(ex.is_set_unknown_registrar_handle());
        //BOOST_CHECK_EQUAL(ex.throw_me(), true);//this is not real use case
    }
}

BOOST_AUTO_TEST_CASE(throwTestException)
{
    BOOST_CHECK_EXCEPTION(
    try
    {
        try
        {
            try
            {
                BOOST_THROW_EXCEPTION(
                        TestException()
                        .set_unknown_registrar_handle("test_registrar")
                        .set_unknown_contact_handle("test_contact")
                        .set_testing_int_data(5)
                        .set_testing_unsigned_int_data(6)
                        .add_contact1_handle("test-vector-data")
                        .set_vector_of_contact2_handle(Util::vector_of<std::string>("test-vector-data1")("test-vector-data2")("test-vector-data3"))
                        .set_vector_of_contact3_handle(Util::vector_of<int>(5)(5)(5))
                        .add_contact3_handle(1)
                        .add_contact3_handle(2)
                        .add_contact3_handle(1)
                        .add_contact3_handle(3)
                        << ErrorInfo_unknown_registry_object_identifier("test_roid")//add anything

                        );
            }
            catch (::LibFred::ExceptionStack& exs)
            {
                exs.add_exception_stack_info("stack context info");
                throw;
            }
        }
        catch(boost::exception& ex)
        {
            BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));

            try
            {
                if (dynamic_cast<const ::LibFred::ExceptionStack&>(ex).is_set_exception_stack_info())
                {
                    BOOST_TEST_MESSAGE(dynamic_cast<const ::LibFred::ExceptionStack&>(ex).get_exception_stack_info());
                }
            }
            catch(const std::exception&)
            {
                BOOST_TEST_MESSAGE("\nhave no  ExceptionStack info");
            }

            BOOST_TEST_MESSAGE("\nwhen not interested in exception child type using error_info instance");
            const std::string* test_data_ptr = 0;
            test_data_ptr = boost::get_error_info<ErrorInfo_unknown_contact_handle>(ex);
            if(test_data_ptr)
            {
                BOOST_TEST_MESSAGE(*test_data_ptr);
                BOOST_CHECK(std::string("test_contact").compare(*test_data_ptr)==0);
            }
            test_data_ptr = boost::get_error_info<ErrorInfo_unknown_registrar_handle>(ex);
            if(test_data_ptr)
            {
                BOOST_TEST_MESSAGE(*test_data_ptr);
                BOOST_CHECK(std::string("test_registrar").compare(*test_data_ptr)==0);
            }

            BOOST_TEST_MESSAGE("\nwith known child exception type using wrapper");
            if(dynamic_cast<TestException&>(ex).is_set_unknown_contact_handle())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_contact_handle());
                BOOST_CHECK(std::string("test_contact").compare(dynamic_cast<TestException&>(ex).get_unknown_contact_handle())==0);
            }
            if(dynamic_cast<TestException&>(ex).is_set_unknown_registrar_handle())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle());
                BOOST_CHECK(std::string("test_registrar").compare(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle())==0);
            }
            if(dynamic_cast<TestException&>(ex).is_set_testing_int_data())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_testing_int_data());
                BOOST_CHECK(5 == dynamic_cast<TestException&>(ex).get_testing_int_data());
            }
            if(dynamic_cast<TestException&>(ex).is_set_testing_unsigned_int_data())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_testing_unsigned_int_data());
                BOOST_CHECK(6 == dynamic_cast<TestException&>(ex).get_testing_unsigned_int_data());
            }

            throw;//to check std::exception
        }
    }
    catch(std::exception& ex)
    {
        BOOST_TEST_MESSAGE("catch(std::exception&): ");
        BOOST_TEST_MESSAGE(ex.what());
        throw;
    }
    , std::exception
    , check_std_exception);
}

class TestClass
{
public:
    DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
    DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);
    DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier, std::string);
    DECLARE_EXCEPTION_DATA(testing_int_data, int);

    DECLARE_VECTOR_OF_EXCEPTION_DATA(contact1_handle, std::string);
    DECLARE_VECTOR_OF_EXCEPTION_DATA(contact2_handle, std::string);
    DECLARE_VECTOR_OF_EXCEPTION_DATA(contact3_handle, int);
    DECLARE_VECTOR_OF_EXCEPTION_DATA(contact4_handle, unsigned int);

    ///exception instance for tests
    struct TestException
    : virtual ::LibFred::OperationException
      , ExceptionData_unknown_contact_handle<TestException>
      , ExceptionData_unknown_registrar_handle<TestException>
      , ExceptionData_testing_int_data<TestException>
      , ExceptionData_vector_of_contact1_handle<TestException>
      , ExceptionData_vector_of_contact2_handle<TestException>
      , ExceptionData_vector_of_contact3_handle<TestException>
      , ExceptionData_vector_of_contact4_handle<TestException>
    {};

    void whoops1()
    {
        TestException test_exception;
        test_exception.set_unknown_registrar_handle("test_registrar")
        .set_unknown_contact_handle("test_contact")
        .set_testing_int_data(5)
        .add_contact1_handle("test-vector-data")
        .set_vector_of_contact2_handle(Util::vector_of<std::string>("test-vector-data1")("test-vector-data2")("test-vector-data3"))
        .set_vector_of_contact3_handle(Util::vector_of<int>(5)(5)(5))
        .add_contact3_handle(1)
        .add_contact3_handle(2)
        .add_contact3_handle(1)
        .add_contact3_handle(3)
        .set_vector_of_contact4_handle(Util::vector_of<unsigned int>(5)(5)(5))
        .add_contact4_handle(1)
        .add_contact4_handle(2)
        .add_contact4_handle(1)
        .add_contact4_handle(3)
        ;

        if(test_exception.throw_me())
            BOOST_THROW_EXCEPTION(test_exception);
    }

    void whoops2()
    {
        TestException test_exception;
        test_exception
        << ErrorInfo_unknown_registry_object_identifier("test_roid")//add anything
        ;
        test_exception.set_throw_me();//used operator<<

        if(test_exception.throw_me())
            BOOST_THROW_EXCEPTION(test_exception);
    }

};

BOOST_AUTO_TEST_CASE(throwNestedException)
{
    BOOST_CHECK_EXCEPTION(
    try
    {
        try
        {
            try
            {
                TestClass().whoops1();//test throw
            }
            catch (::LibFred::ExceptionStack& exs)
            {
                exs.add_exception_stack_info("stack context info");
                throw;
            }
        }
        catch(boost::exception& ex)
        {
            BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));

            try
            {
                if (dynamic_cast<const ::LibFred::ExceptionStack&>(ex).is_set_exception_stack_info())
                {
                    BOOST_TEST_MESSAGE(dynamic_cast<const ::LibFred::ExceptionStack&>(ex).get_exception_stack_info());
                }
            }
            catch(const std::exception&)
            {
                BOOST_TEST_MESSAGE("\nhave no  ExceptionStack info");
            }

            BOOST_TEST_MESSAGE("\nwhen not interested in exception child type using error_info instance");
            const std::string* test_data_ptr = 0;
            test_data_ptr = boost::get_error_info<ErrorInfo_unknown_contact_handle>(ex);
            if(test_data_ptr)
            {
                BOOST_TEST_MESSAGE(*test_data_ptr);
                BOOST_CHECK(std::string("test_contact").compare(*test_data_ptr)==0);
            }
            test_data_ptr = boost::get_error_info<ErrorInfo_unknown_registrar_handle>(ex);
            if(test_data_ptr)
            {
                BOOST_TEST_MESSAGE(*test_data_ptr);
                BOOST_CHECK(std::string("test_registrar").compare(*test_data_ptr)==0);
            }

            BOOST_TEST_MESSAGE("\nwith known child exception type using wrapper");
            if(dynamic_cast<TestException&>(ex).is_set_unknown_contact_handle())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_contact_handle());
                BOOST_CHECK(std::string("test_contact").compare(dynamic_cast<TestException&>(ex).get_unknown_contact_handle())==0);
            }
            if(dynamic_cast<TestException&>(ex).is_set_unknown_registrar_handle())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle());
                BOOST_CHECK(std::string("test_registrar").compare(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle())==0);
            }
            if(dynamic_cast<TestException&>(ex).is_set_testing_int_data())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_testing_int_data());
                BOOST_CHECK(5 == dynamic_cast<TestException&>(ex).get_testing_int_data());
            }
            throw;//to check std::exception
        }
    }
    catch(std::exception& ex)
    {
        BOOST_TEST_MESSAGE("catch(std::exception&): ");
        BOOST_TEST_MESSAGE(ex.what());
        throw;
    }
    , std::exception
    , check_std_exception);

    BOOST_CHECK_EXCEPTION(
    try
    {
        try
        {
            try
            {
                TestClass().whoops2();//test throw
            }
            catch (::LibFred::ExceptionStack& exs)
            {
                exs.add_exception_stack_info("stack context info");
                throw;
            }
        }
        catch(boost::exception& ex)
        {
            BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));

            try
            {
                if (dynamic_cast<const ::LibFred::ExceptionStack&>(ex).is_set_exception_stack_info())
                {
                    BOOST_TEST_MESSAGE(dynamic_cast<const ::LibFred::ExceptionStack&>(ex).get_exception_stack_info());
                }
            }
            catch(const std::exception&)
            {
                BOOST_TEST_MESSAGE("\nhave no  ExceptionStack info");
            }

            BOOST_TEST_MESSAGE("\nwhen not interested in exception child type using error_info instance");
            const std::string* test_data_ptr = 0;
            test_data_ptr = boost::get_error_info<ErrorInfo_unknown_contact_handle>(ex);
            if(test_data_ptr)
            {
                BOOST_TEST_MESSAGE(*test_data_ptr);
                BOOST_CHECK(std::string("test_contact").compare(*test_data_ptr)==0);
            }
            test_data_ptr = boost::get_error_info<ErrorInfo_unknown_registrar_handle>(ex);
            if(test_data_ptr)
            {
                BOOST_TEST_MESSAGE(*test_data_ptr);
                BOOST_CHECK(std::string("test_registrar").compare(*test_data_ptr)==0);
            }

            BOOST_TEST_MESSAGE("\nwith known child exception type using wrapper");
            if(dynamic_cast<TestException&>(ex).is_set_unknown_contact_handle())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_contact_handle());
                BOOST_CHECK(std::string("test_contact").compare(dynamic_cast<TestException&>(ex).get_unknown_contact_handle())==0);
            }
            if(dynamic_cast<TestException&>(ex).is_set_unknown_registrar_handle())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle());
                BOOST_CHECK(std::string("test_registrar").compare(dynamic_cast<TestException&>(ex).get_unknown_registrar_handle())==0);
            }
            if(dynamic_cast<TestException&>(ex).is_set_testing_int_data())
            {
                BOOST_TEST_MESSAGE(dynamic_cast<TestException&>(ex).get_testing_int_data());
                BOOST_CHECK(5 == dynamic_cast<TestException&>(ex).get_testing_int_data());
            }
            throw;//to check std::exception
        }
    }
    catch(std::exception& ex)
    {
        BOOST_TEST_MESSAGE("catch(std::exception&): ");
        BOOST_TEST_MESSAGE(ex.what());
        throw;
    }
    , std::exception
    , check_std_exception);
}


BOOST_AUTO_TEST_CASE(throwInternalError)
{
    BOOST_CHECK_EXCEPTION(
    try
    {
        try
        {
            try
            {
                BOOST_THROW_EXCEPTION(::LibFred::InternalError("oops"));
            }
            catch (::LibFred::ExceptionStack& exs)
            {
                exs.add_exception_stack_info("stack context info");
                throw;
            }
        }
        catch(::LibFred::InternalError& ex)
        {
            BOOST_TEST_MESSAGE( boost::diagnostic_information(ex));

            BOOST_TEST_MESSAGE( ex.what());

            try
            {
                if (dynamic_cast<const ::LibFred::ExceptionStack&>(ex).is_set_exception_stack_info())
                {
                    BOOST_TEST_MESSAGE(dynamic_cast<const ::LibFred::ExceptionStack&>(ex).get_exception_stack_info());
                }
            }
            catch(const std::exception&)
            {
                BOOST_TEST_MESSAGE("\nhave no  ExceptionStack info");
            }

            throw;//to check std::exception
        }
    }
    catch(std::exception& ex)
    {
        BOOST_TEST_MESSAGE("catch(std::exception&): ");
        BOOST_TEST_MESSAGE(ex.what());
        throw;
    }
    , std::exception
    , check_std_exception);
}

BOOST_AUTO_TEST_CASE(substring)
{
    BOOST_CHECK(std::string("aaabbbccc").find("abc") == std::string::npos);
    BOOST_CHECK(std::string("aaababcbbccc").find("abc") != std::string::npos);
}

struct ExceptionBase
{
    int base_test;

    ExceptionBase(ExceptionBase const& i) noexcept
    : base_test(i.base_test)
    {}

    ExceptionBase() noexcept
    : base_test(0)
    {}

};

struct TestExceptionFlag
  : virtual std::exception
  , virtual boost::exception
  , //virtual
  ExceptionBase
{
    int test;

    TestExceptionFlag(TestExceptionFlag const& i) noexcept
    : ExceptionBase(i)
    , test(i.test)
    {}

    TestExceptionFlag() noexcept
    : test(0)
    {}

};

BOOST_AUTO_TEST_CASE(flag_copy)
{
    TestExceptionFlag ex1;
    ex1.test = 10;
    ex1.base_test = 20;

    BOOST_CHECK(ex1.test == 10);
    BOOST_CHECK(ex1.base_test == 20);

    TestExceptionFlag ex2 (ex1);
    BOOST_CHECK(ex2.test == 10);
    BOOST_CHECK(ex2.base_test == 20);

    try
    {
        BOOST_THROW_EXCEPTION(ex1);
    }
    catch(const TestExceptionFlag& ex )
    {
        BOOST_CHECK(ex.test == 10);
        BOOST_CHECK(ex.base_test == 20);
    }
}


BOOST_AUTO_TEST_SUITE_END();//TestOperationException

