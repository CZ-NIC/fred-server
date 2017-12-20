#ifndef PAGETABLE_SESSION_H_
#define PAGETABLE_SESSION_H_

#include "src/libfred/requests/session.hh"
#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_LogSession_i : public ccReg_PageTable_i,
	public PortableServer::RefCountServantBase {
private: 
	std::unique_ptr<LibFred::Session::List> m_lel;

public:
	//ccReg_Session_i(LibFred::Request::List *list, const Settings *_ptr);
	ccReg_LogSession_i(LibFred::Session::List *list);
	~ccReg_LogSession_i();
	DECL_PAGETABLE_I;

	ccReg::Filters::Compound_ptr add();
	LibFred::Session::Session *findId(ccReg::TID _id);

public: 
	const static int NUM_COLUMNS;
};

#endif // PAGETABLE_SESSION_H_
