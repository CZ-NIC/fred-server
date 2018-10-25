#ifndef PAGETABLE_FILES_HH_FBEA4C0A6EAB48FAB91EE21A7845E074
#define PAGETABLE_FILES_HH_FBEA4C0A6EAB48FAB91EE21A7845E074

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "src/libfred/file.hh"

class ccReg_Files_i : public ccReg_PageTable_i,
                      public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::File::List> file_list_;

public:
  ccReg_Files_i(LibFred::File::List *_list);
  ~ccReg_Files_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  LibFred::File::File* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_FILES_H_*/
