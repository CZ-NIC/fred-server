/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

#include <exception>
#include <boost/tokenizer.hpp>
#include <set>
#include <boost/algorithm/string/trim.hpp>

#include "src/bin/corba/mailer_manager.hh"
#include "src/deprecated/util/log.hh"
#include "util/log/logger.hh"

MailerManager::MailerManager(NameService *ns) : ns_ptr(ns)
{
  try {
    _resolveInit();
  }
  catch(...) {
    LOGGER.warning("can't connect to mailer in initialization...");
  }
}

LibFred::TID
MailerManager::sendEmail(
  const std::string& from,
  const std::string& to,
  const std::string& subject,
  const std::string& mailTemplate,
  const LibFred::Mailer::Parameters &params,
  const LibFred::Mailer::Handles &handles,
  const LibFred::Mailer::Attachments &attach,
  const std::string& reply_to
)
{
  LOGGER.debug(boost::format("recipients = '%1%'") % to);
  if (to.empty() || to == "NULL ") {
    LOGGER.error("recipients empty!? not sending");
    throw LibFred::Mailer::NOT_SEND();
  }
  // prepare header
  ccReg::MailHeader header;
  header.h_from = CORBA::string_dup(from.c_str());
  header.h_to = CORBA::string_dup(to.c_str());
  std::string h_reply_to = boost::algorithm::trim_copy(reply_to);
  if (!h_reply_to.empty()) {
      header.h_reply_to = CORBA::string_dup(h_reply_to.c_str());
  }
  // prepare data
  ccReg::KeyValues data;
  data.length(params.size());
  LibFred::Mailer::Parameters::const_iterator i; // source position
  unsigned j; // result position
  for (i=params.begin(),j=0; i!=params.end(); i++,j++) {
    data[j].key = CORBA::string_dup(i->first.c_str());
    data[j].value =CORBA::string_dup(i->second.c_str());
    LOGGER.debug(boost::format("param %1% '%2%'='%3%'") % j % (char*)data[j].key % (char*)data[j].value); 

  }
  // prepare handles
  ccReg::Lists handleList;
  LOGGER.debug(boost::format("handleList length is %1%") % handles.size());

  handleList.length(handles.size());
  for (unsigned i=0; i<handles.size(); i++)
  {
    handleList[i] = CORBA::string_dup(handles[i].c_str());
     LOGGER.debug(boost::format("handle '%1%'") % (char*)handleList[i]);

  }
  ccReg::Attachment_seq attachments;
  LOGGER.debug(boost::format("attachment length is %1%") % attach.size());
  attachments.length(attach.size());
  for (unsigned i=0; i<attach.size(); i++)
  {
    attachments[i] = attach[i];
    LOGGER.debug(boost::format("attachment '%1%'") % attachments[i]);

  }

  // send immidiately
  bool prev = false;
  CORBA::String_var prevMsg;
  // call mailer
  // if (CORBA::is_nil(mailer)) throw LibFred::Mailer::NOT_SEND();
  try {
    LOGGER.debug(boost::format("mailer->mailNotify mailType '%1%'") % mailTemplate.c_str());
    _resolveInit();
    CORBA::Long id = mailer->mailNotify(
      mailTemplate.c_str(),header,data,handleList,attachments,prev,prevMsg
    );
    return (unsigned long)id;
  } catch (...) {
    throw LibFred::Mailer::NOT_SEND();
  }
} 

MailerManager::List& 
MailerManager::getMailList()
{
  return mailList;
}

#define LIST_CHUNK_SIZE 100

