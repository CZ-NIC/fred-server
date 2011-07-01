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

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "corba/file_manager_client.h"

#include "commonclient.h"
#include "notifyclient.h"
#include "fredlib/info_buffer.h"

#include "cfg/faked_args.h"
#include "cfg/handle_args.h"
#include "cfg/config_handler_decl.h"
#include "cfg/handle_general_args.h"
#include "hp/handle_hpmail_args.h"

#include "fredlib/db_settings.h"

using namespace Database;

namespace Admin {



void
NotifyClient::runMethod()
{
    if (notify_state_changes){//m_conf.hasOpt(NOTIFY_STATE_CHANGES_NAME)
        state_changes();
    } else if (notify_letters_postservis_send//m_conf.hasOpt(NOTIFY_LETTERS_SEND_NAME)
            ) {
        letters_send();
    } else if (notify_sms_send//m_conf.hasOpt(NOTIFY_SMS_SEND_NAME)
            ) {
        sms_send();
    }
}

void
NotifyClient::state_changes()
{
    std::auto_ptr<Fred::Document::Manager> docMan(
            Fred::Document::Manager::create(
                docgen_path.get_value()//m_conf.get<std::string>(REG_DOCGEN_PATH_NAME)
                ,docgen_template_path.get_value()//m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME)
                ,fileclient_path.get_value()//m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME)
                ,m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context//m_conf.get<std::string>(NS_CONTEXT_NAME)
            );
    MailerManager mailMan(cc.getNS());
    std::auto_ptr<Fred::Zone::Manager> zoneMan(
            Fred::Zone::Manager::create());

    Fred::Messages::ManagerPtr msgMan
        = Fred::Messages::create_manager();


    std::auto_ptr<Fred::Domain::Manager> domMan(
            Fred::Domain::Manager::create(m_db, zoneMan.get()));
    std::auto_ptr<Fred::Contact::Manager> conMan(
            Fred::Contact::Manager::create(
                m_db,
                restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                )
            );
    std::auto_ptr<Fred::NSSet::Manager> nssMan(
            Fred::NSSet::Manager::create(
                m_db,
                zoneMan.get(),
                restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                )
            );
    std::auto_ptr<Fred::KeySet::Manager> keyMan(
            Fred::KeySet::Manager::create(
                m_db,
                restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                )
            );
    std::auto_ptr<Fred::Registrar::Manager> regMan(
            Fred::Registrar::Manager::create(m_db));
    std::auto_ptr<Fred::Notify::Manager> notifyMan(
            Fred::Notify::Manager::create(
                m_db,
                &mailMan,
                conMan.get(),
                nssMan.get(),
                keyMan.get(),
                domMan.get(),
                docMan.get(),
                regMan.get(),
                msgMan));

    std::string exceptTypes("");
    if (notify_state_changes_params.notify_except_types.is_value_set()//m_conf.hasOpt(NOTIFY_EXCEPT_TYPES_NAME)
            ) {
        exceptTypes = notify_state_changes_params.notify_except_types.get_value();//m_conf.get<std::string>(NOTIFY_EXCEPT_TYPES_NAME);
    }
    int limit = 0;
    if (notify_state_changes_params.notify_limit.is_value_set()//m_conf.hasOpt(NOTIFY_LIMIT_NAME)
            ) {
        limit = notify_state_changes_params.notify_limit.get_value();//m_conf.get<unsigned int>(NOTIFY_LIMIT_NAME);
    }
    notifyMan->notifyStateChanges(
            exceptTypes, limit,
            //m_conf.hasOpt(NOTIFY_DEBUG_NAME)
            notify_state_changes_params.notify_debug ? &std::cout : NULL,
                    notify_state_changes_params.notify_use_history_tables//m_conf.hasOpt(NOTIFY_USE_HISTORY_TABLES_NAME)
                    );
}

void NotifyClient::letters_send()
{
    // TODO code duplicity except for the last line
    // you can:
    //          - move the body of the sendLetters() here
    //          - separate constructor for Notify::Manager
    //          - make sendLetters static
    //callHelp(m_conf, no_help);

    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context//m_conf.get<std::string>(NS_CONTEXT_NAME)
            );
    std::auto_ptr<Fred::File::Transferer> fileclient(new FileManagerClient(cc.getNS()));

    sendLetters(
           fileclient,
           hpmail_config.is_value_set()//m_conf.hasOpt(NOTIFY_HPMAIL_CONFIG_NAME)
           ? hpmail_config.get_value()//m_conf.get<std::string> (NOTIFY_HPMAIL_CONFIG_NAME)
            : HPMAIL_CONFIG
            );
}

void NotifyClient::sms_send()
{

    //callHelp(m_conf, no_help);
    sendSMS(cmdline_sms_command.is_value_set()//m_conf.hasOpt(NOTIFY_SMS_COMMAND_NAME)
            ? cmdline_sms_command.get_value()//m_conf.get<std::string> (NOTIFY_SMS_COMMAND_NAME)
            : ( configfile_sms_command.is_value_set()//m_conf.hasOpt(SMS_COMMAND_NAME)
                ? configfile_sms_command.get_value()//m_conf.get<std::string> (SMS_COMMAND_NAME)
                : std::string("exit 1 "))
            , std::string("'")
           );
}

