#ifndef PAGETABLE_PUBLICREQUESTS_H_
#define PAGETABLE_PUBLICREQUESTS_H_

#include "pagetable_impl.h"

class ccReg_PublicRequests_i : public ccReg_PageTable_i,
                               public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Register::PublicRequest::List> request_list_;

public:
  ccReg_PublicRequests_i(Register::PublicRequest::List *_list);
  ~ccReg_PublicRequests_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Register::PublicRequest::PublicRequest* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_PUBLICREQUESTS_H_*/
