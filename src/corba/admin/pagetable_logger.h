#ifndef PAGETABLE_LOGGER_H_
#define PAGETABLE_LOGGER_H_

#include "pagetable_impl.h"

class ccReg_Logger_i : public ccReg_PageTable_i,
	public PortableServer::RefCountServantBase {
private: 
	std::auto_ptr<Fred::Logger::List> m_lel;

public:
	//ccReg_Logger_i(Fred::Request::List *list, const Settings *_ptr);
	ccReg_Logger_i(Fred::Logger::List *list);
	~ccReg_Logger_i();
	DECL_PAGETABLE_I;

        void setOffset(CORBA::Long _offset);
        void setLimit(CORBA::Long _limit);
	ccReg::Filters::Compound_ptr add();
	Fred::Logger::Request *findId(ccReg::TID _id);

public: 
	const static int NUM_COLUMNS;
};

#endif // PAGETABLE_LOGGER_H_
