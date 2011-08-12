#ifndef BANK_STATEMENT_LIST_IMPL_H_
#define BANK_STATEMENT_LIST_IMPL_H_


#include "bank_statement_impl.h"
#include "bank_statement_list.h"
#include "common_impl_new.h"
#include "db_settings.h"
#include "types/money.h"

namespace Fred {
namespace Banking {


class StatementListImpl : public Fred::CommonListImplNew,
                          public StatementList
{
private:
    bool partial_load_;

public:
    StatementListImpl() : CommonListImplNew(), partial_load_(false)
    {
    }

    ~StatementListImpl()
    {
        clear();
    }

    Statement* get(const unsigned int &index) const
    {
        try {
            Statement *statement =
                dynamic_cast<Statement*>(m_data.at(index));
            if (statement) {
                return statement;
            } else {
                throw std::exception();
            }
        } catch (...) {
            throw std::exception();
        }
    }

    Statement* getById(const unsigned long long &id) const
    {
        for (unsigned int i = 0; i < getSize(); i++) {
            if (get(i)->getId() == id) {
                return get(i);
            }
        }
        return NULL;
    }

    void reload(Database::Filters::Union &filter)
    {
        TRACE("[CALL] Fred::Banking::StatementListImpl::reload(Database::Filters::Union &)");
        clear();
        filter.clearQueries();

        bool at_least_one = false;
        Database::SelectQuery id_query;
        Database::Filters::Compound::iterator sit = filter.begin();
        for (; sit != filter.end(); ++sit) {
            Database::Filters::BankStatement *sf =
                dynamic_cast<Database::Filters::BankStatement *>(*sit);
            if (!sf) {
                continue;
            }
            Database::SelectQuery *tmp = new Database::SelectQuery();
            tmp->addSelect(new Database::Column(
                        "id", sf->joinBankStatementTable(), "DISTINCT"));
            filter.addQuery(tmp);
            at_least_one = true;
        }
        if (!at_least_one) {
            LOGGER(PACKAGE).error("wrong filter passed for reload!");
            return;
        }
        id_query.order_by() << "id DESC";
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
            << "t_1.balance_debet, "
            << "t_1.file_id";
        object_info_query.from()
            << getTempTableName() << " tmp "
            << "JOIN bank_statement t_1 ON (tmp.id = t_1.id)";
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
                Money balance     = std::string(*(++col));
                Money oldBalance  = std::string(*(++col));
                Money credit      = std::string(*(++col));
                Money debet       = std::string(*(++col));
                unsigned long long fileId   = *(++col);

                StatementImpl *statement = new StatementImpl();
                statement->setId(id);
                statement->setAccountId(accountId);
                statement->setNum(number);
                statement->setCreateDate(crDate);
                statement->setBalanceOldDate(oldDate);
                statement->setBalanceNew(balance);
                statement->setBalanceOld(oldBalance);
                statement->setBalanceCredit(credit);
                statement->setBalanceDebet(debet);
                statement->setFileId(fileId);
                appendToList(statement);
            }
            if (isEmpty()) {
                return;
            }

            CommonListImplNew::reload();

        } catch (Database::Exception &ex) {
            std::string message = ex.what();
            if(message.find(Database::Connection::getTimeoutString()) != std::string::npos) {
                LOGGER(PACKAGE).info("Statement timeout in request list.");
                clear();
                throw;
            } else {
                LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
                clear();
            }
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
    } // void ListImpl::reload(Database::Filters::Union &filter)


    void sort(MemberType member, bool asc)
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
            default:
                break;
        }
    }

    const char* getTempTableName() const
    {
        return "tmp_statement_filter_result";
    }


};


}
}


#endif /*BANK_STATEMENT_LIST_IMPL_H_*/

