#ifndef PAGETABLE_FILES_H_
#define PAGETABLE_FILES_H_

#include "pagetable_impl.h"

class ccReg_Files_i : public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Register::File::List> file_list_;

public:
  ccReg_Files_i(Register::File::List *_list);
  ~ccReg_Files_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  Register::File::File* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_FILES_H_*/
