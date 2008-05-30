/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

#include "bank.h"
#include "old_utils/dbsql.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define MAKE_TIME_DEF(ROW,COL,DEF)  \
  (boost::posix_time::ptime(db->IsNotNull(ROW,COL) ? \
  boost::posix_time::time_from_string(\
  db->GetFieldValue(ROW,COL)) : DEF))         
#define MAKE_TIME(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,boost::posix_time::not_a_date_time)         
#define MAKE_TIME_NEG(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,boost::posix_time::neg_infin)         
#define MAKE_TIME_POS(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,boost::posix_time::pos_infin)         
#define MAKE_DATE(ROW,COL)  \
 (boost::gregorian::date(db->IsNotNull(ROW,COL) ? \
 boost::gregorian::from_string(\
 db->GetFieldValue(ROW,COL)) : \
 (boost::gregorian::date)boost::gregorian::not_a_date_time))

#define STR_TO_MONEY(x) atol(x)

namespace Register
{
  namespace Banking
  {
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //    PaymentImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class PaymentImpl : virtual public Payment
    {
      TID id;
      std::string accountNumber;
      std::string accountBankCode;
      std::string constSymbol;
      std::string varSymbol;
      std::string specSymbol;
      Invoicing::Money price;
      std::string memo;
      TID invoiceId;
     public:
      PaymentImpl() :
        id(0), price(0), invoiceId(0)
      {}
      PaymentImpl(DB *db, unsigned l) :
        id(STR_TO_ID(db->GetFieldValue(l,0))),
        accountNumber(db->GetFieldValue(l,1)),
        accountBankCode(db->GetFieldValue(l,2)),
        constSymbol(db->GetFieldValue(l,3)),
        varSymbol(db->GetFieldValue(l,4)),
        specSymbol(db->GetFieldValue(l,5)),
        price(STR_TO_MONEY(db->GetFieldValue(l,6))),
        memo(db->GetFieldValue(l,7)),
        invoiceId(STR_TO_ID(db->GetFieldValue(l,8)))
      {}
      virtual TID getId() const
      {
        return id;
      }
      virtual const std::string& getAccountNumber() const
      {
        return accountNumber;
      }
      virtual const std::string& getAccountBankCode() const
      {
        return accountBankCode;
      }
      virtual const std::string& getConstSymbol() const
      {
        return constSymbol;
      }
      virtual const std::string& getVarSymbol() const
      {
        return varSymbol;
      }
      virtual const std::string& getSpecSymbol() const
      {
        return specSymbol;
      }
      virtual Invoicing::Money getPrice() const
      {
        return price;
      }
      virtual const std::string& getMemo() const
      {
        return memo;
      }
      virtual TID getInvoiceId() const
      {
        return invoiceId;
      } 
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //    OnlinePaymentImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class OnlinePaymentImpl : virtual public OnlinePayment, public PaymentImpl
    {
      TID accountId;
      boost::posix_time::ptime crDate;
      std::string accountName;
      std::string ident;
     public:
      OnlinePaymentImpl() :
        PaymentImpl(), accountId(0)
      {}
      OnlinePaymentImpl(DB *db, unsigned l) :
        PaymentImpl(db,l), 
        accountId(STR_TO_ID(db->GetFieldValue(l,9))),
        crDate(MAKE_TIME(l,10)),
        accountName(db->GetFieldValue(l,11)),
        ident(db->GetFieldValue(l,12))
      {}
      virtual TID getAccountId() const
      {
        return accountId;
      }
      virtual boost::posix_time::ptime getCrDate() const
      {
        return crDate;
      }
      virtual const std::string& getAccountName() const
      {
        return accountName;
      }
      virtual const std::string& getIdent() const
      {
        return ident;
      }      
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //    OnlinePaymentListImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class OnlinePaymentListImpl : virtual public OnlinePaymentList 
    {
      typedef std::vector<OnlinePaymentImpl *> ListType;
      ListType payments;
      DB *db;
      void clear()
      {
        for (unsigned i=0; i<payments.size(); i++)
          delete payments[i];
        payments.clear();
      }
     public:
      OnlinePaymentListImpl(DB *_db) :
        db(_db)
      {}
      ~OnlinePaymentListImpl()
      {
        clear();
      }
      virtual void reload() throw (SQL_ERROR)
      {
        clear();
        std::stringstream sql;
        sql << "SELECT "
            << "id, account_number, bank_code, konstsym, varsymb, '', "
            << "price * 100, memo, invoice_id, "
            << "account_id, crdate, name, ident "
            << "FROM bank_ebanka_list";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
          payments.push_back(new OnlinePaymentImpl(db,i));
        db->FreeSelect();
      }
      virtual unsigned long getCount() const
      {
        return payments.size();
      }
      virtual OnlinePayment *get(unsigned idx) const
      {
        return payments.size() < idx ? payments[idx] : NULL;
      }
      virtual void clearFilter()
      {
      }
      #define TAGSTART(tag) "<"#tag">"
      #define TAGEND(tag) "</"#tag">"
      #define TAG(tag,f) TAGSTART(tag) << f << TAGEND(tag)
      #define OUTMONEY(f) (f)/100 << "." << \
                          std::setfill('0') << std::setw(2) << (f)%100      
      virtual void exportXML(std::ostream& out)
      {
        out << TAGSTART(online_payments);
        for (unsigned i=0; i<payments.size(); i++) {
          OnlinePayment *p = payments[i];
          out << TAGSTART(online_payment)
              << TAG(id,p->getId())
              << TAG(accout_number,p->getAccountNumber())
              << TAG(accout_bank_code,p->getAccountBankCode())
              << TAG(const_symbol,p->getConstSymbol())
              << TAG(var_symbol,p->getVarSymbol())
              << TAG(spec_symbol,p->getSpecSymbol())
              << TAG(price,OUTMONEY(p->getPrice()))
              << TAG(memo,p->getMemo())
              << TAG(invoice_id,p->getInvoiceId())
              << TAG(accout_id,p->getAccountId())
              << TAG(cr_date,p->getCrDate())
              << TAG(accout_name,p->getAccountName())
              << TAG(ident,p->getIdent())
              << TAGEND(online_payment);
        }
        out << TAGEND(online_payments);
      }      
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //    StatementItemImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class StatementItemImpl : virtual public StatementItem, public PaymentImpl
    {
      Codes code;
      std::string evidenceNumber;
      boost::gregorian::date date;
      Codes codeFromInt(unsigned i)
      {
        switch (i) {
          case 1: return C_CREDIT;
          case 2: return C_DEBET;
          case 3: return C_CREDIT_STORNO;
          case 4: return C_DEBET_STORNO;
          default : 
           //TODO: log error what to do?
           return C_CREDIT;
        };
      }
     public:
      StatementItemImpl() :
        code(C_CREDIT)
      {}
      StatementItemImpl(DB* db, unsigned l) :
        PaymentImpl(db,l),
        code(codeFromInt(atoi(db->GetFieldValue(l,9)))),
        evidenceNumber(db->GetFieldValue(l,10)),
        date(MAKE_DATE(l,11))
      {}
      virtual Codes getCode() const
      {
        return code;
      }
      virtual const std::string& getEvidenceNumber() const
      {
        return evidenceNumber;
      }
      virtual boost::gregorian::date getDate() const
      {
        return date;
      }
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //    StatementImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class StatementImpl : virtual public Statement
    {
      TID id;
      TID accountId;
      unsigned number;
      boost::gregorian::date date;
      Invoicing::Money balance;
      boost::gregorian::date oldDate;
      Invoicing::Money oldBalance;
      Invoicing::Money credit;
      Invoicing::Money debet;
      typedef std::vector<StatementItemImpl *> ItemList;
      ItemList items;
      void clear()
      {
        for (unsigned i=0; i<items.size(); i++)
          delete items[i];
        items.clear();
      }
     public:
      StatementImpl() : 
        id(0), accountId(0), number(0), balance(0), oldBalance(0), 
        credit(0), debet(0)
      {}
      StatementImpl(DB* db, unsigned l) :
        id(STR_TO_ID(db->GetFieldValue(l,0))),
        accountId(STR_TO_ID(db->GetFieldValue(l,1))),
        number(atoi(db->GetFieldValue(l,2))),
        date(MAKE_DATE(l,3)),
        balance(STR_TO_MONEY(db->GetFieldValue(l,4))),
        oldDate(MAKE_DATE(l,5)),
        oldBalance(STR_TO_MONEY(db->GetFieldValue(l,6))),
        credit(STR_TO_MONEY(db->GetFieldValue(l,7))),
        debet(STR_TO_MONEY(db->GetFieldValue(l,8)))
      {}
      ~StatementImpl()
      {
        clear();
      }
      virtual TID getId() const
      {
        return id;
      }
      virtual TID getAccountId() const
      {
        return accountId;
      }
      virtual unsigned getNumber() const
      {
        return number;
      }
      virtual boost::gregorian::date getDate() const
      {
        return date;
      }
      virtual Invoicing::Money getBalance() const
      {
        return balance;
      }
      virtual boost::gregorian::date getOldDate() const
      {
        return oldDate;
      }
      virtual Invoicing::Money getOldBalance() const
      {
        return oldBalance;
      }
      virtual Invoicing::Money getCredit() const
      {
        return credit;
      }
      virtual Invoicing::Money getDebet() const
      {
        return debet;
      }
      virtual unsigned getItemsCount() const
      {
        return items.size();
      }
      virtual StatementItem *getItem(unsigned idx) const
      {
        return idx < items.size() ? items[idx] : NULL;
      }
      void addItem(DB* db, unsigned l)
      {
        items.push_back(new StatementItemImpl(db,l));
      }
      bool hasId(TID _id) const
      {
        return id == _id;
      }
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   StatementListImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class StatementListImpl : virtual public StatementList
    {
      typedef std::vector<StatementImpl *> ListType;
      ListType statements;
      DB *db;
      void clear()
      {
        for (unsigned i=0; i<statements.size(); i++)
          delete statements[i];
        statements.clear();
      }
     public:
      StatementListImpl(DB *_db) :
        db(_db)
      {}
      ~StatementListImpl()
      {
        clear();
      }
      virtual void reload() throw (SQL_ERROR)
      {
        clear();
        std::stringstream sql;
        sql << "SELECT "
            << "id, account_id, num, create_date, balance_old * 100, "
            << "balance_old_date, balance_new * 100, "
            << "balance_credit * 100, balance_debet * 100 "
            << "FROM bank_statement_head";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
          statements.push_back(new StatementImpl(db,i));
        db->FreeSelect();
        sql.str("");
        sql << "SELECT "
            << "id, account_number, bank_code, konstsym, varsymb, specsymb, "
            << "price * 100, account_memo, invoice_id, "
            << "code, account_evid, account_date, "
            << "statement_id "
            << "FROM bank_statement_item";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          StatementImpl *si = findById(STR_TO_ID(db->GetFieldValue(i,12)));
          if (si) si->addItem(db,i);
        }
        db->FreeSelect();
      }
      virtual unsigned long getCount() const
      {
        return statements.size();
      }
      virtual Statement *get(unsigned idx) const
      {
        return statements.size() < idx ? statements[idx] : NULL;
      }
      virtual StatementImpl *findById(TID id) const
      {
        ListType::const_iterator i = find_if(
          statements.begin(), statements.end(),
          std::bind2nd(std::mem_fun(&StatementImpl::hasId),id)
        );
        return i != statements.end() ? *i : NULL;
      }
      virtual void clearFilter()
      {
      }
      virtual void exportXML(std::ostream& out)
      {
        out << TAGSTART(statements);
        for (unsigned i=0; i<statements.size(); i++) {
          Statement *s = statements[i];
          out << TAGSTART(statement)
              << TAG(id,s->getId())
              << TAG(accout_id,s->getAccountId())
              << TAG(number,s->getNumber())
              << TAG(date,s->getDate())
              << TAG(balance,OUTMONEY(s->getBalance()))
              << TAG(old_date,s->getOldDate())
              << TAG(oldBalance,OUTMONEY(s->getOldBalance()))
              << TAG(credit,OUTMONEY(s->getCredit()))
              << TAG(debet,OUTMONEY(s->getDebet()))
              << TAGSTART(items);
          for (unsigned k=0; k<s->getItemsCount(); k++) {
            StatementItem *i = s->getItem(k);      
            out << TAGSTART(item)
                << TAG(id,i->getId())
                << TAG(accout_number,i->getAccountNumber())
                << TAG(accout_bank_code,i->getAccountBankCode())
                << TAG(const_symbol,i->getConstSymbol())
                << TAG(var_symbol,i->getVarSymbol())
                << TAG(spec_symbol,i->getSpecSymbol())
                << TAG(price,OUTMONEY(i->getPrice()))
                << TAG(memo,i->getMemo())
                << TAG(invoice_id,i->getInvoiceId())
                << TAG(code,i->getCode())
                << TAG(evidence_number,i->getEvidenceNumber())
                << TAG(date,i->getDate())
                << TAGEND(item);
          }
          out << TAGEND(items)  
              << TAGEND(statement);
        }
        out << TAGEND(statements);
      }      
       
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   ManagerImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    /// implementation for Manager interface
    class ManagerImpl : virtual public Manager
    {
      DB *db;
     public:
      ManagerImpl(
        DB *_db
      ) : db(_db)
      {}
      /// create empty list of statements      
      virtual StatementList* createStatementList() const
      {
        return new StatementListImpl(db);
      }
      /// create empty list of statements      
      virtual OnlinePaymentList* createOnlinePaymentList() const
      {
        return new OnlinePaymentListImpl(db);
      }
    }; // ManagerImpl
    Manager *Manager::create(
      DB *db
    )
    {
      return new ManagerImpl(db);
    }    
  }; // Invoicing
}; // Register
