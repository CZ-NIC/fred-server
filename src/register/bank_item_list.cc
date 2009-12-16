#include "bank_item_list.h"
#include "common_impl_new.h"
#include "db_settings.h"

namespace Register {
namespace Banking {

StatementItem *
ItemList::get(const unsigned int &index) const throw (std::exception)
{
    try {
        StatementItem *stat =
            dynamic_cast<StatementItem *>(m_data.at(index));
        if (stat) {
            return stat;
        } else {
            throw std::exception();
        }
    } catch (...) {
        throw std::exception();
    }
}

StatementItem *
ItemList::getById(const unsigned long long &id) const
{
    for (unsigned int i = 0; i < getSize(); i++) {
        if (get(i)->getId() == id) {
            return get(i);
        }
    }
    return NULL;
}

void
ItemList::reload(Database::Filters::Union &filter)
{
    TRACE("[CALL] Register::Banking::ItemList::reload(Database::Filters::Union &)");
    clear();
    filter.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator sit = filter.begin();
    for (; sit != filter.end(); ++sit) {
        Database::Filters::StatementItem *sif =
            dynamic_cast<Database::Filters::StatementItem *>(*sit);
        if (!sif) {
            continue;
        }
        Database::SelectQuery *tmp = new Database::SelectQuery();
        tmp->addSelect(new Database::Column(
                    "id", sif->joinStatementItemTable(), "DISTINCT"));
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
    
    Database::Connection conn = Database::Manager::acquire();
    try {
        fillTempTable(tmp_table_query);
        
        Database::SelectQuery object_info_query;
        object_info_query.select()
            << "t_1.id, t_1.statement_id, t_1.account_number, t_1.bank_code, "
            << "t_1.type, t_1.code, t_1.konstSym, t_1.varSymb, t_1.specsymb, t_1.price, "
            << "t_1.account_evid, t_1.account_date, t_1.account_memo, "
            << "t_1.invoice_id, t_1.account_name, t_1.crtime";          
        object_info_query.from()
            << getTempTableName() << " tmp "
            << "JOIN bank_item t_1 ON (tmp.id = t_1.id)";
        object_info_query.order_by()
            << "tmp.id";

        Database::Result r_stat = conn.exec(object_info_query);
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

            StatementItem *item = new StatementItem();                
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
            appendToList(item);
        }
        CommonListImplNew::reload();
    } catch (Database::Exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
    } catch (std::exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
    }
}

void
ItemList::sort(ItemMemberType member, bool asc)
{
    switch (member) {
        case IMT_ID:
            stable_sort(m_data.begin(), m_data.end(), CompareId(asc));
            break;
        case IMT_STATEMENT_ID:
            stable_sort(m_data.begin(), m_data.end(), CompareStatementId(asc));
            break;
        case IMT_ACCOUNT_NUMBER:
            stable_sort(m_data.begin(), m_data.end(), CompareAccountNumber(asc));
            break;
        case IMT_BANK_CODE:
            stable_sort(m_data.begin(), m_data.end(), CompareBankCodeId(asc));
            break;
        case IMT_TYPE:
            stable_sort(m_data.begin(), m_data.end(), CompareType(asc));
            break;
        case IMT_CODE:
            stable_sort(m_data.begin(), m_data.end(), CompareCode(asc));
            break;
        case IMT_CONSTSYMB:
            stable_sort(m_data.begin(), m_data.end(), CompareKonstSym(asc));
            break;
        case IMT_VARSYMB:
            stable_sort(m_data.begin(), m_data.end(), CompareVarSymb(asc));
            break;
        case IMT_SPECSYMB:
            stable_sort(m_data.begin(), m_data.end(), CompareSpecSymb(asc));
            break;
        case IMT_ACCOUNT_EVID:
            stable_sort(m_data.begin(), m_data.end(), CompareAccountEvid(asc));
            break;
        case IMT_ACCOUNT_DATE:
            stable_sort(m_data.begin(), m_data.end(), CompareAccountDate(asc));
            break;
        case IMT_PRICE:
            stable_sort(m_data.begin(), m_data.end(), ComparePrice(asc));
            break;
        case IMT_ACCOUNT_NAME:
            stable_sort(m_data.begin(), m_data.end(), CompareAccountName(asc));
            break;
        case IMT_CREATE_TIME:
            stable_sort(m_data.begin(), m_data.end(), CompareCrTime(asc));
            break;

    }
}

const char *
ItemList::getTempTableName() const
{
    return "tmp_statement_item_filter_result";
}

} // namespace Banking
} // namespace Register