    void NotifyClient::send_letters_impl(
            Fred::File::Transferer* fileman
            , const HPCfgMap& hpmail_config
            , Fred::Messages::LetterProcInfo& proc_letters
            , std::string& new_status
            , std::string& batch_id)
    {
        if(proc_letters.empty())
        {
            LOGGER(PACKAGE).debug("NotifyClient::send_letters_impl: proc_letters is empty");
            return;
        }
        else
        {
            LOGGER(PACKAGE).debug(std::string(
                "NotifyClient::send_letters_impl: proc_letters size: ")
            + boost::lexical_cast<std::string>(proc_letters.size()));
        }

        try
        {
            LOGGER(PACKAGE).info(
                "NotifyClient::send_letters_impl: init postservice upload");
            HPMail::set(hpmail_config);

            for(unsigned i=0;i<proc_letters.size();i++)
            {
                Fred::Messages::letter_proc mp = proc_letters.at(i);

                NamedMailFile smail;
                smail.name = mp.fname;
                fileman->download(mp.file_id, smail.data);

                LOGGER(PACKAGE).debug(boost::format(
                    "NotifyClient::send_letters_impl: adding file (id=%1%) to batch")
                    % mp.file_id);
                HPMail::get()->save_file_for_upload(smail);
            }
            batch_id = HPMail::get()->upload();
        }
        catch (std::exception& ex) {
            std::string msg = str(boost::format(
                "NotifyClient::send_letters_impl: error occured (%1%)")
                % ex.what());
            LOGGER(PACKAGE).error(msg);
            std::cerr << msg << std::endl;
            new_status = "send_failed"; // set error status in database
        }
        catch (...) {
            std::string msg = "NotifyClient::send_letters_impl: unknown error occured";
            LOGGER(PACKAGE).error(msg);
            std::cerr << msg << std::endl;
            new_status = "send_failed"; // set error status in database
        }


    }//NotifyClient::send_letters_impl


  /** This method sends letters from table letter_archive
   * it sets current processed row to status=6 (under processing)
   * and cancels execution at the beginning if any row in this table
   * is already being processed.
   *
   */

