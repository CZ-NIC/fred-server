#ifndef PAGETABLE_KEYSETS_H_
#define PAGETABLE_KEYSETS_H_

#include "pagetable_impl.h"

class ccReg_KeySets_i : public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:
    std::auto_ptr<Register::KeySet::List> m_kl;

public:
    ccReg_KeySets_i(Register::KeySet::List *kl, const Settings *_ptr);
    ~ccReg_KeySets_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Register::KeySet::KeySet *findId(ccReg::TID _id);
};

#endif // PAGETABLE_KEYSETS_H_

