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

    Database::SelectQuery object_info_query;
    object_info_query.select()
        << "t_1.id";
    object_info_query.from()
        << getTempTableName() << " tmp "
        << "JOIN bank_item t_1 ON (tmp.id = t_1.id)";
    object_info_query.order_by()
        << "tmp.id";
    Database::Connection conn = Database::Manager::acquire();
    try {
        fillTempTable(tmp_table_query);

        Database::Result res = conn.exec(object_info_query);
        for (int i = 0; i < (int)res.size(); i++) {
            unsigned long long id = res[i][0];
            StatementItem *item = new StatementItem();
            item->setId(id);
            item->reload();
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
        case IMT_EVID:
            stable_sort(m_data.begin(), m_data.end(), CompareAccountEvid(asc));
            break;
        case IMT_DATE:
            stable_sort(m_data.begin(), m_data.end(), CompareAccountDate(asc));
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