  void NotifyClient::sendLetters(
          std::auto_ptr<Fred::File::Transferer> fileman
          , const std::string &conf_file)
  {
     Logging::Context ctx("send letters");
     TRACE("[CALL] Fred::Notify::sendLetters()");

     Fred::Messages::ManagerPtr messages_manager
         = Fred::Messages::create_manager();

     HPCfgMap hpmail_config = readHPConfig(conf_file);

     //letters
     std::string new_status = "sent";
     std::string batch_id;
     std::string comm_type = "letter";

     const std::size_t max_attempts_limit = 3;

     Fred::Messages::LetterProcInfo proc_letters
         = messages_manager->load_letters_to_send(0, comm_type, max_attempts_limit);
     send_letters_impl(fileman.get()
             ,hpmail_config,proc_letters,new_status,batch_id);
     messages_manager->set_letter_status(
         proc_letters,new_status,batch_id, comm_type, max_attempts_limit);

     //registered letters
     if(hpmail_config["hp_login_registered_letter_batch_id"].empty())
     {
         LOGGER(PACKAGE).info(
                 "NotifyClient::sendLetters: not sending registered letters");
         return;
     }

     new_status = "sent";
     batch_id=std::string("");
     comm_type = "registered_letter";

     LOGGER(PACKAGE).debug(std::string(
             "NotifyClient::sendLetters: hp_login_registered_letter_batch_id ")
         +hpmail_config["hp_login_registered_letter_batch_id"]);

     hpmail_config["hp_login_batch_id"]
         = hpmail_config["hp_login_registered_letter_batch_id"];

     Fred::Messages::LetterProcInfo proc_reg_letters
         = messages_manager->load_letters_to_send(0, comm_type, max_attempts_limit);
     send_letters_impl(fileman.get()
             ,hpmail_config,proc_reg_letters,new_status,batch_id);
     messages_manager->set_letter_status(
             proc_reg_letters,new_status,batch_id, comm_type, max_attempts_limit);
  }//sendLetters

