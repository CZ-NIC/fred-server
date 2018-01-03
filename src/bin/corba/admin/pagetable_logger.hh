#ifndef PAGETABLE_LOGGER_HH_67E6A46D22DA4C55B3A4E36505728A45
#define PAGETABLE_LOGGER_HH_67E6A46D22DA4C55B3A4E36505728A45

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Logger_i : public ccReg_PageTable_i,
	public PortableServer::RefCountServantBase {
private: 
	std::unique_ptr<LibFred::Logger::List> m_lel;

public:
	//ccReg_Logger_i(LibFred::Request::List *list, const Settings *_ptr);
	ccReg_Logger_i(LibFred::Logger::List *list);
	~ccReg_Logger_i();
	DECL_PAGETABLE_I;

        void setOffset(CORBA::Long _offset);
	ccReg::Filters::Compound_ptr add();
	LibFred::Logger::Request *findId(ccReg::TID _id);

public: 
	const static int NUM_COLUMNS;
};

#endif
