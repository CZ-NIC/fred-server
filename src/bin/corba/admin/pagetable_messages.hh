#ifndef PAGETABLE_MESSAGES_HH_345B2A5F5F0A40EAB1D6192B48989139
#define PAGETABLE_MESSAGES_HH_345B2A5F5F0A40EAB1D6192B48989139

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

#endif
