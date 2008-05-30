#ifndef PAGETABLE_MAILS_H_
#define PAGETABLE_MAILS_H_

#include "pagetable_impl.h"

class ccReg_Mails_i : public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase 
{
private:
  MailerManager mm;

public:
  ccReg_Mails_i(NameService *ns);
  ~ccReg_Mails_i();
  DECL_PAGETABLE_I;

  DECL_ATTRIBUTE_ID(id);
  DECL_ATTRIBUTE_TYPE(status,CORBA::Long);
  DECL_ATTRIBUTE_TYPE(type,CORBA::UShort);
  DECL_ATTRIBUTE_STR(handle);
  DECL_ATTRIBUTE_STR(fulltext);
  DECL_ATTRIBUTE_STR(attachment);
  DECL_ATTRIBUTE_DATETIME(createTime);
};

#endif /*PAGETABLE_MAILS_H_*/
