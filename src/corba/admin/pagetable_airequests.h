#ifndef PAGETABLE_AIREQUESTS_H_
#define PAGETABLE_AIREQUESTS_H_

#include "pagetable_impl.h"

class ccReg_AIRequests_i : public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
  Register::AuthInfoRequest::List *airl;
 public:
  ccReg_AIRequests_i(Register::AuthInfoRequest::List *_airl);
  ~ccReg_AIRequests_i();
  DECL_PAGETABLE_I;
  DECL_ATTRIBUTE_ID(id);
  DECL_ATTRIBUTE_STR(handle);
  DECL_ATTRIBUTE_TYPE(status,ccReg::AuthInfoRequest::RequestStatus);
  DECL_ATTRIBUTE_TYPE(type,ccReg::AuthInfoRequest::RequestType);
  DECL_ATTRIBUTE_DATETIME(crTime);
  DECL_ATTRIBUTE_DATETIME(closeTime);
  DECL_ATTRIBUTE_STR(reason);
  DECL_ATTRIBUTE_STR(svTRID);
  DECL_ATTRIBUTE_STR(email);
};


#endif /*PAGETABLE_AIREQUESTS_H_*/
