/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simple.h"
#include "commonclient.h"
#include "notifyclient.h"
#include "register/info_buffer.h"
#include "register/messages/messages_impl.h"


#include "cfg/faked_args.h"
#include "cfg/handle_args.h"
#include "cfg/config_handler.h"
#include "cfg/handle_general_args.h"
#include "hp/handle_hpmail_args.h"

#include "register/db_settings.h"

using namespace Database;

namespace Admin {

const struct options *
NotifyClient::getOpts()
{
    return m_opts;
}


void
NotifyClient::runMethod()
{
    if (m_conf.hasOpt(NOTIFY_STATE_CHANGES_NAME)) {
        state_changes();
    } else if (m_conf.hasOpt(NOTIFY_LETTERS_CREATE_NAME)) {
        letters_create();
    } else if (m_conf.hasOpt(NOTIFY_LETTERS_SEND_NAME)) {
        letters_send();
    } else if (m_conf.hasOpt(NOTIFY_SMS_SEND_NAME)) {
        sms_send();
    } else if (m_conf.hasOpt(NOTIFY_FILE_SEND_NAME)) {
        file_send();
    } else if (m_conf.hasOpt(NOTIFY_SHOW_OPTS_NAME)) {
        show_opts();
    }
}

void
NotifyClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("Notify", getOpts(), getOptsCount());
}

void
NotifyClient::state_changes()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create());

    Register::Messages::ManagerPtr msgMan
        = Register::Messages::create_manager();


    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::auto_ptr<Register::Notify::Manager> notifyMan(
            Register::Notify::Manager::create(
                &m_db,
                &mailMan,
                conMan.get(),
                nssMan.get(),
                keyMan.get(),
                domMan.get(),
                docMan.get(),
                regMan.get(),
                msgMan));

    std::string exceptTypes("");
    if (m_conf.hasOpt(NOTIFY_EXCEPT_TYPES_NAME)) {
        exceptTypes = m_conf.get<std::string>(NOTIFY_EXCEPT_TYPES_NAME);
    }
    int limit = 0;
    if (m_conf.hasOpt(NOTIFY_LIMIT_NAME)) {
        limit = m_conf.get<unsigned int>(NOTIFY_LIMIT_NAME);
    }
    notifyMan->notifyStateChanges(
            exceptTypes, limit,
            m_conf.hasOpt(NOTIFY_DEBUG_NAME) ? &std::cout : NULL,
            m_conf.hasOpt(NOTIFY_USE_HISTORY_TABLES_NAME));
}

