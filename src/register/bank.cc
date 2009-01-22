/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "bank.h"
#include "log/logger.h"

namespace Register {
namespace Banking {

#define TAGSTART(tag) "<"#tag">"
#define TAGEND(tag) "</"#tag">"
#define TAG(tag,f) TAGSTART(tag) << f << TAGEND(tag)

#define transformString(str)    \
    ((str.empty()) ? "NULL" : "'" + str + "'")
#define transformId(id)         \
    ((id.to_string().compare("0") == 0) ? "NULL" : id.to_string())

class PaymentImpl:
    virtual public Payment {
private:
    std::string     m_accountNumber;
    std::string     m_bankCode;
    std::string     m_constSymb;
    std::string     m_varSymb;
    Database::Money m_price;
    std::string     m_accountMemo;
    Database::ID    m_invoiceId;
public:
    PaymentImpl(const std::string accountNumber,
            const std::string bankCode, const std::string constSymb,
            const std::string varSymb, const Database::Money price,
            const std::string accountMemo, const Database::ID invoiceId):
        m_accountNumber(accountNumber), m_bankCode(bankCode),
        m_constSymb(constSymb), m_varSymb(varSymb), m_price(price),
        m_accountMemo(accountMemo), m_invoiceId(invoiceId)
    { }
    PaymentImpl():
        m_accountNumber(), m_bankCode(), m_constSymb(), m_varSymb(),
        m_price(), m_accountMemo(), m_invoiceId()
    { }
    virtual const std::string &getAccountNumber() const
    {
        return m_accountNumber;
    }
    virtual const std::string &getBankCode() const
    {
        return m_bankCode;
    }
    virtual const std::string &getConstSymbol() const 
    {
        return m_constSymb;
    }
    virtual const std::string &getVarSymbol() const 
    {
        return m_varSymb;
    }
    virtual const Database::Money &getPrice() const 
    {
        return m_price;
    }
    virtual const std::string &getMemo() const 
    {
        return m_accountMemo;
    }
    virtual const Database::ID &getInvoiceId() const 
    {
        return m_invoiceId;
    }
    virtual void setAccountNumber(std::string accountNumber) 
    {
        m_accountNumber = accountNumber;
    }
    virtual void setBankCode(std::string bankCode)
    {
        m_bankCode = bankCode;
    }
    virtual void setConstSymbol(std::string constSymbol) 
    {
        m_constSymb = constSymbol;
    }
    virtual void setVarSymbol(std::string varSymbol) 
    {
        m_varSymb = varSymbol;
    }
    virtual void setPrice(Database::Money price) 
    {
        m_price = price;
    }
    virtual void setMemo(std::string memo) 
    {
        m_accountMemo = memo;
    }
    virtual void setInvoiceId(Database::ID invoiceId) 
    {
        m_invoiceId = invoiceId;
    }
}; // class PaymentImpl

class OnlineStatementImpl:
    public Register::CommonObjectImpl,
    virtual public OnlineStatement,
    virtual public PaymentImpl {
private:
    Database::ID m_accountId;
    Database::DateTime m_crDate;
    std::string m_accountName;
    std::string m_ident;
    Database::Connection *m_conn;
public:
    OnlineStatementImpl(const Database::ID id, const Database::ID accountId,
            const Database::Money price, const Database::DateTime crDate,
            const std::string accountNumber, const std::string bankCode,
            const std::string constSymb, const std::string varSymb,
            const std::string memo, const std::string accountName,
            const std::string ident, Database::ID invoiceId):
        PaymentImpl(accountNumber, bankCode, constSymb, varSymb,
                price, memo, invoiceId),
        CommonObjectImpl(id),
        m_accountId(accountId), m_crDate(crDate), m_accountName(accountName),
        m_ident(ident), m_conn(NULL)
    { }
    OnlineStatementImpl(Database::Connection *conn):
        PaymentImpl(),
        CommonObjectImpl(),
        m_accountId(), m_crDate(), m_accountName(), m_ident(), m_conn(conn)
    { }
    virtual const Database::ID &getAccountId() const 
    {
        return m_accountId;
    }
    virtual const Database::DateTime &getCrDate() const 
    {
        return m_crDate;
    }
    virtual const std::string &getAccountName() const 
    {
        return m_accountName;
    }
    virtual const std::string &getIdent() const 
    {
        return m_ident;
    }
    virtual void setAccountId(Database::ID accountId) 
    {
        m_accountId = accountId;
    }
    virtual void setCrDate(Database::DateTime crDate) 
    {
        m_crDate = crDate;
    }
    virtual void setCrDate(std::string crDate)
    {
        m_crDate = Database::DateTime(crDate);
    }
    virtual void setAccountName(std::string accountName) 
    {
        m_accountName = accountName;
    }
    virtual void setIdent(std::string ident) 
    {
        m_ident = ident;
    }
    virtual void setConn(Database::Connection *conn)
    {
        m_conn = conn;
    }
    virtual Database::Connection *getConn() const
    {
        return m_conn;
    }
    void update()
    {
        TRACE("[CALL] Register::Banking::OnlineStatement::update()");
        std::auto_ptr<Database::Query> updateStat(new Database::Query());
        updateStat->buffer()
            << "UPDATE bank_ebanka_list SET"
            << " account_id = " << transformId(getAccountId())
            << ", price = " << getPrice().format() 
            << ", crdate = '" << getCrDate() << "'"
            << ", account_number = " << transformString(getAccountNumber())
            << ", bank_code = " << transformString(getBankCode())
            << ", konstsym = " << transformString(getConstSymbol())
            << ", varsymb = " << transformString(getVarSymbol())
            << ", memo = " << transformString(getMemo())
            << ", name = " << transformString(getAccountName())
            << ", ident = " << transformString(getIdent())
            << ", invoice_id = " << transformId(getInvoiceId())
            << " WHERE id = " << id_;
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(*updateStat);
            transaction.commit();
            LOGGER(PACKAGE).info(boost::format(
                        "online payment item id='%1%' updated successfully")
                    % id_);
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        }
    } // void update()
    void insert()
    {
        TRACE("[CALL] Register::Banking::OnlineStatement::insert()");
        Database::InsertQuery insertStat("bank_ebanka_list");
        insertStat.add("account_id", getAccountId());
        insertStat.add("price", getPrice());
        insertStat.add("crdate", getCrDate());
        if (!getAccountNumber().empty()) {
            insertStat.add("account_number", getAccountNumber());
        }
        if (!getBankCode().empty()) {
            insertStat.add("bank_code", getBankCode());
        }
        if (!getConstSymbol().empty()) {
            insertStat.add("konstsym", getConstSymbol());
        }
        if (!getVarSymbol().empty()) {
            insertStat.add("varsymb", getVarSymbol());
        }
        if (!getMemo().empty()) {
            insertStat.add("memo", getMemo());
        }
        if (!getAccountName().empty()) {
            insertStat.add("name", getAccountName());
        }
        if (!getIdent().empty()) {
            insertStat.add("ident", getIdent());
        }
        insertStat.add("invoice_id", getInvoiceId());
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(insertStat);
            transaction.commit();
            Database::Sequence seq(*m_conn, "bank_ebanka_list_id_seq");
            id_ = seq.getCurrent();
            LOGGER(PACKAGE).info(boost::format(
                        "online statement item id='%1%' created successfully")
                    % id_);
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        }
    } // void insert()
    virtual void save() 
    {
        TRACE("[CALL] Register::Banking::OnlineStatement::save()");
        if (id_) {
            update();
        } else {
            insert();
        }
    }
}; // class OnlineStatementImpl

