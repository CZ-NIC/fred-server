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
#include "invoice.h"
#include "log/logger.h"
#include "types/convert_sql_db_types.h"
#include "types/sqlize.h"
#include "types/stringify.h"

#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>

namespace Register {
namespace Banking {

#define XML_NODE_NONE
#define XML_NODE_ELEMENT
#define XML_NODE_ATTRIBUTE
#define XML_NODE_TEXT
#define XML_NODE_CDATA
#define XML_NODE_ENTITY_REFERENCE
#define XML_NODE_ENTITY
#define XML_NODE_PROCESSING_INSTRUCTION
#define XML_NODE_COMMENT
#define XML_NODE_DOCUMENT
#define XML_NODE_DOCUMENT_TYPE
#define XML_NODE_DOCUMENT_FRAGMENT
#define XML_NODE_NOTATION
#define XML_NODE_WHITESPACE
#define XML_NODE_SIGNIFICANT_WHITESPACE
#define XML_NODE_END_ELEMENT
#define XML_NODE_END_ENTITY
#define XML_NODE_XML_DECLARATION

#define STATEMENTS_ROOT             "statements"
#define STATEMENT_STATEMENT         "statement"
#define STATEMENT_ACCOUNT_NUMBER    "account_number"
#define STATEMENT_NUMBER            "number"
#define STATEMENT_DATE              "date"
#define STATEMENT_BALANCE           "balance"
#define STATEMENT_OLD_DATE          "old_date"
#define STATEMENT_OLD_BALANCE       "oldBalance"
#define STATEMENT_CREDIT            "credit"
#define STATEMENT_DEBET             "debet"
#define STATEMENT_ITEMS             "items"
#define ITEM_ITEM                   "item"
#define ITEM_IDENT                  "ident"
#define ITEM_ACCOUNT_NUMBER         "account_number"
#define ITEM_ACCOUNT_BANK_CODE      "account_bank_code"
#define ITEM_CONST_SYMBOL           "const_symbol"
#define ITEM_VAR_SYMBOL             "var_symbol"
#define ITEM_SPEC_SYMBOL            "spec_symbol"
#define ITEM_PRICE                  "price"
#define ITEM_CODE                   "code"
#define ITEM_MEMO                   "memo"
#define ITEM_DATE                   "date"
#define ITEM_NAME                   "name"


#define TAGSTART(tag) "<"#tag">"
#define TAGEND(tag) "</"#tag">"
#define TAG(tag,f) TAGSTART(tag) << f << TAGEND(tag)

#define transformString(str)    \
    ((str.empty()) ? "NULL" : "'" + str + "'")

#define transformId(id) \
    ((id == Database::ID()) ? "NULL" : sqlize(id))

#define TEST_NODE_PRESENCE(parent, name)                                \
    if (!parent.hasChild(name)) {                                       \
        LOGGER(PACKAGE).error(boost::format("``%1%'' node not found")   \
                % name);                                                \
        return false;                                                   \
    }

std::string
loadInStream(std::istream &in)
{
    char ch;
    std::stringstream ss;
    while (!in.eof()) {
        in.get(ch);
        ss << ch;
    }
    return ss.str();
}

class XMLcreator {
private:
    xmlBuffer       *m_buffer;
    xmlTextWriter   *m_writer;
    bool            m_writeXmlHeader;
public:
    XMLcreator(): m_buffer(NULL), m_writer(NULL)
    { }
    ~XMLcreator()
    {
        if (m_writer != NULL) {
            xmlFreeTextWriter(m_writer);
        }
        xmlBufferFree(m_buffer);
    }
    bool init(bool writeXmlHeader = false)
    {
        m_writeXmlHeader = writeXmlHeader;
        m_buffer = xmlBufferCreate();
        if (m_buffer == NULL) {
            return false;
        }
        m_writer = xmlNewTextWriterMemory(m_buffer, 0);
        if (m_writer == NULL) {
            return false;
        }
        if (m_writeXmlHeader) {
            xmlTextWriterStartDocument(m_writer, NULL, NULL, NULL);
        }
        return true;
    }
    void start(std::string name)
    {
        if (xmlTextWriterStartElement(
                    m_writer, (const xmlChar *)name.c_str()) < 0) {
            throw std::exception();
        }
    }
    void start(const char *name)
    {
        if (xmlTextWriterStartElement(
                    m_writer, (const xmlChar *)name) < 0) {
            throw std::exception();
        }
    }
    void end()
    {
        if (xmlTextWriterEndElement(m_writer) < 0) {
            throw std::exception();
        }
    }
    void text(std::string name, std::string value)
    {
        if (xmlTextWriterWriteFormatElement(
                    m_writer, (const xmlChar *)name.c_str(), value.c_str()) < 0) {
            throw std::exception();
        }
    }
    void text(std::string name, const char *value)
    {
        if (xmlTextWriterWriteFormatElement(
                    m_writer, (const xmlChar *)name.c_str(), value) < 0) {
            throw std::exception();
        }
    }
    void text(std::string name, int value)
    {
        if (xmlTextWriterWriteFormatElement(
                    m_writer, (const xmlChar *)name.c_str(), "%d", value) < 0) {
            throw std::exception();
        }
    }
    void text(std::string name, double value)
    {
        if (xmlTextWriterWriteFormatElement(
                    m_writer, (const xmlChar *)name.c_str(), "%lf", value) < 0) {
            throw std::exception();
        }
    }
    void text(std::string name, Database::ID value)
    {
        this->text(name, stringify(value));
    }
    void text(std::string name, Database::Date value)
    {
        this->text(name, stringify(value));
    }
    void text(std::string name, Database::DateTime value)
    {
        this->text(name, stringify(value));
    }
    void text(std::string name, Database::Money value)
    {
        this->text(name, stringify(value));
    }
    std::string finalize()
    {
        xmlFreeTextWriter(m_writer);
        m_writer = NULL;
        return std::string((const char *)m_buffer->content);
    }
}; // class XMLcreator;

class XMLnode {
private:
    std::string             m_name;
    std::string             m_value;
    std::vector<XMLnode>    m_children;
public:
    XMLnode()
    { }
    XMLnode(const XMLnode &sec)
    {
        m_name = sec.getName();
        m_value = sec.getValue();
        for (int i = 0; i < sec.getChildrenSize(); i++) {
            addChild(sec.getChild(i));
        }
    }
    ~XMLnode()
    { }
    XMLnode operator=(const XMLnode &sec)
    {
        m_name = sec.getName();
        m_value = sec.getValue();
        for (int i = 0; i < sec.getChildrenSize(); i++) {
            addChild(sec.getChild(i));
        }
        return sec;
    }
    void setName(std::string name)
    {
        m_name = name;
    }
    void setValue(std::string value)
    {
        m_value = value;
    }
    void addChild(XMLnode node)
    {
        m_children.push_back(node);
    }
    std::string getName() const
    {
        return m_name;
    }
    std::string getValue() const
    {
        return m_value;
    }
    int getChildrenSize() const
    {
        return m_children.size();
    }
    int getChildrenSize(std::string name) const
    {
        int size = 0;
        for (int i = 0; i < getChildrenSize(); i++) {
            if (name.compare(getChild(i).getName()) == 0) {
                size++;
            }
        }
        return size;
    }
    XMLnode getChild(int i) const
    {
        if (i > (int)m_children.size()) {
            throw std::exception();
        }
        return m_children[i];
    }
    bool hasChild(std::string name) const
    {
        for (int i = 0; i < getChildrenSize(); i++) {
            if (name.compare(getChild(i).getName()) == 0) {
                return true;
            }
        }
        return false;
    }
    XMLnode getChild(std::string name) const
    {
        for (int i = 0; i < getChildrenSize(); i++) {
            if (name.compare(getChild(i).getName()) == 0) {
                return getChild(i);
            }
        }
        return XMLnode();
    }
}; // class XMLnode;

class XMLparser {
private:
    xmlDoc      *m_doc;
    xmlNode     *m_root;
    XMLnode     m_rootNode;
    XMLnode parseNode(xmlNode *node)
    {
        XMLnode ret;
        xmlNode *child;
        ret.setName((const char *)node->name);
        child = node->children;
        if (child) {
            if (child->type == XML_TEXT_NODE) {
                if (*child->content != '\n') {
                    ret.setValue((const char *)child->content);
                }
            }
            for (; child; child = child->next) {
                if (child->type == XML_ELEMENT_NODE) {
                    ret.addChild(parseNode(child));
                } else if (child->type == XML_TEXT_NODE) {
                }
            }
        }
        return ret;
    }
public:
    XMLparser(): m_doc(NULL), m_root(NULL)
    { }
    ~XMLparser()
    {
    }
    bool parse(std::string xml)
    {
        m_doc = xmlReadMemory(xml.c_str(), xml.length(), "", NULL, 0);
        if (m_doc == NULL) {
            return false;
        }
        m_root = xmlDocGetRootElement(m_doc);
        if (m_root == NULL) {
            return false;
        }
        try {
            m_rootNode = parseNode(m_root);
        } catch (...) {
            return false;
        }
        xmlFreeDoc(m_doc);
        xmlCleanupParser();
        return true;
    }
    XMLnode getRootNode() const
    {
        return m_rootNode;
    }
}; // class XMLparse;

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
    bool update(Database::Transaction &transaction)
    {
        TRACE("[CALL] Register::Banking::OnlineStatement::update("
                "Database::Transaction &)");
        std::auto_ptr<Database::Query> updateStat(new Database::Query());
        Database::UpdateQuery uquery("bank_ebanka_list");
        uquery.add("account_id", getAccountId());
        uquery.add("price", getPrice());
        uquery.add("crdate", getCrDate());
        uquery.add("account_number", getAccountNumber());
        uquery.add("bank_code", getBankCode());
        uquery.add("konstsym", getConstSymbol());
        uquery.add("varsymb", getVarSymbol());
        uquery.add("memo", getMemo());
        uquery.add("name", getAccountName());
        uquery.add("ident", getIdent());
        uquery.add("invoice_id", getInvoiceId());
        uquery.where().add("id", "=", id_, "AND");
        try {
            assert(m_conn);
            transaction.exec(uquery);
            transaction.commit();
            LOGGER(PACKAGE).info(boost::format(
                        "online payment item id='%1%' updated successfully")
                    % id_);
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // void OnlineStatementImpl::update(Database::Transaction &)

    bool update()
    {
        TRACE("[CALL] Register::Banking::OnlineStatement::update()");
        Database::Transaction transaction(*m_conn);
        return update(transaction);
    } // void OnlineStatementImpl::update()

    void updateBankAccount()
    {
        TRACE("[CALL] Register::Banking::OnlineStatementImpl::"
                "updateBankAccount()");
        Database::Query query;
        query.buffer()
            << "UPDATE bank_account SET balance = balance + "
            << Database::Value(getPrice())
            // TODO now i have payment number format incopatible with
            // last_num column - so do not update it
            << ", last_date = " << Database::Value(getCrDate().date()) //, last_num=" << getIdent()
            << " WHERE id=" << Database::Value(getAccountId());
        try {
            Database::Transaction transaction(*m_conn);
            transaction.exec(query);
            transaction.commit();
        } catch (...) {
            LOGGER(PACKAGE).error("Unable to update ``bank_account'' table");
        }
    }

    bool insert(Database::Transaction &transaction) 
    {
        TRACE("[CALL] Register::Banking::OnlineStatement::insert("
                "Database::Transaction &)");
        Database::InsertQuery insertStat("bank_ebanka_list");
        insertStat.add("account_id", getAccountId());
        insertStat.add("price", getPrice().format());
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
            transaction.exec(insertStat);
            transaction.commit();
            Database::Sequence seq(*m_conn, "bank_ebanka_list_id_seq");
            id_ = seq.getCurrent();
            LOGGER(PACKAGE).info(boost::format(
                        "online statement item id='%1%' created successfully")
                    % id_);
            updateBankAccount();
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // void OnlineStatementImpl::insert(Database::Transaction &)

    bool insert()
    {
        TRACE("[CALL] Register::Banking::OnlineStatement::insert()");
        Database::Transaction transaction(*m_conn);
        return insert(transaction);
    } // void OnlineStatementImpl::insert()

    Database::ID getBankNumberId(std::string account_num, std::string bank_code)
    {
        Database::Query query;
        query.buffer()
            << "select id from bank_account where "
            << "trim(leading '0' from account_number) = "
            << "trim(leading '0' from "
            << Database::Value(account_num) << ") and bank_code = "
            << Database::Value(bank_code);
        Database::Result res = m_conn->exec(query);
        if (res.size() == 0) {
            return Database::ID();
        }
        return *(*res.begin()).begin();
    }
    Database::ID getBankNumberId(std::string account_num)
    {
        std::string account;
        std::string code;
        account = account_num.substr(0, account_num.find('/'));
        code = account_num.substr(account_num.find('/') + 1, std::string::npos);
        return getBankNumberId(account, code);
    }
    virtual bool save(Database::Transaction &transaction)
    {
        TRACE("[CALL] Register::Banking::OnlineStatementImpl::save("
                "Database::Transaction &)");
        try {
            if (id_) {
                update(transaction);
            } else {
                insert(transaction);
            }
        } catch (...) {
            return false;
        }
        return true;
    }
    virtual bool save() 
    {
        TRACE("[CALL] Register::Banking::OnlineStatementImpl::save()");
        try {
            if (id_) {
                update();
            } else {
                insert();
            }
        } catch (...) {
            return false;
        }
        return true;
    }
    bool fromXML(XMLnode statement, XMLnode item)
    {
        TRACE("[CALL] Register::Banking::OnlineStatementImpl::fromXML()");
        if (!statement.hasChild(STATEMENT_ACCOUNT_NUMBER)) {
            LOGGER(PACKAGE).error("account number not found");
            return false;
        }
        if (!item.hasChild(ITEM_ACCOUNT_NUMBER)) {
            LOGGER(PACKAGE).error("account number not found");
            return false;
        }
        if (!item.hasChild(ITEM_ACCOUNT_BANK_CODE)) {
            LOGGER(PACKAGE).error("bank code not found");
            return false;
        }
        if (!item.hasChild(ITEM_PRICE)) {
            LOGGER(PACKAGE).error("price not found");
            return false;
        }
        if (!item.hasChild(ITEM_DATE)) {
            LOGGER(PACKAGE).error("date not found");
            return false;
        }
        setAccountId(getBankNumberId(
                statement.getChild(STATEMENT_ACCOUNT_NUMBER).getValue()));
        if (getAccountId() == Database::ID()) {
            LOGGER(PACKAGE).error("account number does not exist in db");
            return false;
        }
        setIdent(item.getChild(ITEM_IDENT).getValue());
        setAccountNumber(item.getChild(ITEM_ACCOUNT_NUMBER).getValue());
        setBankCode(item.getChild(ITEM_ACCOUNT_BANK_CODE).getValue());
        setConstSymbol(item.getChild(ITEM_CONST_SYMBOL).getValue());
        setVarSymbol(item.getChild(ITEM_VAR_SYMBOL).getValue());
        Database::Money money;
        money.format(item.getChild(ITEM_PRICE).getValue());
        setPrice(money);
        setMemo(item.getChild(ITEM_MEMO).getValue());
        setCrDate(Database::DateTime(item.getChild(ITEM_DATE).getValue()));
        setAccountName(item.getChild(ITEM_NAME).getValue());

        return true;
    } // bool OnlineStatementImpl::fromXML(XMLnode)
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
    virtual const std::string &getAccountEvid() const
    {
        return m_accountEvid;
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
    virtual void setAccountEvid(std::string accountEvid)
    {
        m_accountEvid = accountEvid;
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
    bool update()
    {
        TRACE("[CALL] Register::Banking::StatementItemImpl::update()");
        Database::UpdateQuery uquery("bank_statement_item");
        uquery.add("statement_id", getStatementId());
        uquery.add("account_number", getAccountNumber());
        uquery.add("bank_code", getBankCode());
        uquery.add("code", getCode());
        uquery.add("konstsym", getConstSymbol());
        uquery.add("varsymb", getVarSymbol());
        uquery.add("price", getPrice());
        uquery.add("account_evid", getEvidenceNumber());
        uquery.add("account_date", getDate());
        uquery.add("account_memo", getMemo());
        uquery.add("invoice_id", getInvoiceId());
        uquery.where().add("id", "=", m_id, "AND");
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(uquery);
            transaction.commit();
            LOGGER(PACKAGE).info(boost::format(
                        "statement item id='%1%' updated successfully")
                    % m_id);
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // void StatementItemImpl::update()

    /* test if statement is already present in database,
     * ``account_id'' is ID of our own bank account
     */
    bool isExisting(Database::ID account_id)
    {
        Database::Query query;
        query.buffer()
            << "SELECT si.id FROM bank_statement_item si "
            << "JOIN bank_statement_head sh ON si.statement_id=sh.id "
            << "WHERE si.account_evid = " << Database::Value(getEvidenceNumber())
            << "AND si.account_date = " << Database::Value(getDate())
            << "AND sh.account_id = " << Database::Value(account_id);
        Database::Result res = m_conn->exec(query);
        if (res.size() == 0) {
            return false;
        }
        return true;
    }

    bool insert()
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
        insertItem.add("price", getPrice().format());
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
            return false;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // void StatementItemImpl::insert()
    bool save()
    {
        TRACE("[CALL] Register::Banking::StatementItemImpl::save()");
        bool retval;
        if (m_id) {
            retval = update();
        } else {
            retval = insert();
        }
        return retval;
    }
    bool fromXML(XMLnode item)
    {
        TRACE("[CALL] Register::Banking;:StatementItemImpl::fromXML()");
        TEST_NODE_PRESENCE(item, ITEM_IDENT);
        TEST_NODE_PRESENCE(item, ITEM_ACCOUNT_NUMBER);
        TEST_NODE_PRESENCE(item, ITEM_ACCOUNT_BANK_CODE);
        TEST_NODE_PRESENCE(item, ITEM_PRICE);
        setAccountEvid(item.getChild(ITEM_IDENT).getValue());
        setAccountNumber(item.getChild(ITEM_ACCOUNT_NUMBER).getValue());
        setBankCode(item.getChild(ITEM_ACCOUNT_BANK_CODE).getValue());
        setConstSymbol(item.getChild(ITEM_CONST_SYMBOL).getValue());
        setVarSymbol(item.getChild(ITEM_VAR_SYMBOL).getValue());
        setSpecSymbol(item.getChild(ITEM_SPEC_SYMBOL).getValue());
        Database::Money money;
        money.format(item.getChild(ITEM_PRICE).getValue());
        setPrice(money);
        setCode(atoi(item.getChild(ITEM_CODE).getValue().c_str()));
        setMemo(item.getChild(ITEM_MEMO).getValue());
        setDate(Database::Date(item.getChild(ITEM_DATE).getValue()));
        return true;
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
    bool isExisting()
    {
        Database::Query query;
        query.buffer()
            << "SELECT id FROM bank_statement_head "
            << "WHERE num = " << Database::Value(getNumber())
            << " AND account_id = " << Database::Value(getAccountId())
            << " AND create_date = " << Database::Value(getDate());
        Database::Result res = m_conn->exec(query);
        if (res.size() == 0) {
            return false;
        }
        return true;
    }
    bool update()
    {
        TRACE("[CALL] Register::Banking::StatementImpl::update()");
        Database::UpdateQuery uquery("bank_statement_head");
        uquery.add("account_id", getAccountId());
        uquery.add("num", getNumber());
        uquery.add("create_date", getDate());
        uquery.add("balance_old_date", getOldDate());
        uquery.add("balance_old", getOldBalance());
        uquery.add("balance_new", getBalance());
        uquery.add("balance_credit", getCredit());
        uquery.add("balance_debet", getDebet());
        uquery.where().add("id", "=", id_, "AND");
        try {
            assert(m_conn);
            Database::Transaction transaction(*m_conn);
            transaction.exec(uquery);
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
            return false;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // void StatementImpl::update()

    /* update columns in bank_account table: balance, last_date and
     * last_num
     */
    void updateBankAccount()
    {
        TRACE("[CALL] Register::Banking::StatementImpl::updateBankAccount()");
        Database::Query query;
        query.buffer()
            << "UPDATE bank_account SET balance = balance + " <<
            ((getCredit() == Database::Money()) ? Database::Value(getDebet()) : Database::Value(getCredit()))
            << ", last_date = " << Database::Value(getDate()) 
            << ", last_num = " << Database::Value(getNumber())
            << " WHERE id = " << Database::Value(getAccountId());
        try {
            Database::Transaction transaction(*m_conn);
            transaction.exec(query);
            transaction.commit();
        } catch (...) {
            LOGGER(PACKAGE).error("Unable to update ``bank_account'' table");
        }
    }

    bool insert()
    {
        TRACE("[CALL] Register::Banking::StatementImpl::insert()");
        Database::InsertQuery insertHead("bank_statement_head");
        insertHead.add("account_id", getAccountId());
        insertHead.add("num", getNumber());
        insertHead.add("create_date", getDate());
        insertHead.add("balance_old_date", getOldDate());
        insertHead.add("balance_old", getOldBalance().format());
        insertHead.add("balance_new", getBalance().format());
        insertHead.add("balance_credit", getCredit().format());
        insertHead.add("balance_debet", getDebet().format());
        if (isExisting()) {
            LOGGER(PACKAGE).warning("Payment with same number, account number and "
                    "date already exists");
            return true;
        }
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
            int itemsStored = 0;
            for (int i = 0; i < (int)getStatementItemCount(); i++) {
                StatementItemImpl *item = dynamic_cast<StatementItemImpl *>(
                        const_cast<StatementItem *>(getStatementItemByIdx(i)));
                item->setStatementId(id_);
                if (item->isExisting(getAccountId())) {
                    LOGGER(PACKAGE).warning("Payment item with same number, account "
                            "nubmer and date already exists");
                } else {
                    if (item->save()) {
                        itemsStored++;
                    }
                }
            }
            if (itemsStored == 0) {
                LOGGER(PACKAGE).warning("There are no stored payment items for this payment");
            } else {
                updateBankAccount();
            }
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // void StatementImpl::insert()
    virtual bool save()
    {
        TRACE("[CALL] Register::Banking::StatementImpl::save()");
        bool retval;
        if (id_) {
            retval = update();
        } else {
            retval = insert();
        }
        return retval;
    }
    virtual StatementItem *createStatementItem()
    {
        return addStatementItem(StatementItemImpl(m_conn));
    }
    Database::ID getBankNumberId(std::string account_num, std::string bank_code)
    {
        Database::Query query;
        query.buffer()
            << "select id from bank_account where " 
            << "trim(leading '0' from account_number) = "
            << "trim(leading '0' from "
            << Database::Value(account_num) << ") and bank_code = "
            << Database::Value(bank_code);
        Database::Result res = m_conn->exec(query);
        if (res.size() == 0) {
            return Database::ID();
        }
        return *(*res.begin()).begin();
    }
    Database::ID getBankNumberId(std::string account_num)
    {
        std::string account;
        std::string code;
        account = account_num.substr(0, account_num.find('/'));
        code = account_num.substr(account_num.find('/') + 1, std::string::npos);
        return getBankNumberId(account, code);
    }
    bool fromXML(XMLnode node)
    {
        TRACE("[CALL] Register::Banking::StatementImpl::fromXML()");
        TEST_NODE_PRESENCE(node, STATEMENT_ACCOUNT_NUMBER);
        TEST_NODE_PRESENCE(node, STATEMENT_DATE);
        TEST_NODE_PRESENCE(node, STATEMENT_OLD_DATE);
        TEST_NODE_PRESENCE(node, STATEMENT_OLD_BALANCE);
        TEST_NODE_PRESENCE(node, STATEMENT_BALANCE);
        TEST_NODE_PRESENCE(node, STATEMENT_CREDIT);
        TEST_NODE_PRESENCE(node, STATEMENT_DEBET);
        setAccountId(getBankNumberId(
                    node.getChild(STATEMENT_ACCOUNT_NUMBER).getValue()));
        if (getAccountId() == Database::ID()) {
            LOGGER(PACKAGE).error("account number does not exist in db");
            return false;
        }
        setNumber(atoi(node.getChild(STATEMENT_NUMBER).getValue().c_str()));
        setDate(Database::Date(node.getChild(STATEMENT_DATE).getValue()));
        setOldDate(Database::Date(node.getChild(STATEMENT_OLD_DATE).getValue()));
        Database::Money money;
        money.format(node.getChild(STATEMENT_BALANCE).getValue());
        setOldBalance(money);
        money.format(node.getChild(STATEMENT_OLD_BALANCE).getValue());
        setBalance(money);
        money.format(node.getChild(STATEMENT_CREDIT).getValue());
        setCredit(money);
        money.format(node.getChild(STATEMENT_DEBET).getValue());
        setDebet(money);
        XMLnode items = node.getChild(STATEMENT_ITEMS);
        for (int i = 0; i < items.getChildrenSize(); i++) {
            StatementItemImpl *item =
                dynamic_cast<StatementItemImpl *>(createStatementItem());
            if (!item->fromXML(items.getChild(i))) {
                return false;
            }
        }
        return true;
    } // StatementImpl::fromXML(XMLnode)
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
            << "t_1.id, t_1.account_id, t_1.price, t_1.crdate, "
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

                OnlineStatementImpl *stat = new OnlineStatementImpl(conn_);
                stat->setId(id);
                stat->setAccountId(accountId);
                stat->setPrice(price);
                stat->setCrDate(crDate);
                stat->setAccountNumber(accountNumber);
                stat->setBankCode(bankCode);
                stat->setConstSymbol(constSymbol);
                stat->setVarSymbol(varSymbol);
                stat->setMemo(memo);
                stat->setAccountName(name);
                stat->setIdent(ident);
                stat->setInvoiceId(invoiceId);
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
    } // void OnlineListImpl::reload(Database::Filters::Union &filter)

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

    void writeStatement(XMLcreator &xml, OnlineStatement *stat)
    {
        TRACE("Register::Banking::OnlineStatementImpl::writeStatement"
                "(XMLcreator &, OnlineStatement *)");
        xml.start(STATEMENT_STATEMENT);
        xml.text(STATEMENT_ACCOUNT_NUMBER, stat->getAccountId());
        xml.text(STATEMENT_NUMBER, "");
        xml.text(STATEMENT_DATE, stat->getCrDate());
        xml.text(STATEMENT_BALANCE, "");
        xml.text(STATEMENT_OLD_DATE, "");
        xml.text(STATEMENT_OLD_BALANCE, "");
        xml.text(STATEMENT_CREDIT, "");
        xml.text(STATEMENT_DEBET, "");
        xml.start(STATEMENT_ITEMS);
        xml.text(ITEM_IDENT, stat->getIdent());
        xml.text(ITEM_ACCOUNT_NUMBER, stat->getAccountNumber());
        xml.text(ITEM_ACCOUNT_BANK_CODE, stat->getBankCode());
        xml.text(ITEM_CONST_SYMBOL, stat->getConstSymbol());
        xml.text(ITEM_VAR_SYMBOL, stat->getVarSymbol());
        xml.text(ITEM_SPEC_SYMBOL, "");
        xml.text(ITEM_PRICE, stat->getPrice().format());
        // code is always 2 (credit) for online payment
        xml.text(ITEM_CODE, 2);
        xml.text(ITEM_MEMO, stat->getMemo());
        xml.text(ITEM_DATE, stat->getCrDate());
        xml.text(ITEM_NAME, stat->getAccountName());
        xml.end();
        xml.end();
    }
    void writeStatements(XMLcreator &xml)
    {
        xml.start(STATEMENTS_ROOT);
        for (int i = 0; i < (int)getCount(); i++) {
            OnlineStatement *stat = get(i);
            writeStatement(xml, stat);
        }
        xml.end();
    }
    virtual bool exportXML(std::ostream &out)
    {
        TRACE("Register::Banking::OnlineStatementImpl::exportXML(std::ostrem &)");
        XMLcreator xml;
        if (!xml.init()) {
            return false;
        }
        try {
            writeStatements(xml);
        } catch (...) {
            return false;
        }
        out << xml.finalize();
        return true;
    } // bool OnlineListImpl::exportXML()
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
            << "t_1.balance_old_date, t_1.balance_old, "
            << "t_1.balance_new, t_1.balance_credit, "
            << "t_1.balance_debet";
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

                StatementImpl *stat = new StatementImpl(conn_);
                stat->setId(id);
                stat->setAccountId(accountId);
                stat->setNumber(number);
                stat->setDate(crDate);
                stat->setOldDate(oldDate);
                stat->setBalance(balance);
                stat->setOldBalance(oldBalance);
                stat->setCredit(credit);
                stat->setDebet(debet);
                data_.push_back(stat);
            }
            if (data_.empty()) {
                return;
            }
            resetIDSequence();
            Database::SelectQuery StatementItemQuery;
            StatementItemQuery.select()
                << "t_1.id, t_1.statement_id, t_1.account_number, t_1.bank_code, "
                << "t_1.code, t_1.konstSym, t_1.varSymb, t_1.specsymb, t_1.price, "
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
                std::string constSymb       = *(++col);
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
                    StatementItemImpl statementItem = StatementItemImpl(conn_);
                    statementItem.setId(id);
                    statementItem.setStatementId(statementId);
                    statementItem.setAccountNumber(accountNumber);
                    statementItem.setBankCode(bankCode);
                    statementItem.setCode(code);
                    statementItem.setConstSymbol(constSymb);
                    statementItem.setVarSymbol(varSymb);
                    statementItem.setSpecSymbol(specSymb);
                    statementItem.setPrice(price);
                    statementItem.setAccountEvid(accountEvid);
                    statementItem.setDate(accountDate);
                    statementItem.setMemo(accountMemo);
                    statementItem.setInvoiceId(invoiceId);
                    statPtr->addStatementItem(statementItem);
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
    } // void ListImpl::reload(Database::Filters::Union &filter)

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

    void
    writeItem(XMLcreator &xml, StatementItem *item)
    {
        xml.start(ITEM_ITEM);
        xml.text(ITEM_IDENT, item->getAccountEvid());
        xml.text(ITEM_ACCOUNT_NUMBER, item->getAccountNumber());
        xml.text(ITEM_ACCOUNT_BANK_CODE, item->getBankCode());
        xml.text(ITEM_CONST_SYMBOL, item->getConstSymbol());
        xml.text(ITEM_VAR_SYMBOL, item->getVarSymbol());
        xml.text(ITEM_SPEC_SYMBOL, item->getSpecSymbol());
        xml.text(ITEM_PRICE, item->getPrice());
        xml.text(ITEM_CODE, item->getCode());
        xml.text(ITEM_MEMO, item->getMemo());
        xml.text(ITEM_DATE, item->getDate());
        xml.text(ITEM_NAME, "");
        xml.end();
    } // void ListImpl::writeItem()

    void
    writeStatement(XMLcreator &xml, Statement *stat)
    {
        TRACE("Register::Banking::StatementImpl::writeStatement"
                "(XMLcreator &, Statement *)");
        xml.start(STATEMENT_STATEMENT);
        xml.text(STATEMENT_ACCOUNT_NUMBER, stat->getAccountId());
        xml.text(STATEMENT_NUMBER, stat->getNumber());
        xml.text(STATEMENT_DATE, stat->getDate());
        xml.text(STATEMENT_BALANCE, stat->getBalance());
        xml.text(STATEMENT_OLD_DATE, stat->getOldDate());
        xml.text(STATEMENT_OLD_BALANCE, stat->getOldBalance());
        xml.text(STATEMENT_CREDIT, stat->getCredit());
        xml.text(STATEMENT_DEBET, stat->getDebet());
        xml.start(STATEMENT_ITEMS);
        for (int i = 0; i < (int)stat->getStatementItemCount(); i++) {
            StatementItem *item = (StatementItem *)stat->getStatementItemByIdx(i);
            writeItem(xml, item);
        }
        xml.end();
        xml.end();
    } // void ListImpl::writeStatement()

    void
    writeStatements(XMLcreator &xml)
    {
        xml.start(STATEMENTS_ROOT);
        for (int i = 0; i < (int)getCount(); i++) {
            Statement *stat = get(i);
            writeStatement(xml, stat);
        }
        xml.end();
    } // void ListImpl::writeStatements()

    virtual bool exportXML(std::ostream &out)
    {
        TRACE("Register::Banking::StatementImpl::exportXML(std::ostrem &)");
        XMLcreator xml;
        if (!xml.init()) {
            return false;
        }
        try {
            writeStatements(xml);
        } catch (...) {
            return false;
        }
        out << xml.finalize();
        return true;
    } // void exportXML(std::ostream &out)

}; // class ListImpl

class ManagerImpl:
    virtual public Manager {
private:
    Database::Connection    *m_conn;
    Database::Manager       *m_dbMan;
public:
    ManagerImpl(Database::Manager *dbMan):
        m_conn(dbMan->getConnection()),
        m_dbMan(dbMan)
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

    virtual bool importStatementXml(std::istream &in, bool createCreditInvoice)
    {
        TRACE("[CALL] Register::Banking::Manager::importStatementXml("
                "std::istream &)");
        std::string xml = loadInStream(in);
        XMLparser parser;
        if (!parser.parse(xml)) {
            return false;
        }
        XMLnode root = parser.getRootNode();
        if (root.getName().compare(STATEMENTS_ROOT) != 0) {
            LOGGER(PACKAGE).error(boost::format(
                        "XML: root element name is not ``%1%''")
                    % STATEMENTS_ROOT);
            return false;
        }
        for (int i = 0; i < root.getChildrenSize(); i++) {
            XMLnode statement = root.getChild(i);
            std::auto_ptr<StatementImpl> stat(
                    dynamic_cast<StatementImpl *>(
                        createStatement()));
            if (!stat->fromXML(statement)) {
                return false;
            }
            stat->save();
        }
        if (createCreditInvoice) {
            std::auto_ptr<Invoicing::Manager>
                invMan(Invoicing::Manager::create(m_dbMan));
            return invMan->pairInvoices();
        }
        return true;
    } // ManagerImpl::importStatementXml()

    virtual bool importOnlineStatementXml(std::istream &in, bool createCreditInvoice)
    {
        TRACE("[CALL] Register::Banking::Manager::importOnlineStatementXml("
                "std::istream &)");
        std::string xml = loadInStream(in);
        XMLparser parser;
        if (!parser.parse(xml)) {
            return false;
        }
        XMLnode root = parser.getRootNode();
        if (root.getName().compare(STATEMENTS_ROOT) != 0) {
            LOGGER(PACKAGE).error(boost::format(
                        "XML: root element name is not ``%1%''")
                    % STATEMENTS_ROOT);
            return false;
        }
        for (int i = 0; i < root.getChildrenSize(); i++) {
            XMLnode statement = root.getChild(i);
            XMLnode items = statement.getChild(STATEMENT_ITEMS);
            for (int j = 0; j < items.getChildrenSize(); j++) {
                std::auto_ptr<OnlineStatementImpl> stat(
                        dynamic_cast<OnlineStatementImpl *>(
                            createOnlineStatement()));
                if (!stat->fromXML(statement, items.getChild(j))) {
                    return false;
                }
                stat->save();
            }
        }
        if (createCreditInvoice) {
            std::auto_ptr<Invoicing::Manager>
                invMan(Invoicing::Manager::create(m_dbMan));
            return invMan->pairInvoices();
        }
        return true;
    } // bool ManagerImpl::importOnlineStatementXml()
    virtual bool insertBankAccount(const Database::ID &zone, 
        const std::string &account_number, const std::string &account_name,
        const std::string &bank_code);
}; // class ManagerImpl

Manager *
Manager::create(Database::Manager *dbMan)
{
    TRACE("[CALL] Register::Banking::Manager::create()");
    return new ManagerImpl(dbMan);
}

bool 
ManagerImpl::insertBankAccount(const Database::ID &zone, 
        const std::string &account_number, const std::string &account_name,
        const std::string &bank_code)
{
    TRACE("[CALL] Register::Banking::Manager::insertBankAccount(...)");
    Database::InsertQuery insertAccount("bank_account");
    insertAccount.add("zone", zone);
    insertAccount.add("account_number", account_number);
    insertAccount.add("account_name", account_name);
    insertAccount.add("bank_code", bank_code);
    try {
        m_conn->exec(insertAccount);
    } catch (Database::Exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        return false;
    } catch (std::exception &ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        return false;
    } catch (...) {
        LOGGER(PACKAGE).error("Cannot insert new account into the ``bank_account'' table");
        return false;
    }
    return true;
}
} // namespace Banking
} // namespace Register

