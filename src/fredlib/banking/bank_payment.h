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

#ifndef BANK_PAYMENT_H_
#define BANK_PAYMENT_H_

#include "exceptions.h"
#include "common_new.h"
#include "types/money.h"

namespace Fred {
namespace Banking {

/**
 * \class Payment
 * \brief Interface for bank payment object
 *
 */
class Payment : virtual public Fred::CommonObjectNew
{
public:
    virtual ~Payment()
    {
    }

    virtual const unsigned long long &getId() const = 0;
    virtual const unsigned long long &getStatementId() const = 0;
    virtual const unsigned long long &getAccountId() const = 0;
    virtual const std::string &getAccountNumber() const = 0;
    virtual const std::string &getBankCode() const = 0;
    virtual const int &getCode() const = 0;
    virtual const int &getType() const = 0;
    virtual const int &getStatus() const = 0;
    virtual const std::string &getKonstSym() const = 0;
    virtual const std::string &getVarSymb() const = 0;
    virtual const std::string &getSpecSymb() const = 0;
    virtual Money getPrice() const = 0;
    virtual const std::string &getAccountEvid() const = 0;
    virtual const Database::Date &getAccountDate() const = 0;
    virtual const std::string &getAccountMemo() const = 0;
    virtual const unsigned long long &getAdvanceInvoiceId() const = 0;
    virtual const std::string &getAccountName() const = 0;
    virtual const Database::DateTime &getCrTime() const = 0;
    virtual const std::string &getDestAccount() const = 0;
    virtual void setId(const unsigned long long &id) = 0;
    virtual void setStatementId(const unsigned long long &statementId) = 0;
    virtual void setAccountId(const unsigned long long &accountId) = 0;
    virtual void setAccountNumber(const std::string &accountNumber) = 0;
    virtual void setBankCode(const std::string &bankCodeId) = 0;
    virtual void setCode(const int &code) = 0;
    virtual void setType(const int &type) = 0;
    virtual void setStatus(const int &status) = 0;
    virtual void setKonstSym(const std::string &konstSym) = 0;
    virtual void setVarSymb(const std::string &varSymb) = 0;
    virtual void setSpecSymb(const std::string &specSymb) = 0;
    virtual void setPrice(const Money &price) = 0;
    virtual void setAccountEvid(const std::string &accountEvid) = 0;
    virtual void setAccountDate(const Database::Date &accountDate) = 0;
    virtual void setAccountMemo(const std::string &accountMemo) = 0;
    virtual void setAdvanceInvoiceId(const unsigned long long &invoiceId) = 0;
    virtual void setAccountName(const std::string &accountName) = 0;
    virtual void setCrTime(const Database::DateTime &crTime) = 0;
    virtual void setDestAccount(const std::string &destAccount) = 0;

    virtual unsigned long long getInvoicePrefix() const = 0;
    virtual void setInvoicePrefix(const unsigned long long &_iprefix) = 0;

    virtual std::string toString() const = 0;
    virtual void save() throw (SQL_ERROR) = 0;
    virtual void reload() throw (SQL_ERROR, NOT_FOUND) = 0;
};//class Payment

// smart pointer
typedef std::auto_ptr<Payment> PaymentPtr;

struct EnumListItem
{
  unsigned long long id;
  std::string name;
};
typedef std::vector<EnumListItem> EnumList;

//status names
EnumList getBankAccounts();

}//namespace Banking
}//namespace Fred

#endif /*BANK_PAYMENT_H_*/

