#include "invoice_list.h"
#include "invoice_manager.h"
#include "common_impl_new.h"

namespace Register {
namespace Invoicing {

Invoice *
List::get(unsigned int index) const throw (std::exception)
{
    try {
        Invoice *inv = 
            dynamic_cast<Invoice *>(CommonListImplNew::get(index));
        if (inv) {
            return inv;
        } else {
            throw std::exception();
        }
    } catch (...) {
        throw std::exception();
    }
}

Invoice *
List::getById(unsigned long long id) const
{
    for (unsigned int i = 0; i < getSize(); i++) {
        if (get(i)->getId() == id) {
            return get(i);
        }
    }
    return NULL;
}

void
List::sort(MemberType member, bool asc)
{
    LOGGER(PACKAGE).debug("Register::Invoicing::ListImpl::sort()");
    switch (member) {
        case MT_ZONE:
            stable_sort(m_data.begin(), m_data.end(), CompareZoneId(asc));
            break;
        case MT_CRTIME:
            stable_sort(m_data.begin(), m_data.end(), CompareCrDate(asc));
            break;

        case MT_TAXDATE:
            stable_sort(m_data.begin(), m_data.end(), CompareTaxDate(asc));
            break;
        case MT_TODATE:
            stable_sort(m_data.begin(), m_data.end(), CompareToDate(asc));
            break;
        case MT_FROMDATE:
            stable_sort(m_data.begin(), m_data.end(), CompareFromDate(asc));
            break;
        case MT_NUMBER:
            stable_sort(m_data.begin(), m_data.end(), ComparePrefix(asc));
            break;
        case MT_REGISTRAR:
            stable_sort(m_data.begin(), m_data.end(), CompareRegistrarId(asc));
            break;
        case MT_CREDIT:
            stable_sort(m_data.begin(), m_data.end(), CompareCredit(asc));
            break;
        case MT_PRICE:
            stable_sort(m_data.begin(), m_data.end(), ComparePrice(asc));
            break;
        case MT_VARSYMBOL:
            stable_sort(m_data.begin(), m_data.end(), CompareVarSymbol(asc));
            break;
        case MT_FILEPDF:
            stable_sort(m_data.begin(), m_data.end(), CompareFileId(asc));
            break;
        case MT_FILEXML:
            stable_sort(m_data.begin(), m_data.end(), CompareFileXmlId(asc));
            break;
        case MT_TOTAL:
            stable_sort(m_data.begin(), m_data.end(), CompareTotal(asc));
            break;
        case MT_TYPE:
            stable_sort(m_data.begin(), m_data.end(), CompareType(asc));
            break;
    }
}
void
List::reload(Database::Filters::Union &filter)
{
    TRACE("[CALL] Register::Invoicing::ListImpl::reload(Database::Filters::Union &)");
    clear();
    filter.clearQueries();
    Database::Connection conn = Database::Manager::acquire();

    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator iit = filter.begin();
    for (; iit != filter.end(); ++ iit) {
        Database::Filters::Invoice *invFilter =
            dynamic_cast<Database::Filters::Invoice *>(*iit);
        if (!invFilter) {
            continue;
        }
        Database::SelectQuery *tmp = new Database::SelectQuery();
        tmp->addSelect(new Database::Column(
                    "id", invFilter->joinInvoiceTable(), "DISTINCT"));
        filter.addQuery(tmp);
        at_least_one = true;
    }
    if (!at_least_one) {
        LOGGER(PACKAGE).error("wrong filter passed for reload!");
        return;
    }
    id_query.limit(getLimit());
    filter.serialize(id_query);
    Database::InsertQuery tmp_table_query =
        Database::InsertQuery(getTempTableName(), id_query);
    LOGGER(PACKAGE).debug(boost::format(
                "temporary table '%1%' generated sql = %2%")
            % getTempTableName() % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select()
        << "t_1.id, t_1.zone, t_2.fqdn, t_1.crdate, t_1.taxdate, "
        << "t_5.fromdate, t_5.todate, t_4.typ, t_1.prefix, "
        << "t_1.prefix_type, "
        << "t_1.registrarid, t_1.credit, t_1.price, "
        << "t_1.vat, t_1.total, t_1.totalvat, "
        << "t_1.file, t_1.fileXML, t_3.organization, t_3.street1, "
        << "t_3.city, t_3.postalcode, "
        << "TRIM(t_3.ico), TRIM(t_3.dic), TRIM(t_3.varsymb), "
        << "t_3.handle, t_3.vat, t_3.id, t_3.country, "
        << "t_6.name as file_name, t_7.name as filexml_name";
    object_info_query.from()
        << "tmp_invoice_filter_result tmp "
        << "JOIN invoice t_1 ON (tmp.id = t_1.id) "
        << "JOIN zone t_2 ON (t_1.zone = t_2.id) "
        << "JOIN registrar t_3 ON (t_1.registrarid = t_3.id) "
        << "JOIN invoice_prefix t_4 ON (t_4.id = t_1.prefix_type) "
        << "LEFT JOIN invoice_generation t_5 ON (t_1.id = t_5.invoiceid) "
        << "LEFT JOIN files t_6 ON (t_1.file = t_6.id) "
        << "LEFT JOIN files t_7 ON (t_1.filexml = t_7.id)";
    object_info_query.order_by()
        << "tmp.id";
    try {
        fillTempTable(tmp_table_query);
        Database::Result r_info = conn.exec(object_info_query);
        for (Database::Result::Iterator it = r_info.begin();
                it != r_info.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            Database::ID       id             = *col;
            Database::ID       zone           = *(++col);
            std::string        zoneName       = *(++col);
            Database::DateTime create_time    = *(++col);
            Database::Date     tax_date       = *(++col);
            Database::Date     from_date      = *(++col);
            Database::Date     to_date        = *(++col);
            Type               type           =
                (int)*(++col) == 0 ? IT_DEPOSIT : IT_ACCOUNT;
            unsigned long long number         = *(++col);
            int                prefix_type    = *(++col);
            Database::ID       registrar_id   = *(++col);
            Database::Money    credit         = *(++col);
            Database::Money    price          = *(++col);
            short              vat_rate       = *(++col);
            Database::Money    total          = *(++col);
            Database::Money    total_vat      = *(++col);
            Database::ID       filePDF        = *(++col);
            Database::ID       fileXML        = *(++col);
            std::string        c_organization = *(++col);
            std::string        c_street1      = *(++col);
            std::string        c_city         = *(++col);
            std::string        c_postal_code  = *(++col);
            std::string        c_ico          = *(++col);
            std::string        c_dic          = *(++col);
            std::string        c_var_symb     = *(++col);
            std::string        c_handle       = *(++col);
            bool               c_vat          = *(++col);
            unsigned long long c_id           = *(++col);
            std::string        c_country      = *(++col);
            std::string        filepdf_name   = *(++col);
            std::string        filexml_name   = *(++col);

            Database::DateInterval account_period(from_date, to_date);
            Subject client(c_id, c_handle, c_organization, "", c_street1,
                    c_city, c_postal_code, c_country, c_ico, c_dic,
                    "", "", "", "", "", "", c_vat);

            Invoice *invoice = new Invoice(m_manager);
            invoice->setId(id);
            invoice->setZoneId(zone);
            invoice->setCrDate(create_time);
            invoice->setTaxDate(tax_date);
            invoice->setAccountPeriod(account_period);
            invoice->setType(type);
            invoice->setPrefix(number);
            invoice->setRegistrarId(registrar_id);
            invoice->setCredit(credit);
            invoice->setPrice(price);
            invoice->setVat(vat_rate);
            invoice->setTotal(total);
            invoice->setTotalVat(total_vat);
            invoice->setVarSymbol(c_var_symb);
            invoice->setClient(client);
            invoice->setPrefixTypeId(prefix_type);
            appendToList(invoice);
            LOGGER(PACKAGE).debug(boost::format(
                        "list of invoices size: %1%")
                    % getSize());
        }
        if (isEmpty()) {
            return;
        }
        /* load details to each invoice */
        Database::SelectQuery source_query;
        source_query.select()
            << "tmp.id, ipm.credit, sri.vat, sri.prefix, "
            << "ipm.balance, sri.id, sri.total, "
            << "sri.totalvat, sri.crdate";
        source_query.from()
            << "tmp_invoice_filter_result tmp "
            << "JOIN invoice_credit_payment_map ipm ON (tmp.id = ipm.invoiceid) "
            << "JOIN invoice sri ON (ipm.ainvoiceid = sri.id) ";
        source_query.order_by()
            << "tmp.id";
        Database::Result r_sources = conn.exec(source_query);
        for (Database::Result::Iterator it = r_sources.begin();
                it != r_sources.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();
            unsigned long long invoiceId = *col;
            Invoice *invoice = getById(invoiceId);
            if (invoice) {
                Database::Money     price   = *(++col);
                unsigned int        vatRate = *(++col);
                unsigned long long  number  = *(++col);
                Database::Money     credit  = *(++col);
                Database::ID        id      = *(++col);
                Database::Money     totalPrice  = *(++col);
                Database::Money     totalVat    = *(++col);
                Database::DateTime  crTime  = *(++col);
                Database::Money     vat = 
                    m_manager->countVat(price, vatRate, true);
                PaymentSource *newSource = new PaymentSource();
                newSource->setPrice(price);
                newSource->setVatRate(vatRate);
                newSource->setVat(vat);
                newSource->setNumber(number);
                newSource->setCredit(credit);
                newSource->setId(id);
                newSource->setTotalPrice(totalPrice);
                newSource->setTotalVat(totalVat);
                newSource->setCrTime(crTime);
                invoice->addSource(newSource);
            }
        }
        /* append list of actions to all selected invoices
         * it handle situation when action come from source advance invoices
         * with different vat rates by grouping
         * this is ignored on partial load
         */
        if (!m_partialLoad) {
            Database::SelectQuery actionQuery;
            actionQuery.select()
                << "tmp.id, SUM(ipm.price), i.vat, o.name, "
                << "ior.crdate, ior.exdate, ior.operation, ior.period, "
                << "CASE "
                << "  WHEN ior.period = 0 THEN 0 "
                << "  ELSE SUM(ipm.price) * 12 / ior.period END, "
                << "o.id";
            actionQuery.from() << "tmp_invoice_filter_result tmp "
                << "JOIN invoice_object_registry ior ON (tmp.id = ior.invoiceid) "
                << "JOIN object_registry o ON (ior.objectid = o.id) "
                << "JOIN invoice_object_registry_price_map ipm ON (ior.id = ipm.id) "
                << "JOIN invoice i ON (ipm.invoiceid = i.id) ";
            actionQuery.group_by() 
                << "tmp.id, o.name, ior.crdate, ior.exdate, "
                << "ior.operation, ior.period, o.id, i.vat";
            actionQuery.order_by() 
                << "tmp.id";
            
            Database::Result r_actions = conn.exec(actionQuery);
            Database::Result::Iterator it = r_actions.begin();
            for (; it != r_actions.end(); ++it) {
                Database::Row::Iterator col = (*it).begin();
                Database::ID invoiceId = *col;
                // InvoiceImpl *invoicePtr =
                    // dynamic_cast<InvoiceImpl *>(findIDSequence(invoiceId));
                Invoice *invoice = getById(invoiceId);
                if (invoice) {
                    Database::Money     price       = *(++col);
                    unsigned int        vatRate     = *(++col);
                    std::string         objectName  = *(++col);
                    Database::DateTime  actionTime  = *(++col);
                    Database::Date      exDate      = *(++col);
                    PaymentActionType   type        = 
                        (int)*(++col) == 1 ? PAT_CREATE_DOMAIN : PAT_RENEW_DOMAIN;
                    unsigned int        units       = *(++col);
                    Database::Money     pricePerUnit= *(++col);
                    Database::ID        id          = *(++col);
                    Database::Money     vat         = m_manager->countVat(price, vatRate, true);
                    PaymentAction *action = new PaymentAction();
                    action->setPrice(price);
                    action->setVatRate(vatRate);
                    action->setVat(vat);
                    action->setObjectName(objectName);
                    action->setActionTime(actionTime);
                    action->setExDate(exDate);
                    action->setAction(type);
                    action->setUnitsCount(units);
                    action->setPricePerUnit(pricePerUnit);
                    action->setObjectId(id);
                    invoice->addAction(action);
                }
            }
        } //partialLoad
        CommonListImplNew::reload();
    } catch (Database::Exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
    } catch (std::exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
    }
}
const char *
List::getTempTableName() const
{
        return "tmp_invoice_filter_result";
}
void 
List::setPartialLoad(bool partialLoad)
{
    m_partialLoad = partialLoad;
}

void 
List::doExport(Exporter *exp)
{
    TRACE("[CALL] Register::Invoicing::List::doExport(Exporter *)");
    for (int i = 0; i < (int)m_data.size(); i++) {
        Invoice *invoice = dynamic_cast<Invoice *>(m_data[i]);
        if (invoice) {
            invoice->doExport(exp);
        }
    }
}

void 
List::exportXML(std::ostream &out)
{
    TRACE("[CALL] Register::Invoicing::List::exportXML(std::ostream &)");
    out << "<?xml version='1.0' encoding='utf-8'?>";
    ExporterXML xml(out, false);
    if (getSize() != 1) {
        out << TAGEND(list);
    }
    doExport(&xml);
    if (getSize() != 1) {
        out << TAGEND(list);
    }
}

} // namespace Invoicing
} // namespace Register