void
NotifyClient::letters_create()
{
    callHelp(m_conf, no_help);
    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create());

    Register::Messages::ManagerPtr msgMan
        = Register::Messages::create_manager();


    std::auto_ptr<Register::Domain::Manager> domMan(
            Register::Domain::Manager::create(&m_db, zoneMan.get()));
    std::auto_ptr<Register::Contact::Manager> conMan(
            Register::Contact::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::NSSet::Manager> nssMan(
            Register::NSSet::Manager::create(
                &m_db,
                zoneMan.get(),
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::KeySet::Manager> keyMan(
            Register::KeySet::Manager::create(
                &m_db,
                m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME))
            );
    std::auto_ptr<Register::Registrar::Manager> regMan(
            Register::Registrar::Manager::create(&m_db));
    std::auto_ptr<Register::Notify::Manager> notifyMan(
            Register::Notify::Manager::create(
                &m_db,
                &mailMan,
                conMan.get(),
                nssMan.get(),
                keyMan.get(),
                domMan.get(),
                docMan.get(),
                regMan.get(),
                msgMan));

    notifyMan->generateLetters(m_conf.get<unsigned>(REG_DOCGEN_DOMAIN_COUNT_LIMIT));
}

void NotifyClient::letters_send()
{
    // TODO code duplicity except for the last line
    // you can:
    //          - move the body of the sendLetters() here
    //          - separate constructor for Notify::Manager
    //          - make sendLetters static
    callHelp(m_conf, no_help);

    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    std::auto_ptr<Register::File::Transferer> fileclient(new FileManagerClient(cc.getNS()));

    sendLetters(
           fileclient,
           m_conf.hasOpt(NOTIFY_HPMAIL_CONFIG_NAME) ? 
                m_conf.get<std::string> (NOTIFY_HPMAIL_CONFIG_NAME) :
                HPMAIL_CONFIG        
            );
}

void NotifyClient::sms_send()
{

    callHelp(m_conf, no_help);

    sendSMS(m_conf.hasOpt(NOTIFY_SMS_COMMAND_NAME)
            ? m_conf.get<std::string> (NOTIFY_SMS_COMMAND_NAME)
            : ( m_conf.hasOpt(SMS_COMMAND_NAME)
                ? m_conf.get<std::string> (SMS_COMMAND_NAME)
                : std::string("exit 1 "))
                  );
}


void NotifyClient::file_send()
{
    // TODO code duplicity except for the last line
    // you can:
    //          - move the body of the sendLetters() here
    //          - separate constructor for Notify::Manager
    //          - make sendLetters static
    callHelp(m_conf, no_help);

    std::auto_ptr<Register::Document::Manager> docMan(
            Register::Document::Manager::create(
                m_conf.get<std::string>(REG_DOCGEN_PATH_NAME),
                m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME),
                m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME),
                m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, m_conf.get<std::string>(NS_CONTEXT_NAME));
    
    std::auto_ptr<Register::File::Transferer> fileclient(new FileManagerClient(cc.getNS()));

    sendFile(
            m_conf.get<std::string> (NOTIFY_FILE_SEND_NAME),
            m_conf.hasOpt(NOTIFY_HPMAIL_CONFIG_NAME) ? 
                m_conf.get<std::string> (NOTIFY_HPMAIL_CONFIG_NAME) :
                HPMAIL_CONFIG        
            );
}


      /** This method sends letters from table letter_archive
       * it sets current processed row to status=6 (under processing)
       * and cancels execution at the beginning if any row in this table
       * is already being processed.
       *
       */


  void NotifyClient::sendLetters(std::auto_ptr<Register::File::Transferer> fileman, const std::string &conf_file)
  {
     Logging::Context ctx("send letters");
     TRACE("[CALL] Register::Notify::sendLetters()");

     Register::Messages::ManagerPtr messages_manager
         = Register::Messages::create_manager();

     HPCfgMap hpmail_config = readHPConfig(conf_file);
     Register::Messages::LetterProcInfo proc_letters = messages_manager->load_letters_to_send(0);

     if(proc_letters.empty()) return;

     int new_status = 5;
     std::string batch_id;


     try
     {
         LOGGER(PACKAGE).info("init postservice upload");
         HPMail::set(hpmail_config);

         for(unsigned i=0;i<proc_letters.size();i++)
         {
             Register::Messages::letter_proc mp = proc_letters.at(i);

             NamedMailFile smail;
             smail.name = mp.fname;
             fileman->download(mp.file_id, smail.data);

             LOGGER(PACKAGE).debug(boost::format(
                         "adding file (id=%1%) to batch") % mp.file_id);
             HPMail::get()->save_file_for_upload(smail);
         }
         batch_id = HPMail::get()->upload();
     }
     catch (std::exception& ex) {
         std::string msg = str(boost::format("error occured (%1%)") % ex.what());
         LOGGER(PACKAGE).error(msg);
         std::cerr << msg << std::endl;
         new_status = 4; // set error status in database
     }
     catch (...) {
         std::string msg = "unknown error occured"; 
         LOGGER(PACKAGE).error(msg);
         std::cerr << msg << std::endl;
         new_status = 4; // set error status in database
     }

     //set status
     messages_manager->set_letter_status(proc_letters,new_status,batch_id);

  }//sendLetters

  void NotifyClient::sendSMS(const std::string& command )
  {
      Logging::Context ctx("send sms");
      TRACE("[CALL] Register::Notify::sendSMS()");

      Register::Messages::ManagerPtr messages_manager
          = Register::Messages::create_manager();

      Register::Messages::SmsProcInfo proc_sms = messages_manager->load_sms_to_send(0);
      if(proc_sms.empty()) return;
      LOGGER(PACKAGE).info("sms sending");

      for(unsigned i=0;i<proc_sms.size();i++)
      {
          int new_status = 5;//ok status
          try
          {
              Register::Messages::sms_proc mp = proc_sms.at(i);
              std::string command_with_params
                  = command + " " + mp.phone_number + " " + mp.content;
              if(!system(command_with_params.c_str()))
              {
                  LOGGER(PACKAGE).error(
                          std::string("NotifyClient::sendSMS error command: ")
                              + command_with_params + "failed.");
                  new_status = 4; // set error status
              }//if failed
              else
              {
                  LOGGER(PACKAGE).info(
                          std::string("NotifyClient::sendSMS command: ")
                              + command_with_params + " OK");
              }//if ok
          }//try
          catch (std::exception& ex) {
              std::string msg = str(boost::format("error occured (%1%)") % ex.what());
              LOGGER(PACKAGE).error(msg);
              std::cerr << msg << std::endl;
              new_status = 4; // set error status in database
          }
          catch (...) {
              std::string msg = "unknown error occured";
              LOGGER(PACKAGE).error(msg);
              std::cerr << msg << std::endl;
              new_status = 4; // set error status in database
          }

          proc_sms.at(i).new_status = new_status;//seting new status

      }//for i


      //set status
      messages_manager->set_sms_status(proc_sms);
  }//sendSMS

  void NotifyClient::sendFile(const std::string &filename, const std::string &conf_file)  {

      TRACE("[CALL] Register::Notify::sendFile()");


      HPCfgMap hpmail_config = readHPConfig(conf_file);

      LOGGER(PACKAGE).debug(boost::format("File to send %1% ") % filename);

      std::ifstream infile(filename.c_str());

      if(infile.fail()) {
          std::cerr << "Failed to open file" << filename << std::endl;
          return;
      }

      infile.seekg(0, std::ios::end);
      std::streampos infile_size = infile.tellg();
      infile.seekg(0, std::ios::beg);
       
      MailFile file(infile_size);
      infile.read(&file[0], infile_size);

      try {
         HPMail::set(hpmail_config);
         HPMail::get()->upload(file, filename);
      } catch(std::exception& ex) {
         std::cerr << "Error: " << ex.what() << std::endl;
         throw;
      } catch(...) {
         std::cerr << "Unknown Error" << std::endl;
         throw;
      }

  }

    HPCfgMap 
    NotifyClient::readHPConfig(const std::string &conf_file)
    {
        FakedArgs fa;

        boost::shared_ptr<HandleHPMailArgs> hhp(new HandleHPMailArgs);

        HandlerPtrVector handlers =
        boost::assign::list_of
        (HandleArgsPtr(new HandleGeneralArgs(conf_file) ))
        (HandleArgsPtr(hhp));

        try
        {
            // CfgArgs always needs some argv vector, argc cannot be 0
            int argc = 1;
            char *argv[argc];
            argv[0] = new char [1];
            // zero-length string into argv[0]
            argv[0][0] = '\0';

            fa = CfgArgs::instance<HandleHelpArg>(handlers)->handle(argc, argv);
            HPCfgMap set_cfg = hhp->get_map();
            
            return set_cfg;
        }
        catch(const ReturnFromMain&)
        {
            return HPCfgMap();
            // return 0;
        }
    }

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_NOTIFY, name, name##_DESC, type, callable, visible}

const struct options
NotifyClient::m_opts[] = {
    ADDOPT(NOTIFY_STATE_CHANGES_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_LETTERS_CREATE_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_LETTERS_SEND_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_SMS_SEND_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_DEBUG_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(NOTIFY_EXCEPT_TYPES_NAME, TYPE_STRING, false, false),
    ADDOPT(NOTIFY_LIMIT_NAME, TYPE_UINT, false, false),
    ADDOPT(NOTIFY_USE_HISTORY_TABLES_NAME, TYPE_BOOL, false, false),
    ADDOPT(NOTIFY_FILE_SEND_NAME, TYPE_STRING, true, true),
    ADDOPT(NOTIFY_HPMAIL_CONFIG_NAME, TYPE_STRING, true, true),
    ADDOPT(NOTIFY_SMS_COMMAND_NAME, TYPE_STRING, true, true)
};

#undef ADDOPT

int 
NotifyClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

