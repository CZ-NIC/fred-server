/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>

#include "src/bin/corba/file_manager_client.hh"

#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/notifyclient.hh"
#include "src/deprecated/libfred/info_buffer.hh"

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/hp/handle_hpmail_args.hh"

#include "util/map_at.hh"
#include "src/util/subprocess.hh"
#include "util/printable.hh"
#include "src/util/optys/handle_optys_mail_args.hh"
#include "src/util/optys/upload_client.hh"
#include "src/util/optys/download_client.hh"


#include "libfred/db_settings.hh"

#include "libfred/notifier/process_one_notification_request.hh"

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
    std::unique_ptr<LibFred::Document::Manager> docMan(
            LibFred::Document::Manager::create(
                docgen_path.get_value()//m_conf.get<std::string>(REG_DOCGEN_PATH_NAME)
                ,docgen_template_path.get_value()//m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME)
                ,fileclient_path.get_value()//m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME)
                ,m_nsAddr)
            );
    CorbaClient cc(0, NULL, m_nsAddr, nameservice_context//m_conf.get<std::string>(NS_CONTEXT_NAME)
            );
    MailerManager mailMan(cc.getNS());
    std::unique_ptr<LibFred::Zone::Manager> zoneMan(
            LibFred::Zone::Manager::create());

    LibFred::Messages::ManagerPtr msgMan
        = LibFred::Messages::create_manager();


    std::unique_ptr<LibFred::Domain::Manager> domMan(
            LibFred::Domain::Manager::create(m_db, zoneMan.get()));
    std::unique_ptr<LibFred::Contact::Manager> conMan(
            LibFred::Contact::Manager::create(
                m_db,
                restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                )
            );
    std::unique_ptr<LibFred::Nsset::Manager> nssMan(
            LibFred::Nsset::Manager::create(
                m_db,
                zoneMan.get(),
                restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                )
            );
    std::unique_ptr<LibFred::Keyset::Manager> keyMan(
            LibFred::Keyset::Manager::create(
                m_db,
                restricted_handles//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
                )
            );
    std::unique_ptr<LibFred::Registrar::Manager> regMan(
            LibFred::Registrar::Manager::create(m_db));
    std::unique_ptr<LibFred::Notify::Manager> notifyMan(
            LibFred::Notify::Manager::create(
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
            notify_state_changes_params.notify_debug ? &std::cout : NULL);
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
    std::unique_ptr<LibFred::File::Transferer> fileclient(new FileManagerClient(cc.getNS()));

    sendLetters(
           std::move(fileclient),
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


    bool string_not_empty(const std::string &_s)
    {
        return !_s.empty();
    }

    void NotifyClient::send_letters_impl(
            LibFred::File::Transferer* fileman
            , const HPCfgMap& hpmail_config
            , LibFred::Messages::LetterProcInfo& proc_letters
            , std::string& new_status
            , std::string& batch_id)
    {
        if(proc_letters.empty())
        {
            LOGGER.debug("NotifyClient::send_letters_impl: proc_letters is empty");
            return;
        }
        else
        {
            LOGGER.debug(std::string(
                "NotifyClient::send_letters_impl: proc_letters size: ")
            + boost::lexical_cast<std::string>(proc_letters.size()));
        }

        try
        {
            LOGGER.info(
                "NotifyClient::send_letters_impl: init postservice upload");
            HPMail::set(hpmail_config);

            LOGGER.debug(boost::format("using login batch id: %1%") % std::string(hpmail_config.at("hp_login_batch_id")));

            for(unsigned i=0;i<proc_letters.size();i++)
            {
                LibFred::Messages::letter_proc mp = proc_letters.at(i);

                NamedMailFile smail;
                smail.name = mp.fname;
                fileman->download(mp.file_id, smail.data);

                const LibFred::Messages::PostalAddress &mp_a = mp.postal_address;
                LOGGER.debug(boost::format("NotifyClient::send_letters_impl: adding file (id=%1%) to batch, recipient is %2%")
                        % mp.file_id % boost::algorithm::join_if(boost::assign::list_of(mp_a.name)(mp_a.org)(mp_a.street1)(mp_a.street2)
                            (mp_a.street3)(mp_a.city)(mp_a.state)(mp_a.country), ", ", string_not_empty));
                /* [](const std::string &i){ return !i.empty(); } */

                HPMail::get()->save_file_for_upload(smail);
            }
            batch_id = HPMail::get()->upload();
        }
        catch (std::exception& ex) {
            std::string msg = str(boost::format(
                "NotifyClient::send_letters_impl: error occured (%1%)")
                % ex.what());
            LOGGER.error(msg);
            std::cerr << msg << std::endl;
            new_status = "send_failed"; // set error status in database
        }
        catch (...) {
            std::string msg = "NotifyClient::send_letters_impl: unknown error occured";
            LOGGER.error(msg);
            std::cerr << msg << std::endl;
            new_status = "send_failed"; // set error status in database
        }


    }//NotifyClient::send_letters_impl


    /*
     * Split letter send queue to two batches - domestic and foreign - based on
     * provided domestic country name
     */
    class DomesticForeignLetterBatcher
    {
        public:
            DomesticForeignLetterBatcher(const std::string &_country)
                : domestic_country_name_(boost::algorithm::trim_copy(_country))
            {
            }

            void operator()(LibFred::Messages::letter_proc &_letter_info)
            {
                if (boost::algorithm::trim_copy(_letter_info.postal_address.country) == domestic_country_name_) {
                    domestic_letters_.push_back(_letter_info);
                }
                else {
                    foreign_letters_.push_back(_letter_info);
                }
            }

            LibFred::Messages::LetterProcInfo& get_foreign_letters()
            {
                return foreign_letters_;
            }

            LibFred::Messages::LetterProcInfo& get_domestic_letters()
            {
                return domestic_letters_;
            }

        private:
            std::string domestic_country_name_;
            LibFred::Messages::LetterProcInfo foreign_letters_;
            LibFred::Messages::LetterProcInfo domestic_letters_;
    };

    /**
     * Split letter send queue by message_type
     */
    class MessageTypeLetterBatcher
    {
        std::map<std::string, LibFred::Messages::LetterProcInfo> letters_by_message_type_map_;
    public:
        void operator()(LibFred::Messages::letter_proc &_letter_info)
        {
            letters_by_message_type_map_[_letter_info.message_type].push_back(_letter_info);
        }

        std::map<std::string, LibFred::Messages::LetterProcInfo>& get_letters_by_message_type_map()
        {
                return letters_by_message_type_map_;
        }

    };


  /*
   * This method sends letters from table letter_archive
   * it sets current processed row to status=6 (under processing)
   * and cancels execution at the beginning if any row in this table
   * is already being processed.
   */
  void NotifyClient::sendLetters(std::unique_ptr<LibFred::File::Transferer> fileman, const std::string &conf_file)
  {
     Logging::Context ctx("send letters");
     TRACE("[CALL] LibFred::Notify::sendLetters()");

     LibFred::Messages::ManagerPtr messages_manager = LibFred::Messages::create_manager();

     HPCfgMap hpmail_config = read_config_file<HandleHPMailArgs>(conf_file,
             CfgArgGroups::instance()->get_handler_ptr_by_type<HandleLoggingArgsGrp>()->get_log_config_dump());

     std::string domestic_country_name = "Czech Republic";
     const std::size_t max_attempts_limit = 3;

     /* regular letters handling */
     {
        std::string batch_id = std::string("");
        std::string comm_type = "letter";
        std::string service_handle = "POSTSERVIS";
        std::string default_batch_id = hpmail_config["hp_login_batch_id"];

        LibFred::Messages::LetterProcInfo proc_letters
            = messages_manager->load_letters_to_send(0, comm_type,service_handle, max_attempts_limit);

        /* split letters to domestic and foreign */
        DomesticForeignLetterBatcher batcher = std::for_each(proc_letters.begin(), proc_letters.end(),
                DomesticForeignLetterBatcher(domestic_country_name));

        LOGGER.debug(boost::format("destination country letter distribution: domestic=%1% foreign=%2%")
                % batcher.get_domestic_letters().size() % batcher.get_foreign_letters().size());

        LOGGER.debug("sending domestic letters");
        hpmail_config["hp_login_batch_id"] = default_batch_id + hpmail_config["hp_login_batch_id_suffix_domestic_letters"];
        std::string new_status = "sent";
        send_letters_impl(fileman.get(), hpmail_config, batcher.get_domestic_letters(), new_status, batch_id);
        messages_manager->set_letter_status(batcher.get_domestic_letters(), new_status, batch_id,
                comm_type, service_handle, max_attempts_limit);

        LOGGER.debug("sending foreign letters");
        hpmail_config["hp_login_batch_id"] = default_batch_id + hpmail_config["hp_login_batch_id_suffix_foreign_letters"];
        new_status = "sent";
        send_letters_impl(fileman.get(), hpmail_config, batcher.get_foreign_letters(), new_status, batch_id);
        messages_manager->set_letter_status(batcher.get_foreign_letters(), new_status, batch_id,
                comm_type, service_handle, max_attempts_limit);
     }

     /* registered letters handling */
     {
        if(hpmail_config["hp_login_registered_letter_batch_id"].empty())
        {
            LOGGER.info(
                    "NotifyClient::sendLetters: not sending registered letters");
            return;
        }

        std::string batch_id = std::string("");
        std::string comm_type = "registered_letter";
        std::string service_handle = "POSTSERVIS";

        LOGGER.debug(std::string(
                "NotifyClient::sendLetters: hp_login_registered_letter_batch_id ")
                + hpmail_config["hp_login_registered_letter_batch_id"]);

        hpmail_config["hp_login_batch_id"] = hpmail_config["hp_login_registered_letter_batch_id"];
        std::string default_batch_id = hpmail_config["hp_login_batch_id"];

        LibFred::Messages::LetterProcInfo proc_reg_letters
            = messages_manager->load_letters_to_send(0, comm_type,service_handle, max_attempts_limit);

        /* split letters to domestic and foreign */
        DomesticForeignLetterBatcher batcher = std::for_each(proc_reg_letters.begin(), proc_reg_letters.end(),
                DomesticForeignLetterBatcher(domestic_country_name));

        LOGGER.debug(boost::format("destination country registered letter distribution: domestic=%1% foreign=%2%")
                % batcher.get_domestic_letters().size() % batcher.get_foreign_letters().size());

        LOGGER.debug("sending domestic registered letters");
        hpmail_config["hp_login_batch_id"] = default_batch_id + hpmail_config["hp_login_batch_id_suffix_domestic_letters"];
        std::string new_status = "sent";
        send_letters_impl(fileman.get(), hpmail_config, batcher.get_domestic_letters(), new_status, batch_id);
        messages_manager->set_letter_status(batcher.get_domestic_letters(), new_status, batch_id,
                comm_type, service_handle, max_attempts_limit);

        LOGGER.debug("sending foreign registered letters");
        hpmail_config["hp_login_batch_id"] = default_batch_id + hpmail_config["hp_login_batch_id_suffix_foreign_letters"];
        new_status = "sent";
        send_letters_impl(fileman.get(), hpmail_config, batcher.get_foreign_letters(), new_status, batch_id);
        messages_manager->set_letter_status(batcher.get_foreign_letters(), new_status, batch_id,
                comm_type, service_handle, max_attempts_limit);
     }
  }//sendLetters

  void NotifyClient::sendSMS(const std::string& command , const std::string& param_quote_by )
  {
      Logging::Context ctx("send sms");
      TRACE("[CALL] LibFred::Notify::sendSMS()");

      LibFred::Messages::ManagerPtr messages_manager
          = LibFred::Messages::create_manager();

      const std::size_t max_attempts_limit = 3;
      std::string service_handle = "MOBILEM";
      LibFred::Messages::SmsProcInfo proc_sms = messages_manager->load_sms_to_send(0, service_handle, 3);
      if(proc_sms.empty()) return;
      LOGGER.info("sms sending");

      for(unsigned i=0;i<proc_sms.size();i++)
      {
          std::string new_status = "sent";//ok status
          try
          {
              LibFred::Messages::sms_proc mp = proc_sms.at(i);
              std::string command_with_params
                  = command + " " + param_quote_by + mp.phone_number + param_quote_by
                  + " " + param_quote_by + mp.content + param_quote_by;
              if(system(command_with_params.c_str()))
              {
                  LOGGER.error(
                          std::string("NotifyClient::sendSMS error command: ")
                              + command_with_params + "failed.");
                  new_status = "send_failed"; // set error status
              }//if failed
              else
              {
                  LOGGER.info(
                          std::string("NotifyClient::sendSMS command: ")
                              + command_with_params + " OK");
              }//if ok
          }//try
          catch (std::exception& ex) {
              std::string msg = str(boost::format("error occured (%1%)") % ex.what());
              LOGGER.error(msg);
              std::cerr << msg << std::endl;
              new_status = "send_failed"; // set error status in database
          }
          catch (...) {
              std::string msg = "unknown error occured";
              LOGGER.error(msg);
              std::cerr << msg << std::endl;
              new_status = "send_failed"; // set error status in database
          }

          proc_sms.at(i).new_status = new_status;//seting new status

      }//for i


      //set status
      messages_manager->set_sms_status(proc_sms, service_handle, max_attempts_limit);
  }//sendSMS

  void NotifyClient::sendFile(const std::string &filename, const std::string &conf_file)  {

      TRACE("[CALL] LibFred::Notify::sendFile()");


      HPCfgMap hpmail_config = read_config_file<HandleHPMailArgs>(conf_file,
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleLoggingArgsGrp>()->get_log_config_dump());

      LOGGER.debug(boost::format("File to send %1% ") % filename);

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

//#4712#comment:23
void notify_registered_letters_manual_send_impl(const std::string& nameservice_host_port
        , const std::string& nameservice_context
        , const RegisteredLettersManualSendArgs& params
        )
{

    LibFred::Messages::ManagerPtr messages_manager
        = LibFred::Messages::create_manager();

    //needed by fallback
    LibFred::Messages::LetterProcInfo proc_reg_letters;
    LibFred::Messages::LetterProcInfo fm_failed_reg_letters;//file manager failed letters
    std::string batch_id = std::string("manual send ")
        + boost::posix_time::to_iso_extended_string(
                boost::posix_time::microsec_clock::universal_time())+" UTC";
    std::string comm_type = "registered_letter";
    std::string service_handle = "MANUAL";
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
              .error(msg+err_msg);
          throw std::runtime_error(msg+err_msg);
        }

        //checks

        //if rm is there
        if (!Cmd::Executable("which")("rm").run_with_path(timeout).succeeded()) {
          throw std::runtime_error("rm: command not found");
        }
        //if gs is there
        if (!Cmd::Executable("which")("gs").run_with_path(timeout).succeeded()) {
          throw std::runtime_error("gs: command not found");
        }
        //if base64 is there
        if (!Cmd::Executable("which")("base64").run_with_path(timeout).succeeded()) {
          throw std::runtime_error("base64: command not found");
        }
        //if sendmail is there
        if (!Cmd::Executable("test")("-x")("/usr/sbin/sendmail").run_with_path(timeout).succeeded()) {
          throw std::runtime_error("/usr/sbin/sendmail: command not found");
        }

        // init file manager
        CorbaClient corba_client(0, 0
              , nameservice_host_port
              , nameservice_context);//NS_CONTEXT_NAME
        FileManagerClient fm_client(corba_client.getNS());
        LibFred::File::ManagerPtr file_manager(
              LibFred::File::Manager::create(&fm_client));

        //read letters
        std::string new_status = "sent";

         proc_reg_letters = messages_manager
                 ->load_letters_to_send(0, comm_type, service_handle, max_attempts_limit);

          if (proc_reg_letters.size() == 0)
          {
              Logging::Manager::instance_ref().debug("no registered letters found");

              //get email from cfg
              std::string email = params.email;

              if(email.empty()) throw std::runtime_error("email required");

              {
                  const std::string data =
                      "Subject: No new registered letters " + boost::gregorian::to_iso_extended_string(boost::gregorian::day_clock::local_day()) + "\n"
                      "From: " + email + "\n"
                      "Content-Type: text/plain; charset=UTF-8; format=flowed\n"
                      "Content-Transfer-Encoding: 8bit\n"
                      "\n"
                      "no new registered letters\n";
                  const SubProcessOutput sub_output =
                      Cmd::Data(data).into("/usr/sbin/sendmail")(email).run(timeout);
                  if (!sub_output.succeeded()) {
                      throw std::runtime_error(sub_output.stderr);
                  }
              }

              std::cout << "no new registered letters found" << std::endl;
              return;
          }

          //remove old letter files
          {
            SubProcessOutput sub_output = ShellCmd("rm -f letter*.pdf", timeout).execute();
            if (!sub_output.succeeded()) {
                throw std::runtime_error(sub_output.stderr);
            }
          }
          {
            SubProcessOutput sub_output = ShellCmd("rm -f all.pdf", timeout).execute();
            if (!sub_output.succeeded()) {
                throw std::runtime_error(sub_output.stderr);
            }
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
                LOGGER.error(boost::format("filemanager download: '%1%' error processing letter_id: %2% file_id: %3%") % ex.what()
                        % proc_reg_letters[i].letter_id % proc_reg_letters[i].file_id );
                fm_failed_reg_letters.push_back(proc_reg_letters[i]);//save failed letter
                proc_reg_letters.erase(proc_reg_letters.begin()+i);
                --i;    // correction of index for deleted element
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

            addr_list += "\n " +
                proc_reg_letters[i].postal_address.name    + " ; " +
                proc_reg_letters[i].postal_address.org     + " ; " +
                proc_reg_letters[i].postal_address.street1 + " ; " +
                proc_reg_letters[i].postal_address.street2 + " ; " +
                proc_reg_letters[i].postal_address.street3 + " ; " +
                proc_reg_letters[i].postal_address.city    + " ; " +
                proc_reg_letters[i].postal_address.state   + " ; " +
                proc_reg_letters[i].postal_address.code    + " ; " +
                proc_reg_letters[i].postal_address.country + " ;";

          }//for letter files

          //process letter ids
          messages_manager->set_letter_status(
                  fm_failed_reg_letters,"send_failed",batch_id, comm_type, service_handle, max_attempts_limit);


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
              const std::string date = boost::gregorian::to_iso_extended_string(boost::gregorian::day_clock::local_day());
              const std::string filename = "registered_letters_" + date + ".pdf";
              const std::string data =
                  "Subject: Registered letters to send " + date + "\n"
                  "From: " + email + "\n"
                  "Content-Type: multipart/mixed; boundary=\"SSSSSS\"\n"
                  "--SSSSSS\n"
                  "Content-Disposition: attachment; filename=" + filename + "\n"
                  "Content-Type: application/pdf; charset=UTF-8\n"
                  "Content-Transfer-Encoding: base64\n"
                  "\n" +
                  Cmd::Executable("base64")("./all.pdf").run_with_path(timeout).stdout + "\n"
                  "\n"
                  "--SSSSSS\n"
                  "\n"
                  "batch id: " + batch_id + "\n"
                  "\n" + addr_list;
              SubProcessOutput sub_output = Cmd::Data(data).into("/usr/sbin/sendmail")(email)
                                            .run(timeout);
              //std::cout <<  "out: " << sub_output.stdout << " out length: " << sub_output.stdout.length()
              //          << " err: " << sub_output.stderr << " err length: " << sub_output.stderr.length() << std::endl;
              if (!sub_output.succeeded()) {
                  throw std::runtime_error(sub_output.stderr);
              }
          }


          //process letter ids
          messages_manager->set_letter_status(
                  proc_reg_letters,new_status,batch_id, comm_type, service_handle, max_attempts_limit);

          std::cout << "new registered letters found" << std::endl;

    }
    catch (std::exception &ex) {

        std::string set_letter_status_result;
        try
        {
            proc_reg_letters.insert(proc_reg_letters.end(),fm_failed_reg_letters.begin(),fm_failed_reg_letters.end());
            messages_manager->set_letter_status(
                      proc_reg_letters,"send_failed",batch_id, comm_type, service_handle, max_attempts_limit);
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
                      proc_reg_letters,"send_failed",batch_id, comm_type, service_handle, max_attempts_limit);
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

/**
 * comparison of letter_proc structure with given letter_id
 */
class eq_letter_id
{
    unsigned long long letter_id_;
public:
    eq_letter_id(unsigned long long letter_id)
    : letter_id_(letter_id)
    {}
    bool operator()(const LibFred::Messages::letter_proc& lp) const
    {
        return  lp.letter_id == letter_id_;
    }
};
/**
 * Set status and batch_id of letters send via Optys with given comm_type and service_handle.
 * Set status "sent" to letters in message_type_letters_map and not in failed_letters_by_batch_id_map.
 * Set status "send_failed" to letters in failed_letters_by_batch_id_map.
 * Reset state of unprocessed letters according to send attempts.
 */
void set_optys_letter_status(
    const std::map<std::string, LibFred::Messages::LetterProcInfo>& failed_letters_by_batch_id_map
    , const std::map<std::string, LibFred::Messages::LetterProcInfo>& message_type_letters_map
    , const std::string& comm_type
    , const std::string& service_handle
    , const std::size_t max_attempts_limit
    , const std::string& zip_filename_before_message_type
    , const std::string& zip_filename_after_message_type
    , LibFred::Messages::ManagerPtr messages_manager)
{
    //for maybe sent letters split by message_type
    for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
        ci = message_type_letters_map.begin();
        ci != message_type_letters_map.end(); ++ci)
    {
        const std::string batch_id = zip_filename_before_message_type+ ci->first + zip_filename_after_message_type;//have to match batch_id in upload client

        LibFred::Messages::LetterProcInfo letters = ci->second;

        std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            failed_letters_ci = failed_letters_by_batch_id_map.find(batch_id);

        //if batch_id not found in failed letters then set "sent" status
        if(failed_letters_ci == failed_letters_by_batch_id_map.end())
        {
            messages_manager->set_letter_status(
                letters,"sent",batch_id, comm_type, service_handle, max_attempts_limit);
        }
        else //if batch_id found in failed letters
        {
            LibFred::Messages::LetterProcInfo failed_letters = failed_letters_ci->second;
            LibFred::Messages::LetterProcInfo sent_letters;
            for(std::size_t i = 0; i < letters.size(); ++i)
            {
                //if letter not failed
                if(std::find_if(failed_letters.begin(), failed_letters.end(),
                        eq_letter_id(letters[i].letter_id)) == failed_letters.end())
                {
                    sent_letters.push_back(letters[i]);
                }
            }//for

            messages_manager->set_letter_status(
                    sent_letters,"sent",batch_id, comm_type, service_handle, max_attempts_limit);

            messages_manager->set_letter_status(
                    failed_letters,"send_failed",batch_id, comm_type, service_handle, max_attempts_limit);
        }//batch_id found
    }
}

void notify_letters_optys_send_impl(
        const std::string& nameservice_host_port
        , const std::string& nameservice_context
        , const std::string& optys_config_file
        )
{
    // init managers
    CorbaClient corba_client(0, 0
          , nameservice_host_port
          , nameservice_context);//NS_CONTEXT_NAME
    FileManagerClient fm_client(corba_client.getNS());
    std::shared_ptr<LibFred::File::Manager> file_manager(
          LibFred::File::Manager::create(&fm_client));

    LibFred::Messages::ManagerPtr messages_manager
        = LibFred::Messages::create_manager();

    //optys config
    std::map<std::string, std::string> set_cfg = read_config_file<HandleOptysMailArgs>(optys_config_file,
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleLoggingArgsGrp>()->get_log_config_dump());

    std::string domestic_country_name = "Czech Republic";
    const std::size_t max_attempts_limit = 3;

    std::string yyyymmddthhmmss = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
    std::string service_handle = "OPTYS";
    std::string zip_file_name_domestic_before_message_type(yyyymmddthhmmss + "-D-");
    std::string zip_file_name_foreign_before_message_type(yyyymmddthhmmss + "-F-");

    std::string zip_filename_letter_after_message_type = "-OLZ";
    std::string zip_filename_registered_letter_after_message_type = "-DOP";

    LibFred::Messages::LetterProcInfo proc_letters
        = messages_manager->load_letters_to_send(0, "letter",service_handle, max_attempts_limit);
    LibFred::Messages::LetterProcInfo proc_registered_letters
        = messages_manager->load_letters_to_send(0, "registered_letter",service_handle, max_attempts_limit);

    //split letters by country to domestic and foreign
    DomesticForeignLetterBatcher country_letters_batcher = std::for_each(proc_letters.begin(), proc_letters.end(),
        DomesticForeignLetterBatcher(domestic_country_name));
    DomesticForeignLetterBatcher country_registered_letters_batcher = std::for_each(
        proc_registered_letters.begin(), proc_registered_letters.end(),DomesticForeignLetterBatcher(domestic_country_name));

    //split letters by message_type
    MessageTypeLetterBatcher message_type_domestic_lettes_batcher = std::for_each(
        country_letters_batcher.get_domestic_letters().begin(),
        country_letters_batcher.get_domestic_letters().end(), MessageTypeLetterBatcher());
    MessageTypeLetterBatcher message_type_foreign_letters_batcher = std::for_each(
        country_letters_batcher.get_foreign_letters().begin(),
        country_letters_batcher.get_foreign_letters().end(), MessageTypeLetterBatcher());
    MessageTypeLetterBatcher message_type_domestic_registered_letters_batcher = std::for_each(
        country_registered_letters_batcher.get_domestic_letters().begin(),
        country_registered_letters_batcher.get_domestic_letters().end(), MessageTypeLetterBatcher());
    MessageTypeLetterBatcher message_type_foreign_registered_letters_batcher = std::for_each(
        country_registered_letters_batcher.get_foreign_letters().begin(),
        country_registered_letters_batcher.get_foreign_letters().end(), MessageTypeLetterBatcher());

    std::map<std::string, LibFred::Messages::LetterProcInfo> failed_letters_by_batch_id_map;

    try
    {
        //send letters and collect errors
        failed_letters_by_batch_id_map =
        OptysUploadClient(map_at(set_cfg,"host"),
            boost::lexical_cast<int>(map_at(set_cfg,"port")),
            map_at(set_cfg,"user"),
            map_at(set_cfg,"password"),
            map_at(set_cfg, "zip_tmp_dir"),
            (map_at(set_cfg,"cleanup_zip_tmp_dir") == "true"),
            file_manager)
        .zip_letters(message_type_domestic_lettes_batcher.get_letters_by_message_type_map(),
            zip_file_name_domestic_before_message_type,zip_filename_letter_after_message_type)
        .zip_letters(message_type_foreign_letters_batcher.get_letters_by_message_type_map(),
            zip_file_name_foreign_before_message_type,zip_filename_letter_after_message_type)
        .zip_letters(message_type_domestic_registered_letters_batcher.get_letters_by_message_type_map(),
            zip_file_name_domestic_before_message_type,zip_filename_registered_letter_after_message_type)
        .zip_letters(message_type_foreign_registered_letters_batcher.get_letters_by_message_type_map(),
            zip_file_name_foreign_before_message_type,zip_filename_registered_letter_after_message_type)
        .scp_upload();
    }
    catch(const ScpUploadException& scp_ex)
    {
        std::set<std::string> sent_files = scp_ex.get_sent_zip_file_relative_names();

        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_domestic_lettes_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_domestic_lettes_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_domestic_before_message_type+ ci->first + zip_filename_letter_after_message_type;
            std::string zip_file_name = batch_id + ".zip";
            if(sent_files.find(zip_file_name) == sent_files.end()) //file was not sent
            {
                messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "letter", service_handle, max_attempts_limit);
            }
        }
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_foreign_letters_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_foreign_letters_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_foreign_before_message_type+ ci->first + zip_filename_letter_after_message_type;
            std::string zip_file_name = batch_id + ".zip";
            if(sent_files.find(zip_file_name) == sent_files.end()) //file was not sent
            {
                messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "letter", service_handle, max_attempts_limit);
            }
        }
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_domestic_registered_letters_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_domestic_registered_letters_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_domestic_before_message_type+ ci->first + zip_filename_registered_letter_after_message_type;
            std::string zip_file_name = batch_id + ".zip";
            if(sent_files.find(zip_file_name) == sent_files.end()) //file was not sent
            {
                messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "registered_letter", service_handle, max_attempts_limit);
            }
        }
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_foreign_registered_letters_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_foreign_registered_letters_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_foreign_before_message_type+ ci->first + zip_filename_registered_letter_after_message_type;
            std::string zip_file_name = batch_id + ".zip";
            if(sent_files.find(zip_file_name) == sent_files.end()) //file was not sent
            {
                messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "registered_letter", service_handle, max_attempts_limit);
            }
        }

        throw std::runtime_error(scp_ex.what());
    }
    catch(...)
    {
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_domestic_lettes_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_domestic_lettes_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_domestic_before_message_type+ ci->first + zip_filename_letter_after_message_type;
            messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "letter", service_handle, max_attempts_limit);
        }
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_foreign_letters_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_foreign_letters_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_foreign_before_message_type+ ci->first + zip_filename_letter_after_message_type;
            messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "letter", service_handle, max_attempts_limit);
        }
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_domestic_registered_letters_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_domestic_registered_letters_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_domestic_before_message_type+ ci->first + zip_filename_registered_letter_after_message_type;
            messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "registered_letter", service_handle, max_attempts_limit);
        }
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_foreign_registered_letters_batcher.get_letters_by_message_type_map().begin();
            ci != message_type_foreign_registered_letters_batcher.get_letters_by_message_type_map().end(); ++ci)
        {
            std::string batch_id = zip_file_name_foreign_before_message_type+ ci->first + zip_filename_registered_letter_after_message_type;
            messages_manager->set_letter_status(ci->second,"send_failed", batch_id, "registered_letter", service_handle, max_attempts_limit);
        }
        throw;
    }

    //set sent or send_failed status
    set_optys_letter_status(failed_letters_by_batch_id_map,
        message_type_domestic_lettes_batcher.get_letters_by_message_type_map(),
        "letter",service_handle, max_attempts_limit,
        zip_file_name_domestic_before_message_type, zip_filename_letter_after_message_type,
        messages_manager);
    set_optys_letter_status(failed_letters_by_batch_id_map,
        message_type_foreign_letters_batcher.get_letters_by_message_type_map(),
        "letter",service_handle, max_attempts_limit,
        zip_file_name_foreign_before_message_type, zip_filename_letter_after_message_type,
        messages_manager);
    set_optys_letter_status(failed_letters_by_batch_id_map,
        message_type_domestic_registered_letters_batcher.get_letters_by_message_type_map(),
        "registered_letter",service_handle, max_attempts_limit,
        zip_file_name_domestic_before_message_type, zip_filename_registered_letter_after_message_type,
        messages_manager);
    set_optys_letter_status(failed_letters_by_batch_id_map,
        message_type_foreign_registered_letters_batcher.get_letters_by_message_type_map(),
        "registered_letter",service_handle, max_attempts_limit,
        zip_file_name_foreign_before_message_type, zip_filename_registered_letter_after_message_type,
        messages_manager);
}

