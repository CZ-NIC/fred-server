#ifndef PAGETABLE_MAILS_HH_89987C2BE0114E259F627A785471CC93
#define PAGETABLE_MAILS_HH_89987C2BE0114E259F627A785471CC93

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Mails_i : public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase 
{
private:
  std::unique_ptr<LibFred::Mail::List> mail_list_;
  MailerManager mm;

public:
  ccReg_Mails_i(LibFred::Mail::List *_list, NameService *ns);
  ~ccReg_Mails_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  LibFred::Mail::Mail* findId(ccReg::TID _id);
  
//  DECL_ATTRIBUTE_ID(id);
//  DECL_ATTRIBUTE_TYPE(status,CORBA::Long);
//  DECL_ATTRIBUTE_TYPE(type,CORBA::UShort);
//  DECL_ATTRIBUTE_STR(handle);
//  DECL_ATTRIBUTE_STR(fulltext);
//  DECL_ATTRIBUTE_STR(attachment);
//  DECL_ATTRIBUTE_DATETIME(createTime);
};

#endif /*PAGETABLE_MAILS_H_*/
