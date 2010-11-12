#ifndef _PAGETABLE_MESSAGES_H_
#define _PAGETABLE_MESSAGES_H_

#include "pagetable_impl.h"

class ccReg_Messages_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:    
    Fred::Messages::Manager::MessageListPtr ml;
public:
    ccReg_Messages_i(Fred::Messages::Manager::MessageListPtr messageList);
    ~ccReg_Messages_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Fred::Messages::Message *findId(ccReg::TID id);

}; // class ccReg_Messages_i

#endif // _PAGETABLE_MESSAGES_H_
