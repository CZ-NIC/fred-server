#ifndef _INVOICE_MANAGER_H_
#define _INVOICE_MANAGER_H_

#include "invoice.h"
#include "invoice_list.h"
#include "common_impl_new.h"
#include "db_settings.h"
#include "documents.h"
#include "mailer.h"

namespace Register {
namespace Invoicing {

//-----------------------------------------------------------------------------
//
// Manager
//
//-----------------------------------------------------------------------------
class Manager {
private:
    bool setInvoiceToStatementItem( 
            Database::ID statementId, 
            Database::ID invoiceId);
    bool hasStatementAnInvoice(const Database::ID &statementId);
    bool pairAccountInvoices(bool report = false);
    bool pairCreditInvoices(bool report = false);
    Document::Manager   *m_docMan;
    Mailer::Manager       *m_mailMan;
public:
    Manager(Document::Manager *docMan, Mailer::Manager *mailMan):
        m_docMan(docMan),
        m_mailMan(mailMan)
    { }
    Manager():
        m_docMan(NULL),
        m_mailMan(NULL)
    { }
    bool manualCreateInvoice(
            const Database::ID &paymentId,
            const std::string &registrarHandle);
    bool manualCreateInvoice(
            const Database::ID &paymentId,
            const Database::ID &registrar);
    Invoice *createInvoice(Type type) const;
    Invoice *createAccountInvoice() const;
    Invoice *createDepositInvoice() const;
    bool pairInvoices(bool report = false);
    List *createList() const;
    static Manager *create(Document::Manager *docMan, Mailer::Manager *mailMan);
    static Manager *create();
    Database::Money getCreditByZone(
            const std::string &registrarHandle, 
            const unsigned long long &zoneId) const;
    void archiveInvoices(bool send) const;
    Database::Money countVat(
            const Database::Money &price,
            const unsigned int &vatRate,
            const bool &base);
    bool insertInvoicePrefix(
            const unsigned long long &zoneId,
            const int &type,
            const int &year,
            const unsigned long long &prefix);
    bool insertInvoicePrefix(
            const std::string &zoneName,
            const int &type,
            const int &year,
            const unsigned long long &prefix);
}; // class Manager;

} // namespace Invoicing
} // namespace Register

#endif // _INVOICE_MANAGER_H_