  void NotifyClient::sendSMS(const std::string& command , const std::string& param_quote_by )
  {
      Logging::Context ctx("send sms");
      TRACE("[CALL] Fred::Notify::sendSMS()");

      Fred::Messages::ManagerPtr messages_manager
          = Fred::Messages::create_manager();

      const std::size_t max_attempts_limit = 3;

      Fred::Messages::SmsProcInfo proc_sms = messages_manager->load_sms_to_send(0, 3);
      if(proc_sms.empty()) return;
      LOGGER(PACKAGE).info("sms sending");

      for(unsigned i=0;i<proc_sms.size();i++)
      {
          std::string new_status = "sent";//ok status
          try
          {
              Fred::Messages::sms_proc mp = proc_sms.at(i);
              std::string command_with_params
                  = command + " " + param_quote_by + mp.phone_number + param_quote_by
                  + " " + param_quote_by + mp.content + param_quote_by;
              if(system(command_with_params.c_str()))
              {
                  LOGGER(PACKAGE).error(
                          std::string("NotifyClient::sendSMS error command: ")
                              + command_with_params + "failed.");
                  new_status = "send_failed"; // set error status
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
              new_status = "send_failed"; // set error status in database
          }
          catch (...) {
              std::string msg = "unknown error occured";
              LOGGER(PACKAGE).error(msg);
              std::cerr << msg << std::endl;
              new_status = "send_failed"; // set error status in database
          }

          proc_sms.at(i).new_status = new_status;//seting new status

      }//for i


      //set status
      messages_manager->set_sms_status(proc_sms, max_attempts_limit);
  }//sendSMS

  void NotifyClient::sendFile(const std::string &filename, const std::string &conf_file)  {

      TRACE("[CALL] Fred::Notify::sendFile()");


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

    HPCfgMap NotifyClient::readHPConfig(const std::string &conf_file)
    {
            boost::shared_ptr<HandleHPMailArgs> hhp(new HandleHPMailArgs);

            HandlerPtrVector hpv =
                boost::assign::list_of
                    (HandleArgsPtr(new HandleConfigFileArgs(conf_file) ))
                    (HandleArgsPtr(hhp));

            // it always needs some argv vector, argc cannot be 0
            FakedArgs fa;
            fa.add_argv(std::string(""));

            //handle
            for(HandlerPtrVector::const_iterator i = hpv.begin()
                    ; i != hpv.end(); ++i )
            {
                FakedArgs fa_out;
                (*i)->handle( fa.get_argc(), fa.get_argv(), fa_out);
                fa=fa_out;//last output to next input
            }//for HandlerPtrVector

            HPCfgMap set_cfg = hhp->get_map();

            return set_cfg;
    }

//#4712#comment:23
void notify_registered_letters_manual_send_impl(const std::string& nameservice_host_port
        , const std::string& nameservice_context
        , const RegisteredLettersManualSendArgs& params
        )
{

    Fred::Messages::ManagerPtr messages_manager
        = Fred::Messages::create_manager();

    //needed by fallback
    Fred::Messages::LetterProcInfo proc_reg_letters;
    Fred::Messages::LetterProcInfo fm_failed_reg_letters;//file manager failed letters
    std::string batch_id = std::string("manual send ")
        + boost::posix_time::to_iso_extended_string(
                boost::posix_time::microsec_clock::universal_time())+" UTC";
    std::string comm_type = "registered_letter";
    const std::size_t max_attempts_limit = 3;

    try
    {
        //get working directory from cfg
        optional_string working_directory = params.working_directory;

        //get shell cmd timeout from cfg
        unsigned long timeout = params.shell_cmd_timeout;

        if(working_directory.is_value_set())
        if (chdir(working_directory.get_value().c_str()) == -1)
        {
          std::string err_msg(strerror(errno));
          std::string msg("chdir error : ");
          Logging::Manager::instance_ref()
              .get(PACKAGE).error(msg+err_msg);
          throw std::runtime_error(msg+err_msg);
        }

        //checks

        //if rm is there
        {
          SubProcessOutput sub_output = ShellCmd("rm --version", timeout).execute();
          if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
        }
        //if gs is there
        {
          SubProcessOutput sub_output = ShellCmd("gs --version", timeout).execute();
          if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
        }
        //if base64 is there
        {
          SubProcessOutput sub_output = ShellCmd("base64 --version", timeout).execute();
          if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
        }
        //if sendmail is there
        {
          SubProcessOutput sub_output = ShellCmd("ls /usr/sbin/sendmail", timeout).execute();
          if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
        }

        // init file manager
        CorbaClient corba_client(0, 0
              , nameservice_host_port
              , nameservice_context);//NS_CONTEXT_NAME
        FileManagerClient fm_client(corba_client.getNS());
        Fred::File::ManagerPtr file_manager(
              Fred::File::Manager::create(&fm_client));

        //read letters
        std::string new_status = "sent";

         proc_reg_letters = messages_manager
                 ->load_letters_to_send(0, comm_type, max_attempts_limit);

          if (proc_reg_letters.size() == 0)
          {
              Logging::Manager::instance_ref().get(PACKAGE).debug("no registered letters found");

              //get email from cfg
              std::string email = params.email;

              if(email.empty()) throw std::runtime_error("email required");

              {
                  std::string cmd=
                  std::string("{\n"
                  "echo \"Subject: No new registered letters $(date +'%Y-%m-%d')\n"
                  "From: ")+email+"\nContent-Type: text/plain; charset=UTF-8; format=flowed"
                  "\nContent-Transfer-Encoding: 8bit\n\nno new registered letters\n\";"
                  "\n} | /usr/sbin/sendmail "+email;

                  SubProcessOutput sub_output = ShellCmd(cmd, timeout).execute();
                  if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
              }

              std::cout << "no new registered letters found" << std::endl;
              return;
          }

          //remove old letter files
          {
            SubProcessOutput sub_output = ShellCmd("rm -f letter*.pdf", timeout).execute();
            if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
          }
          {
            SubProcessOutput sub_output = ShellCmd("rm -f all.pdf", timeout).execute();
            if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
          }

          std::string addr_list;

          //process letter ids
          for(std::size_t i = 0; i < proc_reg_letters.size(); ++i)
          {
            unsigned long long file_id = proc_reg_letters[i].file_id;

            std::string letter_file_name = std::string("./letter")
                    + boost::lexical_cast<std::string>(file_id)
                    + ".pdf";

            std::vector<char> file_buffer;

            try
            {
                file_manager->download(file_id,file_buffer);

            }
            catch (std::exception &ex)
            {
                LOGGER(PACKAGE).error(boost::format("filemanager download: '%1%' error processing letter_id: %2% file_id: %3%") % ex.what()
                        % proc_reg_letters[i].letter_id % proc_reg_letters[i].file_id );
                fm_failed_reg_letters.push_back(proc_reg_letters[i]);//save failed letter
                proc_reg_letters.erase(proc_reg_letters.begin()+i);
                continue;
            }

            std::ofstream letter_file;
            letter_file.open(letter_file_name.c_str() //./letter$FILEID.pdf
              ,std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

            if(letter_file.is_open())
            {
                letter_file.write(&(file_buffer[0]), file_buffer.size());
            }
            else
            {
                throw std::runtime_error("letterfile open error");
            }

            addr_list+=std::string("echo \"\n")
                    +" "+proc_reg_letters[i].postal_address.name +" ;"
                    +" "+proc_reg_letters[i].postal_address.org +" ;"
                    +" "+proc_reg_letters[i].postal_address.street1 +" ;"
                    +" "+proc_reg_letters[i].postal_address.street2 +" ;"
                    +" "+proc_reg_letters[i].postal_address.street3 +" ;"
                    +" "+proc_reg_letters[i].postal_address.city +" ;"
                    +" "+proc_reg_letters[i].postal_address.state +" ;"
                    +" "+proc_reg_letters[i].postal_address.code +" ;"
                    +" "+proc_reg_letters[i].postal_address.country +" ;"
                    +"\";";

          }//for letter files

          //process letter ids
          messages_manager->set_letter_status(
                  fm_failed_reg_letters,"send_failed",batch_id, comm_type, max_attempts_limit);


          //concat letters
          {
              SubProcessOutput sub_output = ShellCmd(
                  "gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=all.pdf letter*.pdf"
                      " > /dev/null 2>&1 || echo gs return code $? 1>&2"
                  , timeout).execute();
              if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
          }

          //send all.pdf by email

          //get email from cfg
          std::string email = params.email;

          if(email.empty()) throw std::runtime_error("email required");

          {
              std::string cmd=
              std::string("{\n"
              "echo \"Subject: Registered letters to send $(date +'%Y-%m-%d')\n"
              "From: ")+email+"\nContent-Type: multipart/mixed; boundary=\"SSSSSS\""
              "\n--SSSSSS\nContent-Disposition: attachment; filename=registered_letters_$(date +'%Y-%m-%d').pdf"
              "\nContent-Type: application/pdf; charset=UTF-8\nContent-Transfer-Encoding: base64\n\n\";"
              "\nbase64 ./all.pdf\n "
              "echo \"\n\n--SSSSSS\n\nbatch id: "+batch_id+"\n\n\";"
              +addr_list+
              "\n} | /usr/sbin/sendmail "+email;

              SubProcessOutput sub_output = ShellCmd(cmd, timeout).execute();
              //std::cout << "out: " << sub_output.stdout<< "out length: " << sub_output.stdout.length()
                //            << " err: " << sub_output.stderr << " err length: " << sub_output.stderr.length() << std::endl;
              if (!sub_output.stderr.empty()) throw std::runtime_error(sub_output.stderr);
          }


          //process letter ids
          messages_manager->set_letter_status(
                  proc_reg_letters,new_status,batch_id, comm_type, max_attempts_limit);

          std::cout << "new registered letters found" << std::endl;

    }
    catch (std::exception &ex) {

        std::string set_letter_status_result;
        try
        {
            proc_reg_letters.insert(proc_reg_letters.end(),fm_failed_reg_letters.begin(),fm_failed_reg_letters.end());
            messages_manager->set_letter_status(
                      proc_reg_letters,"send_failed",batch_id, comm_type, max_attempts_limit);
        }
        catch (std::exception &ex)
        {
            set_letter_status_result +=std::string("set_letter_status: ")+ex.what();
        }

        catch(...)
        {
            set_letter_status_result +="set_letter_status failed";
        }

      throw std::runtime_error(str(boost::format(
                      "notify_registered_letters_manual_send_impl: %1% %2%")
                      % ex.what() % set_letter_status_result));
    }
    catch (...) {

        std::string set_letter_status_result;
        try
        {
            proc_reg_letters.insert(proc_reg_letters.end(),fm_failed_reg_letters.begin(),fm_failed_reg_letters.end());
            messages_manager->set_letter_status(
                      proc_reg_letters,"send_failed",batch_id, comm_type, max_attempts_limit);
        }
        catch (std::exception &ex)
        {
            set_letter_status_result +=std::string("set_letter_status: ")+ex.what();
        }

        catch(...)
        {
            set_letter_status_result +="set_letter_status failed";
        }

      throw std::runtime_error(std::string("notify_registered_letters_manual_send_impl: unknown error ")+set_letter_status_result);
    }

      return ;
}//

} // namespace Admin;