#define SET_TIME(x,y) if (x.is_special()) {\
  y.date.year = 0; y.date.month = 0; y.date.day = 0; \
  y.hour = 0; y.minute = 0; y.second = 0; \
} else { \
  y.date.year = x.date().year(); y.date.month = x.date().month(); \
  y.date.day = x.date().day(); \
  y.hour = x.time_of_day().hours(); y.minute = x.time_of_day().minutes(); \
  y.second = x.time_of_day().seconds(); \
}  
void 
MailerManager::reload(MailerManager::Filter& f)
{
  ccReg::MailFilter mf;
  if (!f.id) mf.mailid = -1;
  else mf.mailid = f.id;
  if (!f.type) mf.mailtype = -1;
  else mf.mailtype = f.type;
  mf.status = f.status;
  mf.handle = CORBA::string_dup(f.handle.c_str());
  mf.fulltext = CORBA::string_dup(f.content.c_str());
  //TODO: filter for string
  //mf.attachment = CORBA::string_dup(f.attachment.c_str());
  mf.attachid = -1;
  SET_TIME(f.crTime.begin(),mf.crdate.from);
  SET_TIME(f.crTime.end(),mf.crdate.to);
  try {
    _resolveInit();
    // boost::mutex::scoped_lock scoped_lock(mutex);
    ccReg::MailTypeCodes_var mtc = mailer->getMailTypes();
    mailList.clear();
    LOGGER.trace("[IN] MailerManager::reload(): before mailer->createSearchObject()");
    ccReg::MailSearch_var ms = mailer->createSearchObject(mf);
    LOGGER.trace("[IN] MailerManager::reload(): after mailer->createSearchObject()");
    ccReg::MailList_var mls;
    do {
      mls = ms->getNext(LIST_CHUNK_SIZE);
      for (unsigned i=0; i<mls->length(); i++) {
        MailerManager::Detail d;
        ccReg::Mail& m = mls[i];
        d.id = m.mailid;
        d.content = m.content;
        d.createTime = m.crdate;
        d.modTime = m.moddate;
        d.status = m.status;
        d.type = m.mailtype;
        for (unsigned j=0; j<mtc->length(); j++)
          if (mtc[j].id == d.type) d.typeDesc = mtc[j].name;
        for (unsigned j=0; j<m.handles.length(); j++)
          d.handles.push_back((const char *)m.handles[j]);
        for (unsigned j=0; j<m.attachments.length(); j++)
          d.attachments.push_back(m.attachments[j]);
        mailList.push_back(d);
      }
    } while (mls->length());
  }
  catch (...) {
    throw LOAD_ERROR();
  }
}

MailerManager::Filter::Filter() :
 id(0), crTime(ptime(neg_infin),ptime(pos_infin)),
 modTime(ptime(neg_infin),ptime(pos_infin)), type(0), status(-1)
{
} 

void
MailerManager::Filter::clear()
{
  id = 0;
  type = 0;
  status = -1;
  handle = "";
  content = "";
  attachment = "";
  crTime = time_period(ptime(neg_infin),ptime(pos_infin));
} 

void 
MailerManager::_resolveInit()
{
  try {
    boost::mutex::scoped_lock scoped_lock(mutex);
    if (!CORBA::is_nil(mailer)) {
      LOGGER.debug("mailer already resolved");
      return;
    }
    LOGGER.debug("resolving corba reference");
    mailer = ccReg::Mailer::_narrow(ns_ptr->resolve("Mailer"));
 } catch (...) { 
    LOGGER.error("resolving of corba 'Mailer' object failed");
    throw RESOLVE_FAILED(); 
  }
  LOGGER.debug("resolving of corba 'Mailer' object ok");
}


/**
 * Besides of checking, also MODIFIES _email_list (removes emails without @, removes dupliacates, sorts, sets separator to " ").
 */
bool
MailerManager::checkEmailList(std::string &_email_list) const 
{
  LOGGER.debug(boost::format("checking email list '%1%'") % _email_list);
  using namespace boost;
  typedef tokenizer<char_separator<char> > token_list;

  std::set<std::string> valid;
  std::string list = _email_list;
  _email_list.clear();
  
  char_separator<char> sep(" ,");
  token_list tokens(list, sep);
  for (token_list::iterator token = tokens.begin(); token != tokens.end(); ++token) {
    /* validate email address */
    std::string::size_type i = (*token).find("@");
    if (i != std::string::npos && i != (*token).size() - 1 && i != 0) {
      /* add to output set - this removes duplicates */
      valid.insert(*token);
    }
  }

  for (std::set<std::string>::const_iterator address = valid.begin(); address != valid.end(); ++address) {
    if (!_email_list.empty())
      _email_list += " ";
    _email_list += *address;
  }

  LOGGER.debug(boost::format("check done; filtered email list '%1%'") % _email_list);
  return !_email_list.empty();
}