class StatementItemImpl:
    virtual public PaymentImpl, 
    virtual public StatementItem {
private:
    Database::ID    m_id;
    Database::ID    m_statementId;
    int             m_code;
    std::string     m_specSymb;
    std::string     m_accountEvid;
    Database::Date  m_accountDate;
protected:
    Database::Connection *m_conn;
public:
    StatementItemImpl(const Database::ID id, const Database::ID statementId,
            const std::string accountNumber, const std::string bankCode,
            const int code, const std::string constSymb,
            const std::string varSymb, const std::string specSymb,
            const Database::Money price,
            const std::string accountEvid, const Database::Date accountDate,
            const std::string accountMemo, const Database::ID invoiceId,
            Database::Connection *conn = NULL):
        PaymentImpl(accountNumber, bankCode, constSymb, varSymb,
                price, accountMemo, invoiceId),
        m_id(id), m_statementId(statementId), m_code(code), m_specSymb(specSymb),
        m_accountEvid(accountEvid), m_accountDate(accountDate), m_conn(conn)
    { }
    StatementItemImpl(Database::Connection *conn):
        PaymentImpl(),
        m_id(), m_statementId(), m_code(), m_specSymb(), m_accountEvid(),
        m_accountDate(), m_conn(conn)
    { }
    virtual const Database::ID &getId() const
    {
        return m_id;
    }
    virtual const Database::ID &getStatementId() const
    {
        return m_statementId;
    }
    virtual const int getCode() const 
    {
        return m_code;
    }
    virtual const std::string &getSpecSymbol() const
    {
        return m_specSymb;
    }
    virtual const std::string &getEvidenceNumber() const 
    {
        return m_accountEvid;
    }
    virtual const Database::Date &getDate() const 
    {
        return m_accountDate;
    }
    virtual void setId(Database::ID id) 
    {
        m_id = id;
    }
    virtual void setStatementId(Database::ID statementId) 
    {
        m_statementId = statementId;
    }
    virtual void setCode(int code) 
    {
        m_code = code;
    }
    virtual void setEvidenceNumber(std::string evidenceNumber) 
    {
        m_accountEvid = evidenceNumber;
    }
    virtual void setDate(Database::Date date) 
    {
        m_accountDate = date;
    }
    virtual void setDate(std::string date)
    {
        m_accountDate = Database::Date(date);
    }
    virtual void setSpecSymbol(std::string specSymbol) 
    {
        m_specSymb = specSymbol;
    }
    virtual void setConn(Database::Connection *conn)
    {
        m_conn = conn;
    }
    virtual Database::Connection *getConn() const
    {
        return m_conn;
    }
    void update()
    {
        TRACE("[CALL] Register::Banking::StatementItemImpl::update()");
        std::auto_ptr<Database::Query> update (new Database::Query());
        update->buffer()
            << "UPDATE bank_statement_item SET"
            << " statement_id = " << transformId(getStatementId())
            << ", account_number = " << transformString(getAccountNumber())
            << ", bank_code = " << transformString(getBankCode())
            << ", code = " << getCode()
            << ", konstsym = " << transformString(getConstSymbol())
            << ", varsymb = " << transformString(getVarSymbol())
            << ", price = " << getPrice().format()
            << ", account_evid = " << transformString(getEvidenceNumber())
            << ", account_date = '" << getDate() << "'"
            << ", account_memo = " << transformString(getMemo())
            << ", invoice_id = " << transformId(getInvoiceId())
            << " WHERE id = " << m_id;
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(*update);
            transaction.commit();
            LOGGER(PACKAGE).info(boost::format(
                        "statement item id='%1%' updated successfully")
                    % m_id);
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        }
    } // void update
    void insert()
    {
        TRACE("[CALL] Register::Banking::StatementItemImpl::insert()");
        Database::InsertQuery insertItem("bank_statement_item");
        insertItem.add("statement_id", getStatementId());
        if (!getAccountNumber().empty()) {
            insertItem.add("account_number", getAccountNumber());
        }
        if (!getBankCode().empty()) {
            insertItem.add("bank_code", getBankCode());
        }
        insertItem.add("code", getCode());
        if (!getConstSymbol().empty()) {
            insertItem.add("konstsym", getConstSymbol());
        }
        if (!getVarSymbol().empty()) {
            insertItem.add("varsymb", getVarSymbol());
        }
        insertItem.add("price", getPrice());
        if (!getEvidenceNumber().empty()) {
            insertItem.add("account_evid", getEvidenceNumber());
        }
        insertItem.add("account_date", getDate());
        if (!getMemo().empty()) {
            insertItem.add("account_memo", getMemo());
        }
        insertItem.add("invoice_id", getInvoiceId());
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(insertItem);
            transaction.commit();
            Database::Sequence seq(*m_conn, "bank_statement_item_id_seq");
            m_id = seq.getCurrent();
            LOGGER(PACKAGE).info(boost::format(
                        "statement item id='%1%' created successfully")
                    % m_id);
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        }
    } // void insert
    void save()
    {
        TRACE("[CALL] Register::Banking::StatementItemImpl::save()");
        if (m_id) {
            update();
        } else {
            insert();
        }
    }
}; // class StatementItemImpl

