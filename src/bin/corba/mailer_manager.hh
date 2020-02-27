/*
 * Copyright (C) 2006-2020  CZ.NIC, z. s. p. o.
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
#ifndef MAILER_MANAGER_HH_CD2883382FF043D6B8B25672F755FEE5
#define MAILER_MANAGER_HH_CD2883382FF043D6B8B25672F755FEE5

#include <stdexcept>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "corba/Mailer.hh"
#include "libfred/mailer.hh"
#include "src/bin/corba/nameservice.hh"

using namespace boost::posix_time;

/// Implementation of registry mailer
/** Use remote CORBA mailer to implement registry mailer functionality.
 * This implementation is connected to the rest of registry by supplying
 * generic pointer to abstract mailer interface */ 
class MailerManager : public LibFred::Mailer::Manager
{ 
  NameService       *ns_ptr;
  ccReg::Mailer_var mailer;
  boost::mutex      mutex;

 public:
  class RESOLVE_FAILED : public std::runtime_error
  {
  public:
      RESOLVE_FAILED()
      : std::runtime_error("MailerManager RESOLVE_FAILED")
      {}
  };
  class LOAD_ERROR : public std::runtime_error
  {
  public:
      LOAD_ERROR()
      : std::runtime_error("MailerManager LOAD_ERROR")
      {}
  };
  MailerManager(NameService *ns); 
  virtual LibFred::TID sendEmail(
    const std::string& from,
    const std::string& to,
    const std::string& subject,
    const std::string& mailTemplate,
    const LibFred::Mailer::Parameters &params,
    const LibFred::Mailer::Handles &handles,
    const LibFred::Mailer::Attachments &attach,
    const std::string& reply_to = std::string("")
  ) ;
  /**
   * Besides of checking, also MODIFIES _email_list (removes emails without @, removes dupliacates, sorts, sets separator to " ").
   */
  bool checkEmailList(std::string &_email_list) const;

  struct Filter 
  {
    Filter();
    void clear();
    LibFred::TID id;
    time_period crTime;
    time_period modTime;
    long type;
    long status;
    std::string content;
    std::string handle;
    std::string attachment;
  };
  struct Detail
  {
    LibFred::TID id;
    std::string createTime;
    std::string modTime;
    long type;
    std::string typeDesc;
    long status;
    std::string content;
    std::vector<std::string> handles;
    std::vector<LibFred::TID> attachments;
  };
  typedef std::vector<Detail> List;  
 private:
  List mailList;
  void _resolveInit();
 public:
  List& getMailList();
  void reload(Filter& mf);
};

#endif
