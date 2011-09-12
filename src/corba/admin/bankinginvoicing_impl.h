#ifndef _BANKING_IMPL_H_
#define _BANKING_IMPL_H_

#include <string>
#include "fredlib/invoicing/invoice.h"
#include "corba/nameservice.h"

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

#endif // _BANKING_IMPL_H_