static std::string join_map(const std::map<std::string, std::string>& _map) {
    std::vector<std::string> key_value_pairs;
    for(std::map<std::string, std::string>::const_iterator it = _map.begin();
        it != _map.end();
        ++it
    ) {
        key_value_pairs.push_back( it->first + "=" + it->second );
    }

    return boost::algorithm::join(key_value_pairs, " ");
}

void send_object_event_notification_emails_impl(std::shared_ptr<LibFred::Mailer::Manager> _mailer) {

    while(true) {
        LibFred::OperationContextCreator ctx;
        try {
            if( ! Notification::process_one_notification_request(ctx, _mailer) ) {
                break;
            }
        } catch(const Notification::FailedToSendMail& e) {
            /* avoid loop by comitting the transaction and thus removing the problematic notification request */
            ctx.commit_transaction();

            LOGGER.error(
                "send_object_event_notification_emails_impl: failed to send letter "
                    "event: "                   + to_db_handle( e.failed_request_data.event.get_event() )   + " " +
                    "object_type: "             + to_db_handle( e.failed_request_data.event.get_type() )    + " " +
                    "event_done_by_registrar: " + boost::lexical_cast<std::string>( e.failed_request_data.done_by_registrar ) + " " +
                    "history_id_post_change: "  + boost::lexical_cast<std::string>( e.failed_request_data.history_id_post_change ) + " " +
                    "failed_recipient: "        + e.failed_recipient + " " +
                    "skipped_recipients: "      + boost::algorithm::join( e.skipped_recipients, ", " ) + " " +
                    "email_template: "          + e.template_name + " " +
                    "email_parameters: "        + join_map( e.template_parameters )
            );

            throw std::runtime_error("send_object_event_notification_emails_impl: failed to send letter ");

        } catch(const Notification::FailedToLockRequest& e) {
            LOGGER.error(
                "unable to get data from notification queue - another process is running"
            );

            throw std::runtime_error("unable to get data from notification queue - another process is running");
        }
        ctx.commit_transaction();
    }

}

} // namespace Admin;

