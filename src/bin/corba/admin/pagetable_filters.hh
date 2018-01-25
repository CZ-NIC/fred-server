#ifndef PAGETABLE_FILTERS_HH_74B4C35E5F934C618BBCC1967E38525D
#define PAGETABLE_FILTERS_HH_74B4C35E5F934C618BBCC1967E38525D

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Filters_i : public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  LibFred::Filter::List& m_filter_list;
 
public:
  ccReg_Filters_i(LibFred::Filter::List& _filter_list);
  ~ccReg_Filters_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
};

#endif /*PAGETABLE_FILTERS_H_*/