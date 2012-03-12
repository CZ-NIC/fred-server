#ifndef BANK_PAYMENT_LIST_IMPL_H_
#define BANK_PAYMENT_LIST_IMPL_H_


#include "bank_payment_impl.h"
#include "bank_payment_list.h"
#include "common_impl_new.h"
#include "db_settings.h"
#include "types/money.h"

namespace Fred {
namespace Banking {


class PaymentListImpl : public Fred::CommonListImplNew,
                        public PaymentList
{
public:
    PaymentListImpl() : CommonListImplNew()
    {
    }

    ~PaymentListImpl()
    {
        clear();
    }

    Payment* get(const unsigned int &index) const
    {
        try {
            Payment *payment = dynamic_cast<Payment *>(m_data.at(index));
            if (payment) {
                return payment;
            } else {
                throw std::exception();
            }
        } catch (...) {
            throw std::exception();
        }
    }

    Payment* getById(const unsigned long long &id) const
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
        TRACE("[CALL] Fred::Banking::ItemList::reload(Database::Filters::Union &)");
        clear();
        filter.clearQueries();

        bool at_least_one = false;
        Database::SelectQuery id_query;
        Database::Filters::Compound::iterator sit = filter.begin();
        for (; sit != filter.end(); ++sit) {
            Database::Filters::BankPayment *sif =
                dynamic_cast<Database::Filters::BankPayment *>(*sit);
            if (!sif) {
                continue;
            }
            Database::SelectQuery *tmp = new Database::SelectQuery();
            tmp->addSelect(new Database::Column(
                        "id", sif->joinBankPaymentTable(), "DISTINCT"));
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

        Database::Connection conn = Database::Manager::acquire();
        try {
            fillTempTable(tmp_table_query);

            Database::SelectQuery object_info_query;
            object_info_query.select()
                << "t_1.id, t_1.statement_id, t_1.account_number, t_1.bank_code, "
                 "t_1.type, t_1.code, t_1.konstSym, t_1.varSymb, t_1.specsymb, t_1.price, "
                 "t_1.account_evid, t_1.account_date, t_1.account_memo, "
                 "t_2.id, t_1.account_name, t_1.crtime, "
                 "t_2.prefix, t_3.account_name || ' ' || t_3.account_number || '/' || t_3.bank_code";
            object_info_query.from()
                << getTempTableName()
                << " tmp "
                " JOIN bank_payment t_1 ON (tmp.id = t_1.id) "
                " LEFT JOIN ( "
                " bank_payment_registrar_credit_transaction_map t_6 "
                " JOIN invoice_registrar_credit_transaction_map t_7 ON t_6.registrar_credit_transaction_id = t_7.registrar_credit_transaction_id "
                " JOIN invoice t_2 ON (t_7.invoice_id = t_2.id) "
                " JOIN invoice_prefix t_5 ON (t_2.invoice_prefix_id = t_5.id and t_5.typ = 0) " //typ advance invoice
                " ) ON t_6.bank_payment_id = t_1.id "
                " JOIN bank_account t_3 ON (t_1.account_id = t_3.id) ";
            object_info_query.order_by()
                << "tmp.id DESC";

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
                Money price       = std::string(*(++col));
                std::string accountEvid     = *(++col);
                Database::Date accountDate  = *(++col);
                std::string accountMemo     = *(++col);
                Database::ID invoiceId      = *(++col);
                std::string accountName     = *(++col);
                Database::DateTime crTime   = *(++col);
                unsigned long long iprefix  = *(++col);
                std::string destAccount     = *(++col);

                PaymentImpl *payment = new PaymentImpl();
                payment->setId(id);
                payment->setStatementId(statementId);
                payment->setAccountNumber(accountNumber);
                payment->setBankCode(bankCode);
                payment->setType(type);
                payment->setCode(code);
                payment->setKonstSym(constSymb);
                payment->setVarSymb(varSymb);
                payment->setSpecSymb(specSymb);
                payment->setPrice(price);
                payment->setAccountEvid(accountEvid);
                payment->setAccountDate(accountDate);
                payment->setAccountMemo(accountMemo);
                payment->setAdvanceInvoiceId(invoiceId);
                payment->setAccountName(accountName);
                payment->setCrTime(crTime);
                payment->setInvoicePrefix(iprefix);
                payment->setDestAccount(destAccount);
                appendToList(payment);
            }
            CommonListImplNew::reload();
        } catch (Database::Exception &ex) {
            std::string message = ex.what();
            if (message.find(Database::Connection::getTimeoutString())
                    != std::string::npos) {
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
    }

    void sort(ItemMemberType member, bool asc)
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
                stable_sort(m_data.begin(), m_data.end(), CompareBankCode(asc));
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
            case IMT_ACCOUNT_MEMO:
                stable_sort(m_data.begin(), m_data.end(), CompareAccountMemo(asc));
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
            case IMT_DEST_ACCOUNT:
                stable_sort(m_data.begin(), m_data.end(), CompareDestAccount(asc));
                break;
            default:
            	throw std::runtime_error("unimplemented sort");
            	break;
        }
    }

    const char * getTempTableName() const
    {
        return "tmp_statement_item_filter_result";
    }

};



}
}


#endif /*BANK_PAYMENT_LIST_IMPL_H_*/

