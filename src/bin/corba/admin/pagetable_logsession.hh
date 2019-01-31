#ifndef PAGETABLE_LOGSESSION_HH_6FA7C69F550246F1A3BFE920EDB1C534
#define PAGETABLE_LOGSESSION_HH_6FA7C69F550246F1A3BFE920EDB1C534

#include "src/deprecated/libfred/requests/session.hh"
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

#endif
