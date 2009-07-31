#include "invoice_exporter.h"
#include "invoice.h"
#include "invoice_common.h"

namespace Register {
namespace Invoicing {

#define TAG(tag,f) TAGSTART(tag) \
    << "<![CDATA[" << f << "]]>" << TAGEND(tag)
#define OUTMONEY(f) (f)/100 << "." << \
    std::setfill('0') << std::setw(2) << abs(f)%100

#define INVOICE_PDF_FILE_TYPE 1 
#define INVOICE_XML_FILE_TYPE 2 

std::ostream &
ExporterXML::doExport(const Subject *subj)
{
    TRACE("[CALL] Register::Invoicing::ExporterXML::doExport(const Subject *)");
    m_out
        << TAG(id,subj->getId()) 
        << TAG(name,subj->getName()) 
        << TAG(fullname,subj->getFullname()) 
        << TAGSTART(address) 
        << TAG(street,subj->getStreet()) 
        << TAG(city,subj->getCity()) 
        << TAG(zip,subj->getZip()) 
        << TAG(country,subj->getCountry()) 
        << TAGEND(address) 
        << TAG(ico,subj->getICO()) 
        << TAG(vat_number,subj->getVatNumber()) 
        << TAG(registration,subj->getRegistration()) 
        << TAG(reclamation,subj->getReclamation()) 
        << TAG(url,subj->getURL()) 
        << TAG(email,subj->getEmail()) 
        << TAG(phone,subj->getPhone()) 
        << TAG(fax,subj->getFax()) 
        << TAG(vat_not_apply,(subj->getVatApply() ? 0 : 1));
    return m_out;
}

void 
ExporterXML::doExport(Invoice *inv)
{
    TRACE("[CALL] Register::Invoicing::ExporterXML::doExport(Invoice *)");
    m_out.imbue(std::locale(
                std::locale(
                    m_out.getloc(),
                    new time_facet("%Y-%m-%d %T")
                    ),
                new date_facet("%Y-%m-%d")
                ));
    // generate invoice xml
    if (m_xmlDecl)
        m_out << "<?xml version='1.0' encoding='utf-8'?>";
    m_out << TAGSTART(invoice)
        << TAGSTART(client);
    doExport(inv->getClient());
    m_out << TAGEND(client)
        << TAGSTART(supplier);
    doExport(inv->getSupplier());
    m_out << TAGEND(supplier)
        << TAGSTART(payment)
        << TAG(invoice_number,inv->getPrefix())
        << TAG(invoice_date,inv->getCrDate().get().date());
    if (inv->getType() == IT_DEPOSIT)
        m_out << TAG(advance_payment_date,inv->getTaxDate());
    else {
        m_out << TAG(tax_point,inv->getTaxDate())
            << TAG(period_from,inv->getAccountPeriod().begin())
            << TAG(period_to,inv->getAccountPeriod().end());
    }
    m_out << TAG(vs,inv->getVarSymbol())
        << TAGEND(payment)
        << TAGSTART(delivery)
        << TAGSTART(vat_rates);
    for (unsigned j=0; j<inv->getPaymentCount(); j++) {
        const Payment *p = inv->getPayment(j);
        m_out << TAGSTART(entry)
            << TAG(vatperc,p->getVatRate())
            << TAG(basetax,p->getPrice().format())
            << TAG(vat,p->getVat().format())
            << TAG(total,p->getPriceWithVat().format())
            << TAGSTART(years);
        for (
                inv->getAnnualPartitioning()->resetIterator(p->getVatRate());
                !inv->getAnnualPartitioning()->end();
                inv->getAnnualPartitioning()->next()
            ) {
            AnnualPartitioning *ap = inv->getAnnualPartitioning();
            m_out << TAGSTART(entry)
                << TAG(year,ap->getYear())
                << TAG(price,ap->getPrice().format())
                << TAG(vat,ap->getVat().format())
                << TAG(total,ap->getPriceWithVat().format())
                << TAGEND(entry);
        }
        m_out << TAGEND(years)
            << TAGEND(entry);
    }
    m_out << TAGEND(vat_rates)
        << TAGSTART(sumarize)
        << TAG(total,inv->getPrice().format());
    if (inv->getType() != IT_DEPOSIT) {
        m_out << TAG(paid, OUTMONEY(-inv->getPrice()));
    } else {
        m_out << TAG(paid, OUTMONEY(0));
    }
    m_out
        << TAG(to_be_paid,OUTMONEY(0))
        << TAGEND(sumarize)
        << TAGEND(delivery);
    if (inv->getSourceCount()) {
        m_out << TAGSTART(advance_payment)
            << TAGSTART(applied_invoices);
        for (unsigned k=0; k<inv->getSourceCount(); k++) {
            const PaymentSource *ps = inv->getSource(k);
            m_out << TAGSTART(consumed)
                << TAG(number,ps->getNumber())
                << TAG(price,ps->getPrice().format())
                << TAG(balance,ps->getCredit().format())
                << TAG(vat,ps->getVat().format())
                << TAG(vat_rate,ps->getVatRate())
                << TAG(pricevat,ps->getPriceWithVat().format())
                << TAG(total,ps->getTotalPrice().format())
                << TAG(total_vat,ps->getTotalVat().format())
                << TAG(total_with_vat,ps->getTotalPriceWithVat().format())
                << TAG(crtime,ps->getCrTime())
                << TAGEND(consumed);
        }
        m_out << TAGEND(applied_invoices)
            << TAGEND(advance_payment);
    }
    if (inv->getActionCount()) {
        m_out << TAGSTART(appendix)
            << TAGSTART(items);
        for (unsigned k=0; k<inv->getActionCount(); k++) {
            const PaymentAction *pa = inv->getAction(k);
            m_out << TAGSTART(item)
                << TAG(subject,pa->getObjectName())
                << TAG(code,
                        (pa->getAction() == PAT_CREATE_DOMAIN ?
                         "RREG" : "RUDR"))
                << TAG(timestamp,pa->getActionTime().iso_str());
            if (!pa->getExDate().is_special())
                m_out << TAG(expiration,pa->getExDate());
            m_out << TAG(count,pa->getUnitsCount()/12) // in years
                << TAG(price,pa->getPricePerUnit().format())
                << TAG(total,pa->getPrice().format())
                << TAG(vat_rate,pa->getVatRate())
                << TAGEND(item);
        }
        m_out << TAGEND(items)
            << TAGSTART(sumarize_items)
            << TAG(total,inv->getPrice().format())
            << TAGEND(sumarize_items)
            << TAGEND(appendix);
    }
    m_out << TAGEND(invoice);
} // void doExport(Invoice *)

std::string 
ExporterArchiver::makeFileName(Invoice *inv, const std::string &suffix)
{
    std::stringstream filename;
    filename << inv->getPrefix() << suffix;
    return filename.str();
}
void 
ExporterArchiver::doExport(Invoice *inv)
{
    TRACE("[CALL] Register::Invoicing::ExporterArchiver::doExport(Invoice *)");
    try {
        std::auto_ptr<Document::Generator>
            gPDF(m_docMan->createSavingGenerator(
                        inv->getType() == IT_DEPOSIT ?
                        Document::GT_ADVANCE_INVOICE_PDF :
                        Document::GT_INVOICE_PDF,
                        makeFileName(inv, ".pdf"),
                        INVOICE_PDF_FILE_TYPE,
                        inv->getClient()->getCountry() == "CZ" ?
                        "cs" : "en")
                );
        ExporterXML(gPDF->getInput(), true).doExport(inv);
        Database::ID filePDF = gPDF->closeInput();
        std::auto_ptr<Document::Generator>
            gXML(m_docMan->createSavingGenerator(
                        Document::GT_INVOICE_OUT_XML,
                        makeFileName(inv, ".xml"),
                        INVOICE_XML_FILE_TYPE,
                        inv->getClient()->getVatApply() ? "cs" : "en")
                );
        ExporterXML(gXML->getInput(), true).doExport(inv);
        Database::ID fileXML = gXML->closeInput();
        inv->setFile(filePDF, fileXML);
        inv->save();
    } catch (...) {
        LOGGER(PACKAGE).error(
                "Register::Invoicing::ExporterArchiver::doExport("
                "Invoice *): exception catched");
    }
} // ExporterArchiver::doExport()

} // namespace Invoicing
} // namespace Register

