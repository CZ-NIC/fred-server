#ifndef _INVOICE_MAILS_H_
#define _INVOICE_MAILS_H_

#include <iostream>
#include "mailer.h"

namespace Register {
namespace Invoicing {

class Mails {
private:
    struct Item {
        std::string     m_registrarEmail;
        Database::Date  m_from;
        Database::Date  m_to;
        Database::ID    m_filePDF;
        Database::ID    m_fileXML;
        Database::ID    m_generation;
        Database::ID    m_invoice;
        Database::ID    m_mail;
        std::string     m_zoneFqdn;

        std::string getTemplateName()
        {
            if (!m_generation) {
                return "invoice_deposit";
            }
            if (!m_invoice) {
                return "invoice_noaudit";
            }
            return "invoice_audit";
        }
        Item(const std::string &registrarEmail, Database::Date from,
                Database::Date to, Database::ID filePDF, Database::ID fileXML,
                Database::ID generation, Database::ID invoice,
                Database::ID mail, const std::string zoneFqdn):
            m_registrarEmail(registrarEmail), m_from(from), m_to(to),
            m_filePDF(filePDF), m_fileXML(fileXML), m_generation(generation),
            m_invoice(invoice), m_mail(mail), m_zoneFqdn(zoneFqdn)
        { }
    }; // struct Item;

    typedef std::vector<Item> MailItems;
    MailItems               m_items;
    Mailer::Manager         *m_mailMan;

    void store(unsigned int index);
public:
    Mails(Mailer::Manager *mailMan):
        m_mailMan(mailMan)
    { }
    void send();
    void load();
};

} // namespace Invoicing
} // namespace Register

#endif // _INVOICE_MAILS_H_
