#ifndef BANKINGINVOICING_IMPL_HH_03F685E625E04510AC2B1C6B1B6B5E20
#define BANKINGINVOICING_IMPL_HH_03F685E625E04510AC2B1C6B1B6B5E20

#include "src/bin/corba/Admin.hh"

#include "src/bin/corba/nameservice.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

#include <string>

class ccReg_BankingInvoicing_i:
    public POA_ccReg::BankingInvoicing,
    public PortableServer::RefCountServantBase {
private:
    NameService *ns_;

    std::string m_connection_string;

public:
    ccReg_BankingInvoicing_i(NameService *_ns) : ns_(_ns)
    { }
    ~ccReg_BankingInvoicing_i()
    { }

    bool pairPaymentRegistrarId(
            CORBA::ULongLong paymentId,
            CORBA::ULongLong registrarId);

    bool pairPaymentRegistrarHandle(
            CORBA::ULongLong paymentId,
            const char *registrarHandle);

    bool setPaymentType(
            CORBA::ULongLong payment_id,
            CORBA::Short payment_type);

};

#endif
