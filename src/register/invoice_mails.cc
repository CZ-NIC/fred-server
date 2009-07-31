#include "db_settings.h"
#include "invoice_mails.h"

namespace Register {
namespace Invoicing {

void 
Mails::store(unsigned int index)
{
    Database::InsertQuery insertMail("invoice_mails");
    insertMail.add("invoiceid", m_items[index].m_invoice);
    insertMail.add("genid", m_items[index].m_generation);
    insertMail.add("mailid", m_items[index].m_mail);
    Database::Connection conn = Database::Manager::acquire();
    conn.exec(insertMail);
}

void 
Mails::send()
{
    TRACE("[CALL] Register::Invoicing::Mails::send()");
    for (unsigned int i = 0; i < m_items.size(); i++) {
        Item *item = &m_items[i];
        Mailer::Parameters params;
        std::stringstream dateBuffer;
        dateBuffer.imbue(std::locale(dateBuffer.getloc(),
                    new date_facet("%d.%m.%Y")));
        dateBuffer << item->m_from;
        params["fromdate"] = dateBuffer.str();
        dateBuffer.str("");
        dateBuffer << item->m_to;
        params["todate"] = dateBuffer.str();
        Mailer::Handles handles;
        params["zone"] = item->m_zoneFqdn;
        // TODO: include domain or registrar handles??
        Mailer::Attachments attach;
        if (item->m_filePDF) {
            attach.push_back(item->m_filePDF);
        }
        if (item->m_fileXML) {
            attach.push_back(item->m_fileXML);
        }
        item->m_mail = m_mailMan->sendEmail("", item->m_registrarEmail, 
                "", item->getTemplateName(), params, handles, attach);
        if (!item->m_mail) {
            LOGGER(PACKAGE).error("Register::Invoicing::Mails::send() error");
        }
        store(i);
    }
} // Mails::send()

void 
Mails::load()
{
    TRACE("[CALL] Register::Invoicing::Mails::load()");
    Database::Query loadMailsQuery;
    loadMailsQuery.buffer() 
        << "SELECT r.email, g.fromdate, g.todate, "
        << "i.file, i.fileXML, g.id, i.id, z.fqdn "
        << "FROM registrar r, invoice i "
        << "LEFT JOIN invoice_generation g ON (g.invoiceid = i.id) "
        << "LEFT JOIN invoice_mails im ON (im.invoiceid = i.id) "
        << "LEFT JOIN zone z ON (z.id = i.zone) "
        << "WHERE i.registrarid = r.id "
        << "AND im.mailid ISNULL "
        << "UNION "
        << "SELECT r.email, g.fromdate, g.todate, NULL, NULL, g.id, "
        << "NULL, z.fqdn "
        << "FROM registrar r, invoice_generation g "
        << "LEFT JOIN invoice_mails im ON (im.genid = g.id) "
        << "LEFT JOIN zone z ON (z.id = g.zone) "
        << "WHERE g.registrarid = r.id AND g.invoiceid ISNULL "
        << "AND im.mailid ISNULL "
        << "AND NOT(r.email ISNULL OR TRIM(r.email)='')";
    Database::Connection conn = Database::Manager::acquire();
    Database::Result loadMailsResult = conn.exec(loadMailsQuery);
    Database::Result::Iterator it = loadMailsResult.begin();
    for (; it != loadMailsResult.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        std::string         email       = *(col);
        Database::Date      from        = *(++col);
        Database::Date      to          = *(++col);
        Database::ID        filePDF     = *(++col);
        Database::ID        fileXML     = *(++col);
        Database::ID        generation  = *(++col);
        Database::ID        invoice     = *(++col);
        Database::ID        mail        = 0;
        std::string         zoneFqdn    = *(++col);
        m_items.push_back(Item(email, from, to, filePDF, fileXML,
                    generation, invoice, mail, zoneFqdn));
    }
} // Mails::load()

} // namespace Invoicing
} // namespace Register

