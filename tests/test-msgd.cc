/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test-msgd.h"

BOOST_AUTO_TEST_SUITE(TestMsgd)

BOOST_AUTO_TEST_CASE( test_corba_interface )
{
    /*
    try
    {
    */
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

        const int msg_count = 10;

        int i = 0;
        for (; i < msg_count; ++i)
        {
            //sms test
            CORBA::String_var sms_contact = CORBA::string_dup("REG-FRED_A");
            CORBA::String_var phone = CORBA::string_dup("+420123456789");
            CORBA::String_var content = CORBA::string_dup("Ahoj!");
            CORBA::String_var sms_message_type = CORBA::string_dup("mojeid_pin2");

            CORBA::ULongLong message_archive_id = 0;

            message_archive_id = messages_ref->saveSmsToSend(
                    sms_contact, phone , content, sms_message_type, 1, 1);

            //letter test
            CORBA::String_var letter_contact = CORBA::string_dup("REG-FRED_B");

            Registry::Messages::PostalAddress_var paddr( new Registry::Messages::PostalAddress);

            paddr->name = CORBA::string_dup("name");
            paddr->org = CORBA::string_dup("");
            paddr->street1 = CORBA::string_dup("st1");
            paddr->street2 = CORBA::string_dup("");
            paddr->street3 = CORBA::string_dup("");
            paddr->city = CORBA::string_dup("city");
            paddr->state = CORBA::string_dup("");
            paddr->code = CORBA::string_dup("11150");
            paddr->city = CORBA::string_dup("Praha");
            paddr->country = CORBA::string_dup("Czech Republic");


            //load test pdf
            std::string test_pdf_file_name = std::string(TEST_DATA)+"doc/fred-server/test-file.pdf";
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
                message_archive_id = messages_ref->saveLetterToSend(
                    letter_contact, paddr.in()
                    , file_content, file_name,file_type
                    , registered_letter_message_type, 1, 1, "registered_letter");
            else
                message_archive_id = messages_ref->saveLetterToSend(
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

BOOST_AUTO_TEST_SUITE_END();
