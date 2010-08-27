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


#include "faked_args.h"
#include "handle_args.h"
#include "config_handler.h"
#include "handle_general_args.h"
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
                regMan.get())
            );
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
                regMan.get())
            );
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
/*
  void NotifyClient::sendLetters(std::auto_ptr<Register::File::Transferer> fileman, const std::string &conf_file)
  {
     TRACE("[CALL] Register::Notify::sendLetters()");
    // transaction is needed for 'ON COMMIT DROP' functionality
    
     HPCfgMap hpmail_config = readHPConfig(conf_file);

     Connection conn = Database::Manager::acquire();

     Result res = conn.exec("SELECT EXISTS "
             "(SELECT * FROM letter_archive WHERE status=6)");

     if ((bool)res[0][0]) {
            LOGGER(PACKAGE).notice("The files are already being processed. No action");
            return;
     }

     Database::Transaction trans(conn);

     // acquire lock which conflicts with itself but not basic locks used
     // by select and stuff..
     conn.exec("LOCK TABLE letter_archive IN SHARE UPDATE EXCLUSIVE MODE");
              
     res = conn.exec("SELECT file_id, attempt, id FROM letter_archive WHERE status=1 or status=4");
     
     if(res.size() == 0) {
         LOGGER(PACKAGE).debug("Register::Notify::sendLetters(): No files ready for processing"); 
         return;
     }
     ID file_id = res[0][0];
     ID letter_id = res[0][2];
     int new_status = 6;
     // we have to set application lock(status 6) somewhere while the table is locked in db
     conn.exec(boost::format("UPDATE letter_archive SET status=6 WHERE file_id=%1%") % file_id) ;
     
     // unlock the table - from now we need to hold the app lock until the end of processing
     trans.commit();

     for (unsigned int i=0;i<res.size(); i++) {

         Database::Transaction t(conn);
         // start processing the next record
         file_id = res[i][0];             
         int attempt = res[i][1];
         letter_id = res[i][2];

         // get the file from filemanager client and create the batch
         MailFile one;
         fileman->download(file_id, one);

         LOGGER(PACKAGE).debug(boost::format ("sendLetters File ID: %1% ") % res[i][0]);

         // data's ready, we can send it
         new_status=5;
         try {
             HPMail::init_session(hpmail_config);
             std::string filename( (boost::format("Letter_%1%.pdf") % letter_id).str());

             HPMail::get()->upload(one, filename);
         } catch(std::exception& ex) {
             std::cout << "Error: " << ex.what() << " on file ID " << file_id << std::endl;
             new_status = 4; // set error status in database
         } catch(...) {
             std::cout << "Unknown Error" << " on file ID " << file_id << std::endl;
             new_status = 4; // set error status in database
         }

         // set status for next item first
         if(i+1 < res.size()) {
             ID next_id = res[i+1][0];
            conn.exec(boost::format("UPDATE letter_archive SET status=6 where file_id=%1%")
            % next_id);
         }

         conn.exec(boost::format("UPDATE letter_archive SET status=%1%, moddate=now(), attempt=%2% "
                "where id=%3%") % new_status % (attempt+1) % letter_id);
        
         t.commit();
     }
  }
*/

  struct message_proc {
      ID id;
      unsigned attempt;
  };

  void NotifyClient::sendLetters(std::auto_ptr<Register::File::Transferer> fileman, const std::string &conf_file)
  {
      TRACE("[CALL] Register::Notify::sendLetters()");
    // transaction is needed for 'ON COMMIT DROP' functionality
    
     HPCfgMap hpmail_config = readHPConfig(conf_file);

     Connection conn = Database::Manager::acquire();

     /* now bail out if other process (presumably another instance of this
      * method) is already doing something with this table. No support 
      * for multithreaded access - we would have to remember which 
      * records exactly are we processing
      */ 
     Result res = conn.exec("SELECT EXISTS "
             "(SELECT * FROM letter_archive WHERE status=6)");

     if ((bool)res[0][0]) {
            LOGGER(PACKAGE).notice("The files are already being processed. No action");
            return;
     }

     Database::Transaction trans(conn);

     // acquire lock which conflicts with itself but not basic locks used
     // by select and stuff..
     conn.exec("LOCK TABLE letter_archive IN SHARE UPDATE EXCLUSIVE MODE");
     // application level lock and notification about data processing
     conn.exec("UPDATE letter_archive SET status = 6 WHERE status = 1 OR status = 4"); 

     // TODO try to enable this
     trans.commit();
     Database::Transaction trans2(conn);

     res = conn.exec("SELECT file_id, id, attempt FROM letter_archive WHERE status=6");
     
     if(res.size() == 0) {
         LOGGER(PACKAGE).debug("Register::Notify::sendLetters(): No files ready for processing"); 
         return;
     }
        
     int new_status = 5;
     std::string batch_id;
     // store IDs and attempt counts of records being processed
     std::vector<message_proc> processed;

     ID file_id = 0;
     ID letter_id = 0;
     processed.reserve(res.size());
     try {

         HPMail::init_session(hpmail_config);
         for(unsigned i=0;i<res.size();i++) {
             NamedMailFile smail;
             file_id = res[i][0];
             letter_id = res[i][1];

             message_proc mp;
             mp.id = letter_id;
             mp.attempt = res[i][2];
             processed.push_back(mp);

             fileman->download(file_id, smail.data);

             LOGGER(PACKAGE).debug(boost::format ("sendLetters File ID: %1% ") % file_id);

             smail.name = (boost::format("Letter_%1%.pdf") % letter_id).str();

             HPMail::get()->save_file_for_upload(smail);
         }
         batch_id = HPMail::get()->upload();
     } catch(std::exception& ex) {
         std::cout << "Error: " << ex.what() << " on file ID " << file_id << std::endl;
         new_status = 4; // set error status in database
     } catch(...) {
         std::cout << "Unknown Error on file ID " << file_id << std::endl;
         new_status = 4; // set error status in database
     }

     res = conn.exec("SELECT id, attempt FROM letter_archive WHERE status = 6");

     for (std::vector<message_proc>::iterator it = processed.begin(); it!=processed.end(); it++) {
           unsigned int new_attempt = (*it).attempt + 1;
           conn.exec(boost::format("UPDATE letter_archive SET status = %1%, "
                                   "batch_id = '%2%', attempt = %3%, "
                                   "moddate = CURRENT_TIMESTAMP WHERE id = %4%")
                                    % new_status % conn.escape(batch_id)
                                    % new_attempt % (*it).id);
     }

     trans2.commit();
  }

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
         HPMail::init_session(hpmail_config);
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
    ADDOPT(NOTIFY_SHOW_OPTS_NAME, TYPE_NOTYPE, true, true),
    ADDOPT(NOTIFY_DEBUG_NAME, TYPE_NOTYPE, false, false),
    ADDOPT(NOTIFY_EXCEPT_TYPES_NAME, TYPE_STRING, false, false),
    ADDOPT(NOTIFY_LIMIT_NAME, TYPE_UINT, false, false),
    ADDOPT(NOTIFY_USE_HISTORY_TABLES_NAME, TYPE_BOOL, false, false),
    ADDOPT(NOTIFY_FILE_SEND_NAME, TYPE_STRING, true, true),
    ADDOPT(NOTIFY_HPMAIL_CONFIG_NAME, TYPE_STRING, true, true)
};

#undef ADDOPT

int 
NotifyClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}

} // namespace Admin;

