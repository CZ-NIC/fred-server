#include "mailer_manager.h"

MailerManager::MailerManager(NameService *ns)
  throw (RESOLVE_FAILED)
{
  try {
    mailer = ccReg::Mailer::_narrow(ns->resolve("Mailer"));
  } catch (...) { throw RESOLVE_FAILED(); }
}

Register::TID 
MailerManager::sendEmail(
  const std::string& from,
  const std::string& to,
  const std::string& subject,
  const std::string& mailTemplate,
  Register::Mailer::Parameters params,
  Register::Mailer::Handles handles,
  Register::Mailer::Attachments attach
) throw (Register::Mailer::NOT_SEND)
{
  // prepare header
  ccReg::MailHeader header;
  header.h_from = CORBA::string_dup(from.c_str());
  header.h_to = CORBA::string_dup(to.c_str());
  // header.h_subject = CORBA::string_dup(subject.c_str());
  // prepare template
  const char * mailType = CORBA::string_dup(mailTemplate.c_str()); 
  // prepare data
  ccReg::KeyValues data;
  data.length(params.size());
  Register::Mailer::Parameters::const_iterator i; // source position
  unsigned j; // result position
  for (i=params.begin(),j=0; i!=params.end(); i++,j++) {
    data[j].key = CORBA::string_dup(i->first.c_str());
    data[j].value =CORBA::string_dup(i->second.c_str());
  }
  // prepare handles
  ccReg::Lists handleList;
  handleList.length(handles.size());
  for (unsigned i=0; i<handles.size(); i++)
    handleList[i] = CORBA::string_dup(handles[i].c_str());
  ccReg::Attachment_seq attachments;
  attachments.length(attach.size());
  for (unsigned i=0; i<attach.size(); i++)
    attachments[i] = attach[i];
  // send immidiately
  bool prev = false;
  CORBA::String_var prevMsg;
  // call mailer
  try {
    CORBA::Long id = mailer->mailNotify(
      mailType,header,data,handleList,attachments,prev,prevMsg
    );
    return (unsigned long)id;
  } catch (...) {
    throw Register::Mailer::NOT_SEND();
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
MailerManager::reload(MailerManager::Filter& f) throw (LOAD_ERROR)
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
    mailList.clear();
    ccReg::MailSearch_var ms = mailer->createSearchObject(mf);
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
        for (unsigned j=0; j<m.handles.length(); j++)
          d.handles.push_back((const char *)m.handles[j]);
        for (unsigned j=0; j<m.attachments.length(); j++)
          d.attachments.push_back(m.attachments[j]);
        // TODO remove: just for testing
        if (!d.attachments.size()) d.attachments.push_back(1);
        mailList.push_back(d);
      }
    } while (mls->length());
  } catch (...) {
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
