#ifndef PAGETABLE_SESSION_H_
#define PAGETABLE_SESSION_H_

#include "pagetable_impl.h"

class ccReg_LogSession_i : public ccReg_PageTable_i,
	public PortableServer::RefCountServantBase {
private: 
	std::auto_ptr<Register::Session::List> m_lel;

public:
	//ccReg_Session_i(Register::Request::List *list, const Settings *_ptr);
	ccReg_LogSession_i(Register::Session::List *list);
	~ccReg_LogSession_i();
	DECL_PAGETABLE_I;

	ccReg::Filters::Compound_ptr add();
	Register::Session::Session *findId(ccReg::TID _id);

public: 
	const static int NUM_COLUMNS;
};

#endif // PAGETABLE_SESSION_H_
