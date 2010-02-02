/*
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BANK_STATEMENT_H_
#define BANK_STATEMENT_H_

#include "exceptions.h"
#include "common_new.h"
#include "bank_payment.h"

namespace Register {
namespace Banking {


/**
 * \class Statement
 * \brief Interface for bank statement object
 *
 */
class Statement : virtual public Register::CommonObjectNew
{
public:
    virtual ~Statement()
    {
    }

    virtual const unsigned long long &getId() const = 0;
    virtual const unsigned long long &getAccountId() const = 0;
    virtual const int &getNum() const = 0;
    virtual const Database::Date &getCreateDate() const = 0;
    virtual const Database::Date &getBalanceOldDate() const = 0;
    virtual const Database::Money &getBalanceOld() const = 0;
    virtual const Database::Money &getBalanceNew() const = 0;
    virtual const Database::Money &getBalanceCredit() const = 0;
    virtual const Database::Money &getBalanceDebet() const = 0;
    virtual const unsigned long long &getFileId() const = 0;
    virtual void setId(const unsigned long long &id) = 0;
    virtual void setAccountId(const unsigned long long &accountId) = 0;
    virtual void setNum(const int &num) = 0;
    virtual void setCreateDate(const Database::Date &createDate) = 0;
    virtual void setBalanceOldDate(const Database::Date &balanceOldDate) = 0;
    virtual void setBalanceOld(const Database::Money &balanceOld) = 0;
    virtual void setBalanceNew(const Database::Money &balanceNew) = 0;
    virtual void setBalanceCredit(const Database::Money &balanceCredit) = 0;
    virtual void setBalanceDebet(const Database::Money &balanceDebet) = 0;
    virtual void setFileId(const unsigned long long &fileId) = 0;

    virtual unsigned int getPaymentCount() const = 0;
    virtual Payment* getPaymentByIdx(const unsigned long long _id) const = 0;

    virtual std::string toString() const = 0;
    virtual void save() throw (SQL_ERROR) = 0;
};

// smart pointer
typedef std::auto_ptr<Statement> StatementPtr;

}
}

#endif /*BANK_STATEMENT_H_*/

