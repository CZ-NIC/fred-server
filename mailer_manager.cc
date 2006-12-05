#include "mailer_manager.h"

MailerManager::MailerManager(NameService *ns)
  throw (RESOLVE_FAILED)
{
  try {
    mailer = ccReg::Mailer::_narrow(ns->resolve("Mailer","fred"));
  } catch (...) { throw RESOLVE_FAILED(); }
}

unsigned long 
MailerManager::sendEmail(
  const std::string& from,
  const std::string& to,
  const std::string& subject,
  const std::string& mailTemplate,
  Register::Mailer::Parameters params,
  Register::Mailer::Handles handles
)
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
    handleList[0] = CORBA::string_dup(handles[i].c_str());
  // no attachments at this time
  ccReg::Lists attachments;
  // send immidiately
  bool prev = false;
  CORBA::String_var prevMsg;
  // call mailer
  CORBA::Long id = mailer->mailNotify(
    mailType,header,data,handleList,attachments,prev,prevMsg
  );
  return (unsigned long)id;
} 
