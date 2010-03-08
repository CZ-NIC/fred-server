#ifndef _BANKING_IMPL_H_
#define _BANKING_IMPL_H_

#include <string>
#include "register/invoice_manager.h"
#include "corba/nameservice.h"

class ccReg_BankingInvoicing_i:
    public POA_ccReg::BankingInvoicing,
    public PortableServer::RefCountServantBase {
private:
    NameService *ns_;

    std::string m_connection_string;
    bool factoringOne(
            Register::Invoicing::Manager *manager,
            ccReg::TID zone,
            ccReg::TID registrar,
            const Database::Date &todate,
            const Database::Date &taxdate);
public:
    ccReg_BankingInvoicing_i(NameService *_ns) : ns_(_ns)
    { }
    ~ccReg_BankingInvoicing_i()
    { }
    /*
    bool addBankAccount(
            const char *zoneName,
            const char *accountNumber,
            const char *accountName,
            const char *bankCode);
    void archiveInvoices(
            bool send);
            */

    bool pairPaymentRegistrarId(
            CORBA::ULongLong paymentId,
            CORBA::ULongLong registrarId);

    bool pairPaymentRegistrarHandle(
            CORBA::ULongLong paymentId,
            const char *registrarHandle);

    bool setPaymentType(
            CORBA::ULongLong payment_id,
            CORBA::Short payment_type);

    /*
    bool addPrefix(
            const char *zoneName,
            CORBA::Long type,
            CORBA::Long year,
            ccReg::TID prefix);
    bool pairInvoices();
    bool createCreditInvoice(
            ccReg::TID zone,
            ccReg::TID registrar,
            const char *price,
            const char *taxdate,
            const char *crdate);
    bool factoring(
            ccReg::TID zone,
            ccReg::TID registrar,
            const char *todate,
            const char *taxdate);
    bool addPrice(
            ccReg::TID zone,
            ccReg::BankingInvoicing::OperationType operation,
            const char *validfrom,
            const char *validto,
            const char *price,
            CORBA::Long period);
            */

};

#endif // _BANKING_IMPL_H_
