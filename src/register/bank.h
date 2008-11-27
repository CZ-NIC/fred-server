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
#include "db/manager.h"
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
    virtual const std::string &getSpecSymbol() const = 0;
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
    virtual unsigned int getStatementItemCount() const = 0;
    virtual const StatementItem *getStatementItemByIdx(unsigned int idx) const
        throw (NOT_FOUND) = 0;
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
    virtual void exportXML(std::ostream &out) = 0;
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
    virtual void exportXML(std::ostream &out) = 0;
};

// banking manager
class Manager {
public:
    virtual List *createList() const = 0;
    virtual OnlineList *createOnlineList() const = 0;
    static Manager *create(Database::Manager *dbMan);
};

} // namespace Bank
} // namespace Register

#endif // _BANK_H_
