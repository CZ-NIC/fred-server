#ifndef _PAGETABLE_MESSAGES_H_
#define _PAGETABLE_MESSAGES_H_

#include "pagetable_impl.h"

class ccReg_Messages_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:    
    Register::Messages::Manager::MessageListPtr ml;
public:
    ccReg_Messages_i(Register::Messages::Manager::MessageListPtr messageList);
    ~ccReg_Messages_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Register::Messages::Message *findId(ccReg::TID id);

}; // class ccReg_Messages_i

#endif // _PAGETABLE_MESSAGES_H_
