/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
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

#ifndef _BANK_H_
#define _BANK_H_

#include "common_object.h"
#include "object.h"
#include "db_settings.h"
#include "model/model_filters.h"

namespace Register {
namespace Banking {

enum MemberType {
    MT_ACCOUNT_NUMBER,
    MT_BANK_CODE,
    MT_PRICE,
    MT_INVOICE_ID,
};

// data which are same for bank_ebanka_list and bank_statement_item
class Payment {
public:
    virtual const std::string &getAccountNumber() const = 0;
    virtual const std::string &getBankCode() const = 0;
    virtual const std::string &getConstSymbol() const = 0;
    virtual const std::string &getVarSymbol() const = 0;
    virtual const Database::Money &getPrice() const = 0;
    virtual const std::string &getMemo() const = 0;
    virtual const Database::ID &getInvoiceId() const = 0;
    virtual void setAccountNumber(std::string accountNumber) = 0;
    virtual void setBankCode(std::string bankCode) = 0;
    virtual void setConstSymbol(std::string constSymbol) = 0;
    virtual void setVarSymbol(std::string varSymbol) = 0;
    virtual void setPrice(Database::Money price) = 0;
    virtual void setMemo(std::string memo) = 0;
    virtual void setInvoiceId(Database::ID invoiceId) = 0;
};

// data from bank_ebanka_list
class OnlineStatement:
    virtual public Payment,
    virtual public Register::CommonObject {
public:
    virtual const Database::ID &getAccountId() const = 0;
    virtual const Database::DateTime &getCrDate() const = 0;
    virtual const std::string &getAccountName() const = 0;
    virtual const std::string &getIdent() const = 0;
    virtual void setAccountId(Database::ID accountId) = 0;
    virtual void setCrDate(Database::DateTime crDate) = 0;
    virtual void setCrDate(std::string crDate) = 0;
    virtual void setAccountName(std::string accountName) = 0;
    virtual void setIdent(std::string ident) = 0;
    virtual void setConn(Database::Connection *conn) = 0;
    virtual Database::Connection *getConn() const = 0;
    virtual void save() = 0;
};

// data from bank_statement_item table
class StatementItem:
    virtual public Payment {
public:
    virtual const Database::ID &getId() const = 0;
    virtual const Database::ID &getStatementId() const = 0;
    virtual const int getCode() const = 0;
    virtual const std::string &getEvidenceNumber() const = 0;
    virtual const Database::Date &getDate() const = 0;
    virtual const std::string &getAccountEvid() const = 0;
    virtual const std::string &getSpecSymbol() const = 0;
    virtual void setId(Database::ID id) = 0;
    virtual void setStatementId(Database::ID statementId) = 0;
    virtual void setCode(int code) = 0;
    virtual void setEvidenceNumber(std::string evidenceNumber) = 0;
    virtual void setDate(Database::Date date) = 0;
    virtual void setDate(std::string date) = 0;
    virtual void setAccountEvid(std::string accountEvid) = 0;
    virtual void setSpecSymbol(std::string specSymbol) = 0;
    virtual void setConn(Database::Connection *conn) = 0;
    virtual Database::Connection *getConn() const = 0;
    virtual void save() = 0;
};

// date from bank_statement_head table
class Statement:
    virtual public Register::CommonObject {
public:
    virtual const Database::ID &getAccountId() const = 0;
    virtual const int getNumber() const = 0;
    virtual const Database::Date &getDate() const = 0;
    virtual const Database::Date &getOldDate() const = 0;
    virtual const Database::Money &getBalance() const = 0;
    virtual const Database::Money &getOldBalance() const = 0;
    virtual const Database::Money &getCredit() const = 0;
    virtual const Database::Money &getDebet() const = 0;
    virtual void setAccountId(Database::ID accountId) = 0;
    virtual void setNumber(int number) = 0;
    virtual void setDate(Database::Date date) = 0;
    virtual void setDate(std::string date) = 0;
    virtual void setOldDate(Database::Date oldDate) = 0;
    virtual void setOldDate(std::string oldDate) = 0;
    virtual void setBalance(Database::Money balance) = 0;
    virtual void setOldBalance(Database::Money oldBalance) = 0;
    virtual void setCredit(Database::Money credit) = 0;
    virtual void setDebet(Database::Money debet) = 0;
    virtual unsigned int getStatementItemCount() const = 0;
    virtual const StatementItem *getStatementItemByIdx(unsigned int idx) const
        throw (NOT_FOUND) = 0;
    virtual void setConn(Database::Connection *conn) = 0;
    virtual Database::Connection *getConn() const = 0;
    virtual void save() = 0;
    virtual StatementItem *createStatementItem() = 0;
};

// list of online payments
class OnlineList:
    virtual public Register::CommonList {
public:
    virtual OnlineStatement *get(unsigned int index) const = 0;
    virtual void reload(Database::Filters::Union &filter) = 0;
    virtual void sort(MemberType member, bool asc) = 0;

    virtual const char *getTempTableName() const = 0;
    virtual void makeQuery(bool, bool, std::stringstream &) const = 0;
    virtual void reload() = 0;
    virtual bool exportXML(std::ostream &out) = 0;
};

// list of payments
class List:
    virtual public Register::CommonList {
public:
    virtual Statement *get(unsigned int index) const = 0;
    virtual void reload(Database::Filters::Union &filter) = 0;
    virtual void sort(MemberType member, bool asc) = 0;

    virtual const char *getTempTableName() const = 0;
    virtual void makeQuery(bool, bool, std::stringstream &) const = 0;
    virtual void reload() = 0;
    virtual bool exportXML(std::ostream &out) = 0;
};

// banking manager
class Manager {
public:
    virtual List *createList() const = 0;
    virtual OnlineList *createOnlineList() const = 0;
    static Manager *create(Database::Manager *dbMan);
    virtual OnlineStatement *createOnlineStatement() const = 0;
    virtual Statement *createStatement() const = 0;
    virtual bool importOnlineStatementXml(std::istream &in) = 0;
    virtual bool importStatementXml(std::istream &in) = 0;
};

} // namespace Bank
} // namespace Register

#endif // _BANK_H_
