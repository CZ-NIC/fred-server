/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include <utility>

#include "test/deprecated/test_msgd.hh"

BOOST_AUTO_TEST_SUITE(TestMsgd)


Registry::Messages::PostalAddress_var make_test_postal_address()
{
    Registry::Messages::PostalAddress_var paddr( new Registry::Messages::PostalAddress);

    paddr->name = CORBA::string_dup("name");
    paddr->org = CORBA::string_dup("");
    paddr->street1 = CORBA::string_dup("st1");
    paddr->street2 = CORBA::string_dup("");
    paddr->street3 = CORBA::string_dup("");
    paddr->state = CORBA::string_dup("");
    paddr->city = CORBA::string_dup("Praha");
    paddr->code = CORBA::string_dup("11150");
    paddr->country = CORBA::string_dup("Czech Republic");

    return paddr;
}

Registry::Messages_var make_messages_object_reference()
{
    //CORBA init
    FakedArgs orb_fa = CfgArgs::instance()->fa;
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
        , ns_args_ptr->nameservice_host
        , ns_args_ptr->nameservice_port
        , ns_args_ptr->nameservice_context);

    BOOST_TEST_MESSAGE( "Registry::Messages::_narrow" );
    Registry::Messages_var messages_ref;
    messages_ref = Registry::Messages::_narrow(
            CorbaContainer::get_instance()->nsresolve("Messages"));

    return messages_ref;
}

BOOST_AUTO_TEST_CASE( test_corba_interface )
{
    /*
    try
    {
    */
        Registry::Messages_var messages_ref = make_messages_object_reference();


        const int msg_count = 10;

        int i = 0;
        for (; i < msg_count; ++i)
        {
            //sms test
            CORBA::String_var sms_contact = CORBA::string_dup("REG-FRED_A");
            CORBA::String_var phone = CORBA::string_dup("+420123456789");
            CORBA::String_var content = CORBA::string_dup("Ahoj!");
            CORBA::String_var sms_message_type = CORBA::string_dup("mojeid_pin2");

            messages_ref->saveSmsToSend(
                    sms_contact, phone , content, sms_message_type, 1, 1);

            //letter test
            CORBA::String_var letter_contact = CORBA::string_dup("REG-FRED_B");

            Registry::Messages::PostalAddress_var paddr = make_test_postal_address();

            //load test pdf
            std::string test_pdf_file_name = std::string(TEST_DATA)+"/doc/fred-server/test_file.pdf";
            std::ifstream test_pdf;
            long long test_pdf_length = 0;
            test_pdf.open (test_pdf_file_name.c_str()
                    , std::ios::in | std::ios::binary);
            if(test_pdf.is_open())
            {//ok file is there
                test_pdf.seekg (0, std::ios::end);//seek end of the file
                test_pdf_length = test_pdf.tellg();//get length of the file
                test_pdf.seekg (0, std::ios::beg);//reset
            }
            else //no more files
                    throw std::runtime_error(
                            std::string("unable to open file: ")
                            + test_pdf_file_name);

            Registry::Messages::ByteBuffer_var file_content( new Registry::Messages::ByteBuffer(test_pdf_length) );//prealocate
            file_content->length(test_pdf_length);//set/alocate
            CORBA::Octet* data = file_content->get_buffer();

            /*
            data[0] = 'p';
            data[1] = 'd';
            data[2] = 'f';
            */

            data[0] = 'p';

            //read pdf into buffer
            test_pdf.read( reinterpret_cast<char*>(&(data[0])), static_cast<std::streamsize>(test_pdf_length) );

            CORBA::String_var file_name = CORBA::string_dup("test1.pdf");

            CORBA::String_var file_type = CORBA::string_dup("expiration warning letter");
            CORBA::String_var registered_letter_message_type = CORBA::string_dup("mojeid_pin2");
            CORBA::String_var letter_message_type = CORBA::string_dup("mojeid_pin3");

            if(i%2)
            {
                //save should throw with wrong postal adress
                paddr->name = CORBA::string_dup("");
                BOOST_CHECK_THROW(
                messages_ref->saveLetterToSend(
                    letter_contact, paddr.in()
                    , file_content, file_name,file_type
                    , registered_letter_message_type, 1, 1, "registered_letter")
                    , Registry::Messages::ErrorReport)
                ;

                paddr->name = CORBA::string_dup("name");
                messages_ref->saveLetterToSend(
                    letter_contact, paddr.in()
                    , file_content, file_name,file_type
                    , registered_letter_message_type, 1, 1, "registered_letter");
            }
            else
                messages_ref->saveLetterToSend(
                    letter_contact, paddr.in()
                    , file_content, file_name,file_type
                    , letter_message_type, 1, 1, "letter");

        }
        BOOST_CHECK_EQUAL(i, msg_count);
        /*

    }
    catch(Registry::Messages::ErrorReport& er)
    {
        std::cerr << "Caught exception ErrorReport: "
             << er.reason.in() << std::endl;

    }

    catch(CORBA::TRANSIENT&)
    {
        std::cerr << "Caught system exception TRANSIENT -- unable to contact the "
             << "server." << std::endl;
    }
    catch(CORBA::SystemException& ex)
    {
        std::cerr << "Caught a CORBA::" << ex._name() << std::endl;
    }
    catch(CORBA::Exception& ex)
    {
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
    }
    catch(omniORB::fatalException& fe)
    {
        std::cerr << "Caught omniORB::fatalException:" << std::endl;
        std::cerr << "  file: " << fe.file() << std::endl;
        std::cerr << "  line: " << fe.line() << std::endl;
        std::cerr << "  mesg: " << fe.errmsg() << std::endl;

    }
    catch(std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;

    }
    catch(...)
    {
        std::cout << "Unknown Error" << std::endl;

    }
         */

}

