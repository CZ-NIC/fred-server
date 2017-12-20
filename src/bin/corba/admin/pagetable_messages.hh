#ifndef _PAGETABLE_MESSAGES_H_
#define _PAGETABLE_MESSAGES_H_

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Messages_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:    
    LibFred::Messages::Manager::MessageListPtr ml;
public:
    ccReg_Messages_i(LibFred::Messages::Manager::MessageListPtr messageList);
    ~ccReg_Messages_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    LibFred::Messages::Message *findId(ccReg::TID id);

}; // class ccReg_Messages_i

#endif // _PAGETABLE_MESSAGES_H_