class StatementImpl:
    public Register::CommonObjectImpl,
    virtual public Statement {
private:
    Database::ID    m_accountId;
    int             m_number;
    Database::Date  m_createDate;
    Database::Date  m_balanceOldDate;
    Database::Money m_balanceNew;
    Database::Money m_balanceOld;
    Database::Money m_balanceCredit;
    Database::Money m_balanceDebet;

    typedef std::vector<StatementItemImpl>  StatementItemListType;
    StatementItemListType   m_statementItems;
protected:
    Database::Connection *m_conn;
public:
    StatementImpl(Database::ID id, Database::ID accountId, int number,
            Database::Date &balanceNewDate, Database::Date &balanceOldDate,
            Database::Money &balanceNew, Database::Money &balanceOld,
            Database::Money &balanceCredit, Database::Money &balanceDebet):
        CommonObjectImpl(id), m_accountId(accountId), m_number(number),
        m_createDate(balanceNewDate), m_balanceOldDate(balanceOldDate),
        m_balanceNew(balanceNew), m_balanceOld(balanceOld),
        m_balanceCredit(balanceCredit), m_balanceDebet(balanceDebet)
    { }
    StatementImpl(Database::Connection *conn):
        CommonObjectImpl(),
        m_accountId(), m_number(), m_createDate(), m_balanceOldDate(),
        m_balanceNew(), m_balanceOld(), m_balanceCredit(), m_balanceDebet(),
        m_conn(conn)
    { }
    virtual const Database::ID &getAccountId() const
    {
        return m_accountId;
    }
    virtual const int getNumber() const
    {
        return m_number;
    }
    virtual const Database::Date &getDate() const
    {
        return m_createDate;
    }
    virtual const Database::Date &getOldDate() const
    {
        return m_balanceOldDate;
    }
    virtual const Database::Money &getBalance() const
    {
        return m_balanceNew;
    }
    virtual const Database::Money &getOldBalance() const
    {
        return m_balanceOld;
    }
    virtual const Database::Money &getCredit() const
    {
        return m_balanceCredit;
    }
    virtual const Database::Money &getDebet() const
    {
        return m_balanceDebet;
    }
    virtual unsigned int getStatementItemCount() const 
    {
        return m_statementItems.size();
    }
    virtual const StatementItem *getStatementItemByIdx(unsigned int idx) const
        throw (NOT_FOUND)
    {
        if (idx >= getStatementItemCount()) {
           throw NOT_FOUND();
        }
        return &m_statementItems[idx];
    }
    StatementItemImpl *addStatementItem(const StatementItemImpl &statementItem)
    {
        m_statementItems.push_back(statementItem);
        return &m_statementItems.at(m_statementItems.size() - 1);
    }
    virtual void setAccountId(Database::ID accountId) 
    {
        m_accountId = accountId;
    }
    virtual void setNumber(int number) 
    {
        m_number = number;
    }
    virtual void setDate(Database::Date date)
    {
        m_createDate = date;
    }
    virtual void setDate(std::string date)
    {
        m_createDate = Database::Date(date);
    }
    virtual void setOldDate(Database::Date oldDate) 
    {
        m_balanceOldDate = oldDate;
    }
    virtual void setOldDate(std::string oldDate)
    {
        m_balanceOldDate = Database::Date(oldDate);
    }
    virtual void setBalance(Database::Money balance) 
    {
        m_balanceNew = balance;
    }
    virtual void setOldBalance(Database::Money oldBalance) 
    {
        m_balanceOld = oldBalance;
    }
    virtual void setCredit(Database::Money credit) 
    {
        m_balanceCredit = credit;
    }
    virtual void setDebet(Database::Money debet) 
    {
        m_balanceDebet = debet;
    }
    virtual void setConn(Database::Connection *conn)
    {
        m_conn = conn;
    }
    virtual Database::Connection *getConn() const
    {
        return m_conn;
    }
    void update()
    {
        TRACE("[CALL] Register::Banking::StatementImpl::update()");
        std::auto_ptr<Database::Query> update(new Database::Query());
        update->buffer()
            << "UPDATE bank_statement_head SET"
            << " account_id = " << transformId(getAccountId())
            << ", num = " << getNumber()
            << ", create_date = '" << getDate() << "'"
            << ", balance_old_date = '" << getOldDate() << "'"
            << ", balance_old = " << getOldBalance().format()
            << ", balance_new = " << getBalance().format()
            << ", balance_credit = " << getCredit().format()
            << ", balance_debet = " << getDebet().format()
            << " WHERE id = " << id_;
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(*update);
            transaction.commit();
            LOGGER(PACKAGE).info(boost::format(
                        "online payment item id='%1%' updated successfully")
                    % id_);
            for (int i = 0; i < (int)getStatementItemCount(); i++) {
                StatementItem *item = (StatementItem *)getStatementItemByIdx(i);
                item->setStatementId(id_);
                item->setConn(getConn());
                item->save();
            }
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        }
    } // void update
    void insert()
    {
        TRACE("[CALL] Register::Banking::StatementImpl::insert()");
        Database::InsertQuery insertHead("bank_statement_head");
        insertHead.add("account_id", getAccountId());
        insertHead.add("num", getNumber());
        insertHead.add("create_date", getDate());
        insertHead.add("balance_old_date", getOldDate());
        insertHead.add("balance_old", getOldBalance());
        insertHead.add("balance_new", getBalance());
        insertHead.add("balance_credit", getCredit());
        insertHead.add("balance_debet", getDebet());
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(insertHead);
            transaction.commit();
            Database::Sequence seq(*m_conn, "bank_statement_head_id_seq");
            id_ = seq.getCurrent();
            LOGGER(PACKAGE).info(boost::format(
                        "statement head id='%1%' created successfully")
                    % id_);
            for (int i = 0; i < (int)getStatementItemCount(); i++) {
                StatementItem *item = (StatementItem *)getStatementItemByIdx(i);
                item->setStatementId(id_);
                item->save();
            }
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            throw;
        }
    } // void insert
    virtual void save()
    {
        TRACE("[CALL] Register::Banking::StatementImpl::save()");
        if (id_) {
            update();
        } else {
            insert();
        }
    }
    virtual StatementItem *createStatementItem()
    {
        return addStatementItem(StatementItemImpl(m_conn));
    }
}; // class StatementImpl

