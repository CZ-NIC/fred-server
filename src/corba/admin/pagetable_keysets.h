#ifndef PAGETABLE_KEYSETS_H_452105AE1547420DA08510AA98FAE347
#define PAGETABLE_KEYSETS_H_452105AE1547420DA08510AA98FAE347

#include "pagetable_impl.h"

class ccReg_KeySets_i : public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:
    std::auto_ptr<Fred::Keyset::List> m_kl;

public:
    ccReg_KeySets_i(Fred::Keyset::List *kl, const Settings *_ptr);
    ~ccReg_KeySets_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Fred::Keyset::Keyset *findId(ccReg::TID _id);
};

#endif // PAGETABLE_KEYSETS_H_

