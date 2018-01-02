#ifndef PAGETABLE_ZONES_HH_B496FF4EB63A448F8BFF649F3603EAB6
#define PAGETABLE_ZONES_HH_B496FF4EB63A448F8BFF649F3603EAB6

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Zones_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:    
    LibFred::Zone::Manager::ZoneListPtr m_zoneList;
public:
    ccReg_Zones_i(LibFred::Zone::Manager::ZoneListPtr zoneList);
    ~ccReg_Zones_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    LibFred::Zone::Zone *findId(ccReg::TID id);
}; // class ccReg_Zones_i

#endif
