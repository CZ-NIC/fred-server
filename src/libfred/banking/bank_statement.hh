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

#ifndef BANK_STATEMENT_HH_E638BA2322F743D08A9D72362EC8B508
#define BANK_STATEMENT_HH_E638BA2322F743D08A9D72362EC8B508

#include "src/libfred/exceptions.hh"
#include "src/libfred/common_new.hh"
#include "src/libfred/banking/bank_payment.hh"
#include "src/util/types/money.hh"

namespace LibFred {
namespace Banking {


/**
 * \class Statement
 * \brief Interface for bank statement object
 *
 */
class Statement : virtual public LibFred::CommonObjectNew
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
    virtual Money getBalanceOld() const = 0;
    virtual Money getBalanceNew() const = 0;
    virtual Money getBalanceCredit() const = 0;
    virtual Money getBalanceDebet() const = 0;
    virtual const unsigned long long &getFileId() const = 0;
    virtual void setId(const unsigned long long &id) = 0;
    virtual void setAccountId(const unsigned long long &accountId) = 0;
    virtual void setNum(const int &num) = 0;
    virtual void setCreateDate(const Database::Date &createDate) = 0;
    virtual void setBalanceOldDate(const Database::Date &balanceOldDate) = 0;
    virtual void setBalanceOld(const Money &balanceOld) = 0;
    virtual void setBalanceNew(const Money &balanceNew) = 0;
    virtual void setBalanceCredit(const Money &balanceCredit) = 0;
    virtual void setBalanceDebet(const Money &balanceDebet) = 0;
    virtual void setFileId(const unsigned long long &fileId) = 0;

    virtual unsigned int getPaymentCount() const = 0;
    virtual Payment* getPaymentByIdx(const unsigned long long _id) const = 0;

    virtual std::string toString() const = 0;
    virtual void save() = 0;
};

// smart pointer
typedef std::unique_ptr<Statement> StatementPtr;

}
}

#endif /*BANK_STATEMENT_H_*/

