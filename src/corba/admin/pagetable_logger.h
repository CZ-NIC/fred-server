#ifndef PAGETABLE_LOGGER_H_
#define PAGETABLE_LOGGER_H_

#include "pagetable_impl.h"

class ccReg_Logger_i : public ccReg_PageTable_i,
	public PortableServer::RefCountServantBase {
private: 
	std::auto_ptr<Register::Logger::List> m_lel;

public:
	//ccReg_Logger_i(Register::Request::List *list, const Settings *_ptr);
	ccReg_Logger_i(Register::Logger::List *list);
	~ccReg_Logger_i();
	DECL_PAGETABLE_I;

	ccReg::Filters::Compound_ptr add();
	Register::Logger::Request *findId(ccReg::TID _id);

public: 
	const static int NUM_COLUMNS;
};

#endif // PAGETABLE_LOGGER_H_