COMPARE_CLASS_IMPL(PaymentImpl, AccountNumber);
COMPARE_CLASS_IMPL(PaymentImpl, BankCode)
COMPARE_CLASS_IMPL(PaymentImpl, Price)
COMPARE_CLASS_IMPL(PaymentImpl, InvoiceId)

class OnlineListImpl:
    public Register::CommonListImpl,
    virtual public OnlineList {
private:
    Manager *m_manager;
public:
    OnlineListImpl(Database::Connection *conn, Manager *manager):
        CommonListImpl(conn),
        m_manager(manager)
    { }

    virtual OnlineStatement *get(unsigned int index) const
    {
        try {
            OnlineStatement *stat =
                dynamic_cast<OnlineStatement *>(data_.at(index));
            if (stat) {
                return stat;
            } else {
                throw std::exception();
            }
        } catch (...) {
            throw std::exception();
        }
    }

    virtual void reload(Database::Filters::Union &filter)
    {
        TRACE("[CALL] Register::Banking::OnlineListImpl::reload(Database::Filters::Union &)");
        clear();
        filter.clearQueries();

        bool at_least_one = false;
        Database::SelectQuery id_query;
        Database::Filters::Compound::iterator osit = filter.begin();
        for (; osit != filter.end(); ++osit) {
            Database::Filters::OnlineStatement *statFilter =
                dynamic_cast<Database::Filters::OnlineStatement *>(*osit);
            if (!statFilter) {
                continue;
            }
            Database::SelectQuery *tmp = new Database::SelectQuery();
            tmp->addSelect(new Database::Column(
                        "id", statFilter->joinOnlineStatementTable(), "DISTINCT"));
            filter.addQuery(tmp);
            at_least_one = true;
        }
        if (!at_least_one) {
            LOGGER(PACKAGE).error("wrong filter passed for reload!");
            return;
        }
        id_query.limit(load_limit_);
        filter.serialize(id_query);
        Database::InsertQuery tmp_table_query =
            Database::InsertQuery(getTempTableName(), id_query);

        LOGGER(PACKAGE).debug(boost::format(
                    "temporary table '%1%' generated sql = %2%")
                % getTempTableName() % tmp_table_query.str());
        Database::SelectQuery object_info_query;
        object_info_query.select()
            << "t_1.id, t_1.account_id, t_1.price * 100, t_1.crdate, "
            << "t_1.account_number, t_1.bank_code, t_1.konstsym, "
            << "t_1.varsymb, t_1.memo, t_1.name, t_1.ident, "
            << "t_1.invoice_id";
        object_info_query.from()
            << getTempTableName() << " tmp "
            << "JOIN bank_ebanka_list t_1 ON (tmp.id = t_1.id)";
        object_info_query.order_by()
            << "tmp.id";
        try {
            fillTempTable(tmp_table_query);

            Database::Result r_info = conn_->exec(object_info_query);
            Database::Result::Iterator it = r_info.begin();
            for (; it != r_info.end(); ++it) {
                Database::Row::Iterator col = (*it).begin();

                Database::ID id             = *col;
                Database::ID accountId      = *(++col);
                Database::Money price       = *(++col);
                Database::DateTime crDate   = *(++col);
                std::string accountNumber   = *(++col);
                std::string bankCode        = *(++col);
                std::string constSymbol     = *(++col);
                std::string varSymbol       = *(++col);
                std::string memo            = *(++col);
                std::string name            = *(++col);
                std::string ident           = *(++col);
                Database::ID invoiceId      = *(++col);

                OnlineStatementImpl *stat = new OnlineStatementImpl(id, accountId, price,
                        crDate, accountNumber, bankCode, constSymbol,
                        varSymbol, memo, name, ident, invoiceId);
                stat->setConn(conn_);
                data_.push_back(stat);
            }
            CommonListImpl::reload();
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
    } // void reload(Database::Filters::Union &filter)

    virtual void sort(MemberType member, bool asc)
    {
        switch (member) {
            case MT_ACCOUNT_NUMBER:
                stable_sort(data_.begin(), data_.end(), CompareAccountNumber(asc));
                break;
            case MT_BANK_CODE:
                stable_sort(data_.begin(), data_.end(), CompareBankCode(asc));
                break;
            case MT_PRICE:
                stable_sort(data_.begin(), data_.end(), ComparePrice(asc));
                break;
            case MT_INVOICE_ID:
                stable_sort(data_.begin(), data_.end(), CompareInvoiceId(asc));
                break;
        }
    }

    virtual const char *getTempTableName() const 
    {
        return "tmp_online_statement_filter_result";
    }

    virtual void makeQuery(bool, bool, std::stringstream &) const
    { }

    virtual void reload()
    { }

    virtual void exportXML(std::ostream &out)
    {
        out << TAGSTART(online_payments);
        for (int i = 0; i < (int)getCount(); i++) {
            OnlineStatement *stat = get(i);
            out << TAGSTART(online_payment)
                << TAG(id, stat->getId())
                << TAG(accout_number, stat->getAccountNumber())
                << TAG(accout_bank_code, stat->getBankCode())
                << TAG(const_symbol, stat->getConstSymbol())
                << TAG(var_symbol, stat->getVarSymbol())
                // XXX spec symbol is not in bank_ebanka_list,
                // but this line is present in old
                // Register::Banking::OnlinePaymentListImpl
                << TAG(spec_symbol, "")
                << TAG(price, stat->getPrice().format())
                << TAG(memo, stat->getMemo())
                << TAG(invoice_id, stat->getInvoiceId())
                << TAG(accout_id, stat->getAccountId())
                << TAG(cr_date, stat->getCrDate())
                << TAG(accout_name, stat->getAccountName())
                << TAG(ident, stat->getIdent())
                << TAGEND(online_payment);
        }
        out <<
            TAGEND(online_payments);
    } // void exportXML(std::ostream &out)
}; // class OnlineListImpl

class ListImpl:
    public Register::CommonListImpl,
    virtual public List {
private:
    Manager *m_manager;
public:
    ListImpl(Database::Connection *conn, Manager *manager):
        CommonListImpl(conn),
        m_manager(manager)
    { }
    virtual Statement *get(unsigned int index) const
    {
        try {
            Statement *stat =
                dynamic_cast<Statement *>(data_.at(index));
            if (stat) {
                return stat;
            } else {
                throw std::exception();
            }
        } catch (...) {
            throw std::exception();
        }
    }
    virtual void reload(Database::Filters::Union &filter)
    {
        TRACE("[CALL] Register::Banking::StatementListImpl::reload(Database::Filters::Union &)");
        clear();
        filter.clearQueries();

        bool at_least_one = false;
        Database::SelectQuery id_query;
        Database::Filters::Compound::iterator sit = filter.begin();
        for (; sit != filter.end(); ++sit) {
            Database::Filters::Statement *sf =
                dynamic_cast<Database::Filters::Statement *>(*sit);
            if (!sf) {
                continue;
            }
            Database::SelectQuery *tmp = new Database::SelectQuery();
            tmp->addSelect(new Database::Column(
                        "id", sf->joinStatementTable(), "DISTINCT"));
            filter.addQuery(tmp);
            at_least_one = true;
        }
        if (!at_least_one) {
            LOGGER(PACKAGE).error("wrong filter passed for reload!");
            return;
        }
        id_query.limit(load_limit_);
        filter.serialize(id_query);
        Database::InsertQuery tmp_table_query =
            Database::InsertQuery(getTempTableName(), id_query);

        LOGGER(PACKAGE).debug(boost::format(
                    "temporary table '%1%' generated sql = %2%")
                % getTempTableName() % tmp_table_query.str());

        Database::SelectQuery object_info_query;
        object_info_query.select()
            << "t_1.id, t_1.account_id, t_1.num, t_1.create_date, "
            << "t_1.balance_old_date, t_1.balance_old * 100, "
            << "t_1.balance_new * 100, t_1.balance_credit * 100, "
            << "t_1.balance_debet * 100";
        object_info_query.from()
            << getTempTableName() << " tmp "
            << "JOIN bank_statement_head t_1 ON (tmp.id = t_1.id)";
        object_info_query.order_by()
            << "tmp.id";
        try {
            fillTempTable(tmp_table_query);

            Database::Result r_info = conn_->exec(object_info_query);
            Database::Result::Iterator it = r_info.begin();
            for (; it != r_info.end(); ++it) {
                Database::Row::Iterator col = (*it).begin();

                Database::ID id             = *col;
                Database::ID accountId      = *(++col);
                int number                  = *(++col);
                Database::Date crDate       = *(++col);
                Database::Date oldDate      = *(++col);
                Database::Money balance     = *(++col);
                Database::Money oldBalance  = *(++col);
                Database::Money credit      = *(++col);
                Database::Money debet       = *(++col);

                StatementImpl *stat = new StatementImpl(id, accountId,
                        number, crDate, oldDate,
                        balance, oldBalance, credit, debet);
                stat->setConn(conn_);
                data_.push_back(stat);
            }
            if (data_.empty()) {
                return;
            }
            resetIDSequence();
            Database::SelectQuery StatementItemQuery;
            StatementItemQuery.select()
                << "t_1.id, t_1.statement_id, t_1.account_number, t_1.bank_code, "
                << "t_1.code, t_1.konstSym, t_1.varSymb, t_1.specsymb, t_1.price * 100, "
                << "t_1.account_evid, t_1.account_date, t_1.account_memo, "
                << "t_1.invoice_id";
            StatementItemQuery.from()
                << getTempTableName() << " tmp "
                << "JOIN bank_statement_item t_1 ON (tmp.id = t_1.statement_id)";
            StatementItemQuery.order_by()
                << "tmp.id";
            Database::Result r_stat = conn_->exec(StatementItemQuery);
            Database::Result::Iterator statIt = r_stat.begin();
            for (; statIt != r_stat.end(); ++statIt) {
                Database::Row::Iterator col = (*statIt).begin();
                Database::ID id             = *col;
                Database::ID statementId    = *(++col);
                std::string accountNumber   = *(++col);
                std::string bankCode        = *(++col);
                int code                    = *(++col);
                std::string konstSymb       = *(++col);
                std::string varSymb         = *(++col);
                std::string specSymb        = *(++col);
                Database::Money price       = *(++col);
                std::string accountEvid     = *(++col);
                Database::Date accountDate  = *(++col);
                std::string accountMemo     = *(++col);
                Database::ID invoiceId      = *(++col);

                StatementImpl *statPtr =
                    dynamic_cast<StatementImpl *>(findIDSequence(statementId));
                if (statPtr) {
                    statPtr->addStatementItem(StatementItemImpl(id, statementId,
                                accountNumber, bankCode, code, konstSymb,
                                varSymb, specSymb, price, accountEvid, accountDate,
                                accountMemo, invoiceId, conn_));
                }
            }
            CommonListImpl::reload();
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
    } // void reload(Database::Filters::Union &filter)

    virtual void sort(MemberType member, bool asc)
    {
        switch (member) {
            case MT_ACCOUNT_NUMBER:
                stable_sort(data_.begin(), data_.end(), CompareAccountNumber(asc));
                break;
            case MT_BANK_CODE:
                stable_sort(data_.begin(), data_.end(), CompareBankCode(asc));
                break;
            case MT_PRICE:
                stable_sort(data_.begin(), data_.end(), ComparePrice(asc));
                break;
            case MT_INVOICE_ID:
                stable_sort(data_.begin(), data_.end(), CompareInvoiceId(asc));
                break;
        }
    }

    virtual const char *getTempTableName() const 
    {
        return "tmp_statement_filter_result";
    }

    virtual void makeQuery(bool, bool, std::stringstream &) const
    { }

    virtual void reload()
    { }

    virtual void exportXML(std::ostream &out)
    {
        out << TAGSTART(statements);
        for (int i = 0; i < (int)getCount(); i++) {
            Statement *stat = get(i);
            out << TAGSTART(statement)
                << TAG(account_number, stat->getAccountId())
                << TAG(number, stat->getNumber())
                << TAG(date, stat->getDate())
                << TAG(balance, stat->getBalance().format())
                << TAG(old_date, stat->getOldDate())
                << TAG(oldBalance, stat->getOldBalance().format())
                << TAG(credit, stat->getCredit().format())
                << TAG(debet, stat->getDebet().format())
                << TAGSTART(items);
            for (int j = 0; j < (int)stat->getStatementItemCount(); j++) {
                StatementItem *itm = (StatementItem *)stat->getStatementItemByIdx(j);
                out << TAGSTART(item)
                    << TAG(accout_number,itm->getAccountNumber())
                    << TAG(accout_bank_code,itm->getBankCode())
                    << TAG(const_symbol,itm->getConstSymbol())
                    << TAG(var_symbol,itm->getVarSymbol())
                    << TAG(spec_symbol,itm->getSpecSymbol())
                    << TAG(price,itm->getPrice().format())
                    << TAG(memo,itm->getMemo())
                    << TAG(date,itm->getDate())
                    << TAGEND(item);
            }
            out << TAGEND(items)
                << TAGEND(statement);
        }
        out << TAGEND(statements);
    } // void exportXML(std::ostream &out)

}; // class ListImpl

class ManagerImpl:
    virtual public Manager {
private:
    Database::Connection *m_conn;
public:
    ManagerImpl(Database::Manager *dbMan):
        m_conn(dbMan->getConnection())
    { }
    virtual ~ManagerImpl()
    {
        boost::checked_delete<Database::Connection>(m_conn);
    }
    List *createList() const
    {
        return new ListImpl(m_conn, (Manager *)this);
    }
    OnlineList *createOnlineList() const
    {
        return new OnlineListImpl(m_conn, (Manager *)this);
    }
    virtual OnlineStatement *createOnlineStatement() const
    {
        return new OnlineStatementImpl(m_conn);
    }
    virtual Statement *createStatement() const
    {
        return new StatementImpl(m_conn);
    }
}; // class ManagerImpl

Manager *
Manager::create(Database::Manager *dbMan)
{
    TRACE("[CALL] Register::Banking::Manager::create()");
    return new ManagerImpl(dbMan);
}

} // namespace Bank
} // namespace Register

