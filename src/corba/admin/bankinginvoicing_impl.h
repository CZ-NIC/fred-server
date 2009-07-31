#ifndef _BANKING_IMPL_H_
#define _BANKING_IMPL_H_

#include <string>
#include "register/invoice_manager.h"

class ccReg_BankingInvoicing_i:
    public POA_ccReg::BankingInvoicing,
    public PortableServer::RefCountServantBase {
private:
    std::string m_connection_string;
    bool factoringOne(
            Register::Invoicing::Manager *manager,
            unsigned long long zone,
            unsigned long long registrar,
            const Database::Date &todate,
            const Database::Date &taxdate);
public:
    ccReg_BankingInvoicing_i()
    { }
    ~ccReg_BankingInvoicing_i()
    { }
    bool addBankAccount(
            const char *zoneName,
            const char *accountNumber,
            const char *accountName,
            const char *bankCode);
    void archiveInvoices(
            bool send);
    bool createInvoiceForPayment(
            unsigned long long paymentId,
            unsigned long long registrarId);
    bool addPrefix(
            const char *zoneName,
            long type,
            long year,
            unsigned long long prefix);
    bool pairInvoices();
    bool createCreditInvoice(
            unsigned long long zone,
            unsigned long long registrar,
            const char *price,
            const char *taxdate,
            const char *crdate);
    bool factoring(
            unsigned long long zone,
            unsigned long long registrar,
            const char *todate,
            const char *taxdate);
    bool addPrice(
            unsigned long long zone,
            ccReg::BankingInvoicing::OperationType operation,
            const char *validfrom,
            const char *validto,
            const char *price,
            long period);

};

#endif // _BANKING_IMPL_H_
