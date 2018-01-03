#ifndef PAGETABLE_PUBLICREQUESTS_HH_627D33FAE5A24136B6772CD7AC5A2EB7
#define PAGETABLE_PUBLICREQUESTS_HH_627D33FAE5A24136B6772CD7AC5A2EB7

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_PublicRequests_i : public ccReg_PageTable_i,
                               public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::PublicRequest::List> request_list_;

public:
  ccReg_PublicRequests_i(LibFred::PublicRequest::List *_list);
  ~ccReg_PublicRequests_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::PublicRequest::PublicRequest* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_PUBLICREQUESTS_H_*/
