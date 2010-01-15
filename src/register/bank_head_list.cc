#include "bank_head_list.h"
#include "common_impl_new.h"
#include "db_settings.h"

namespace Register {
namespace Banking {

StatementHead *
HeadList::get(const unsigned int &index) const throw (std::exception)
{
    try {
        StatementHead *stat =
            dynamic_cast<StatementHead *>(m_data.at(index));
        if (stat) {
            return stat;
        } else {
            throw std::exception();
        }
    } catch (...) {
        throw std::exception();
    }
}

StatementHead *
HeadList::getById(const unsigned long long &id) const
{
    for (unsigned int i = 0; i < getSize(); i++) {
        if (get(i)->getId() == id) {
            return get(i);
        }
    }
    return NULL;
}

void 
HeadList::reload(Database::Filters::Union &filter)
{
    TRACE("[CALL] Register::Banking::StatementListImpl::reload(Database::Filters::Union &)");
    clear();
    filter.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator sit = filter.begin();
    for (; sit != filter.end(); ++sit) {
        Database::Filters::StatementHead *sf =
            dynamic_cast<Database::Filters::StatementHead *>(*sit);
        if (!sf) {
            continue;
        }
        Database::SelectQuery *tmp = new Database::SelectQuery();
        tmp->addSelect(new Database::Column(
                    "id", sf->joinStatementHeadTable(), "DISTINCT"));
        tmp->order_by() << sf->joinStatementHeadTable().getAlias() + ".id DESC";
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
        << "t_1.id, t_1.account_id, t_1.num, t_1.create_date, "
        << "t_1.balance_old_date, t_1.balance_old, "
        << "t_1.balance_new, t_1.balance_credit, "
        << "t_1.balance_debet";
    if(!partialLoad) {
        object_info_query.select() << ", t_1.file_id";
    }
    object_info_query.from()
        << getTempTableName() << " tmp "
        << "JOIN bank_head t_1 ON (tmp.id = t_1.id)";
    object_info_query.order_by()
        << "tmp.id";
    Database::Connection conn = Database::Manager::acquire();
    try {
        fillTempTable(tmp_table_query);

        Database::Result r_info = conn.exec(object_info_query);
        Database::Result::Iterator it = r_info.begin();
        for (; it != r_info.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            unsigned long long id       = *col;
            unsigned long long accountId = *(++col);
            int number                  = *(++col);
            Database::Date crDate       = *(++col);
            Database::Date oldDate      = *(++col);
            Database::Money balance     = *(++col);
            Database::Money oldBalance  = *(++col);
            Database::Money credit      = *(++col);
            Database::Money debet       = *(++col);
            if(!partialLoad) {
                unsigned long long fileId   = *(++col);
            }

            StatementHead *stat = new StatementHead();
            stat->setId(id);
            stat->reload();
            appendToList(stat);
        }
        if (isEmpty()) {
            return;
        }

        if(!partialLoad) {
            Database::SelectQuery StatementItemQuery;
            StatementItemQuery.select()
                << "t_1.id, t_1.statement_id, t_1.account_number, t_1.bank_code, "
                << "t_1.type, t_1.code, t_1.konstSym, t_1.varSymb, t_1.specsymb, t_1.price, "
                << "t_1.account_evid, t_1.account_date, t_1.account_memo, "
                << "t_1.invoice_id, t_1.account_name, t_1.crtime";

            StatementItemQuery.from()
                << getTempTableName() << " tmp "
                << "JOIN bank_item t_1 ON (tmp.id = t_1.statement_id)";
            StatementItemQuery.order_by()
                << "tmp.id";
            Database::Result r_stat = conn.exec(StatementItemQuery);
            Database::Result::Iterator statIt = r_stat.begin();
            for (; statIt != r_stat.end(); ++statIt) {
                Database::Row::Iterator col = (*statIt).begin();
                Database::ID id             = *col;
                Database::ID statementId    = *(++col);
                std::string accountNumber   = *(++col);
                std::string bankCode        = *(++col);
                int type                    = *(++col);
                int code                    = *(++col);
                std::string constSymb       = *(++col);
                std::string varSymb         = *(++col);
                std::string specSymb        = *(++col);
                Database::Money price       = *(++col);
                std::string accountEvid     = *(++col);
                Database::Date accountDate  = *(++col);
                std::string accountMemo     = *(++col);
                Database::ID invoiceId      = *(++col);
                std::string accountName     = *(++col);
                Database::Date crTime       = *(++col);

                StatementHead *head = getById(statementId);
                if (head) {
                    StatementItem *item = head->newStatementItem();
                    item->setId(id);
                    item->setStatementId(statementId);
                    item->setAccountNumber(accountNumber);
                    item->setBankCodeId(bankCode);
                    item->setType(type);
                    item->setCode(code);
                    item->setKonstSym(constSymb);
                    item->setVarSymb(varSymb);
                    item->setSpecSymb(specSymb);
                    item->setPrice(price);
                    item->setAccountEvid(accountEvid);
                    item->setAccountDate(accountDate);
                    item->setAccountMemo(accountMemo);
                    item->setInvoiceId(invoiceId);
                    item->setAccountName(accountName);
                    item->setCrTime(crTime);
                }
            }
            CommonListImplNew::reload();
        }
    } catch (Database::Exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
    } catch (std::exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
    }
} // void ListImpl::reload(Database::Filters::Union &filter)


void
HeadList::sort(MemberType member, bool asc)
{
    switch (member) {
        case MT_ACCOUNT_ID:
            stable_sort(m_data.begin(), m_data.end(), CompareCreateDate(asc));
            break;
        case MT_NUM:
            stable_sort(m_data.begin(), m_data.end(), CompareCreateDate(asc));
            break;
        case MT_CREATE_DATE:
            stable_sort(m_data.begin(), m_data.end(), CompareCreateDate(asc));
            break;
        case MT_BALANCE_OLD_DATE:
            stable_sort(m_data.begin(), m_data.end(), CompareBalanceOldDate(asc));
            break;
        case MT_BALANCE_OLD:
            stable_sort(m_data.begin(), m_data.end(), CompareBalanceOld(asc));
            break;
        case MT_BALANCE_NEW:
            stable_sort(m_data.begin(), m_data.end(), CompareBalanceNew(asc));
            break;
        case MT_BALANCE_CREDIT:
            stable_sort(m_data.begin(), m_data.end(), CompareBalanceCredit(asc));
            break;
        case MT_BALANCE_DEBET:
            stable_sort(m_data.begin(), m_data.end(), CompareBalanceDebet(asc));
            break;
#if 0
        case MT_ZONE_FQDN:
            stable_sort(m_data.begin(), m_data.end(), CompareZoneFqdn(asc));
            break;
        case MT_FILE_ID:
            stable_sort(m_data.begin(), m_data.end(), CompareFileId(asc));
            break;
        case MT_ACCOUNT_NUMBER:
            stable_sort(m_data.begin(), m_data.end(), CompareAccountNumber(asc));
            break;
        case MT_BANK_CODE:
            stable_sort(m_data.begin(), m_data.end(), CompareBankCode(asc));
            break;
#endif
    }
}

const char *
HeadList::getTempTableName() const
{
    return "tmp_statement_filter_result";
}

void
HeadList::doExport(Exporter *exp)
{
    TRACE("[CALL] Register::Invoicing::List::doExport(Exporter *)");
    for (int i = 0; i < (int)m_data.size(); i++) {
        StatementHead *head = dynamic_cast<StatementHead *>(m_data[i]);
        if (head) {
            head->doExport(exp);
        }
    }
}

bool 
HeadList::exportXML(std::ostream &out)
{
    TRACE("Register::Banking::StatementImpl::exportXML(std::ostrem &)");
    ExporterXML xml(out);
    doExport(&xml);
    return true;
} // void exportXML(std::ostream &out)

} // namespace Banking
} // namespace Register
