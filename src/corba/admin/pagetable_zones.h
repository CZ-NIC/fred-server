#ifndef _PAGETABLE_ZONES_H_
#define _PAGETABLE_ZONES_H_

#include "pagetable_impl.h"

class ccReg_Zones_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:    
    Register::Zone::ZoneList *m_zoneList;
public:
    ccReg_Zones_i(Register::Zone::ZoneList *zoneList);
    ~ccReg_Zones_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Register::Zone::Zone *findId(ccReg::TID id);
}; // class ccReg_Zones_i

#endif // _PAGETABLE_ZONES_H_