void test_save_letter_to_send_bad_address_impl(Registry::Messages::PostalAddress_var paddr)
{
    Registry::Messages_var messages_ref = make_messages_object_reference();

    //letter test
    CORBA::String_var letter_contact = CORBA::string_dup("REG-FRED_B");

    //load test pdf
    std::string test_pdf_file_name = std::string(TEST_DATA)+"/doc/fred-server/test_file.pdf";
    std::ifstream test_pdf;
    long long test_pdf_length = 0;
    test_pdf.open (test_pdf_file_name.c_str()
            , std::ios::in | std::ios::binary);
    if(test_pdf.is_open())
    {//ok file is there
        test_pdf.seekg (0, std::ios::end);//seek end of the file
        test_pdf_length = test_pdf.tellg();//get length of the file
        test_pdf.seekg (0, std::ios::beg);//reset
    }
    else //no file
            throw std::runtime_error(
                    std::string("unable to open file: ")
                    + test_pdf_file_name);

    Registry::Messages::ByteBuffer_var file_content( new Registry::Messages::ByteBuffer(test_pdf_length) );//prealocate
    file_content->length(test_pdf_length);//set/alocate
    CORBA::Octet* data = file_content->get_buffer();

    //read pdf into buffer
    test_pdf.read( reinterpret_cast<char*>(&(data[0])), static_cast<std::streamsize>(test_pdf_length) );

    CORBA::String_var file_name = CORBA::string_dup("test1.pdf");

    CORBA::String_var file_type = CORBA::string_dup("expiration warning letter");
    CORBA::String_var registered_letter_message_type = CORBA::string_dup("mojeid_pin2");
    CORBA::String_var letter_message_type = CORBA::string_dup("mojeid_pin3");

    messages_ref->saveLetterToSend(
        letter_contact, paddr.in()
        , file_content, file_name,file_type
        , letter_message_type, 1, 1, "letter");
}

BOOST_AUTO_TEST_CASE( test_save_letter_to_send_bad_address_name )
{
    Registry::Messages::PostalAddress_var paddr = make_test_postal_address();
    paddr->name = CORBA::string_dup(" ");
    try
    {
        test_save_letter_to_send_bad_address_impl(paddr);
        BOOST_ERROR("missing wrong postal address exception");
    }
    catch(Registry::Messages::ErrorReport& er)
    {
        BOOST_CHECK(std::string("wrong postal address: empty address.name "
            "name:   org:  street1: st1 street2:  street3:  state:  city: Praha code: 11150 country: Czech Republic"
            ) == er.reason.in());
    }
}

BOOST_AUTO_TEST_CASE( test_save_letter_to_send_bad_address_street1 )
{
    Registry::Messages::PostalAddress_var paddr = make_test_postal_address();
    paddr->street1 = CORBA::string_dup(" ");
    try
    {
        test_save_letter_to_send_bad_address_impl(paddr);
        BOOST_ERROR("missing wrong postal address exception");
    }
    catch(Registry::Messages::ErrorReport& er)
    {
        BOOST_CHECK(std::string("wrong postal address: empty address.street1 "
            "name: name org:  street1:   street2:  street3:  state:  city: Praha code: 11150 country: Czech Republic"
            ) == er.reason.in());
    }
}

BOOST_AUTO_TEST_CASE( test_save_letter_to_send_bad_address_city )
{
    Registry::Messages::PostalAddress_var paddr = make_test_postal_address();
    paddr->city = CORBA::string_dup(" ");
    try
    {
        test_save_letter_to_send_bad_address_impl(paddr);
        BOOST_ERROR("missing wrong postal address exception");
    }
    catch(Registry::Messages::ErrorReport& er)
    {
        BOOST_CHECK(std::string("wrong postal address: empty address.city "
            "name: name org:  street1: st1 street2:  street3:  state:  city:   code: 11150 country: Czech Republic"
            ) == er.reason.in());
    }
}

BOOST_AUTO_TEST_CASE( test_save_letter_to_send_bad_address_code )
{
    Registry::Messages::PostalAddress_var paddr = make_test_postal_address();
    paddr->code = CORBA::string_dup(" ");
    try
    {
        test_save_letter_to_send_bad_address_impl(paddr);
        BOOST_ERROR("missing wrong postal address exception");
    }
    catch(Registry::Messages::ErrorReport& er)
    {
        BOOST_CHECK(std::string("wrong postal address: empty address.code "
            "name: name org:  street1: st1 street2:  street3:  state:  city: Praha code:   country: Czech Republic"
            ) == er.reason.in());
    }
}

BOOST_AUTO_TEST_CASE( test_save_letter_to_send_bad_address_country )
{
    Registry::Messages::PostalAddress_var paddr = make_test_postal_address();
    paddr->country = CORBA::string_dup(" ");
    try
    {
        test_save_letter_to_send_bad_address_impl(paddr);
        BOOST_ERROR("missing wrong postal address exception");
    }
    catch(Registry::Messages::ErrorReport& er)
    {
        BOOST_CHECK(std::string("wrong postal address: empty address.country "
            "name: name org:  street1: st1 street2:  street3:  state:  city: Praha code: 11150 country:  "
            ) == er.reason.in());
    }
}

BOOST_AUTO_TEST_SUITE_END();
