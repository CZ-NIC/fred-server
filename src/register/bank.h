#ifndef _BANKING_H_
#define _BANKING_H_
#include "invoice.h"

namespace Register
{
  namespace Banking 
  {
    class Payment
    {
     protected:
      /// protected destructor
      virtual ~Payment() {}
     public:
      /// unique identification of record
      virtual TID getId() const = 0;
      /// account number of other side
      virtual const std::string& getAccountNumber() const = 0;
      /// bank of account of other side
      virtual const std::string& getAccountBankCode() const = 0;
      /// constant symbol of payment
      virtual const std::string& getConstSymbol() const = 0;
      /// variable symbol of payment
      virtual const std::string& getVarSymbol() const = 0;
      /// specific symbol of payment
      virtual const std::string& getSpecSymbol() const = 0;
      /// transfering amount
      virtual Invoicing::Money getPrice() const = 0;
      /// note
      virtual const std::string& getMemo() const = 0;
      /// id of invoice if matching registrar was found
      virtual TID getInvoiceId() const = 0;
    };
    /// payment downloaded online 
    class OnlinePayment : virtual public Payment 
    {      
     protected:
      /// protected destructor
      virtual ~OnlinePayment() {}
     public:
      /// id of our account identification 
      virtual TID getAccountId() const = 0;
      /// timestamp of record creation
      virtual boost::posix_time::ptime getCrDate() const = 0;
      /// name of account of other side
      virtual const std::string& getAccountName() const = 0;
      /// unique identification of record on bank side
      virtual const std::string& getIdent() const = 0;
    };
    /// list of online payments
    class OnlinePaymentList 
    {
     public:
      /// public destructor
      virtual ~OnlinePaymentList() {}
      /// reload statements with selected filter
      virtual void reload() throw (SQL_ERROR) = 0;
      /// return count of statements in list 
      virtual unsigned long getCount() const = 0;
      /// return statement by index
      virtual OnlinePayment *get(unsigned idx) const = 0;
      /// clear filter settings
      virtual void clearFilter() = 0;            
      /// export in XML
      virtual void exportXML(std::ostream& out) = 0;
    };
    /// one item on the bank statement
    class StatementItem : virtual public Payment
    {
     protected:
      /// protected destructor
      virtual ~StatementItem() {}
     public:
      /// Types of operations 
      enum Codes {
        C_CREDIT, //< incoming money 
        C_DEBET, //< outgoing money
        C_CREDIT_STORNO, //< storno of incoming money
        C_DEBET_STORNO //< storno of outgoing money
      };
      /// code of operation
      virtual Codes getCode() const = 0;
      /// evidence number associated to payment
      virtual const std::string& getEvidenceNumber() const = 0;
      /// date of accepting payment
      virtual boost::gregorian::date getDate() const = 0;
    };
    /// bank statement for given period of time 
    class Statement
    {
     protected:
      /// protected destructor
      virtual ~Statement() {}
     public:
      /// unique identification of record
      virtual TID getId() const = 0;
      /// id of our account the statement is for 
      virtual TID getAccountId() const = 0;
      /// increasing number of statement
      virtual unsigned getNumber() const = 0;
      /// date of checking balance
      virtual boost::gregorian::date getDate() const = 0;
      /// balance in date
      virtual Invoicing::Money getBalance() const = 0;
      /// date of last statement
      virtual boost::gregorian::date getOldDate() const = 0;
      /// balance in oldDate
      virtual Invoicing::Money getOldBalance() const = 0;
      /// summarize all credits on statement
      virtual Invoicing::Money getCredit() const = 0;
      /// summarize all debets on statement 
      virtual Invoicing::Money getDebet() const = 0;
      /// number of items in statment
      virtual unsigned getItemsCount() const = 0;
      /// get item by index into list od items
      virtual StatementItem *getItem(unsigned idx) const = 0;
    };
    /// list of statements conforming to specified filter   
    class StatementList
    {
     public:
      /// public destructor
      virtual ~StatementList() {}
      /// reload statements with selected filter
      virtual void reload() throw (SQL_ERROR) = 0;
      /// return count of statements in list 
      virtual unsigned long getCount() const = 0;
      /// return statement by index
      virtual Statement *get(unsigned idx) const = 0;
      /// clear filter settings
      virtual void clearFilter() = 0;            
      /// export in XML
      virtual void exportXML(std::ostream& out) = 0;
    };
    /// facade of banking subsystem
    class Manager
    {
     public:
      /// public destructor - client is responsible for destroying manager
      virtual ~Manager() {}
      /// create list of statements
      virtual StatementList *createStatementList() const = 0;
      /// create list of online payments
      virtual OnlinePaymentList *createOnlinePaymentList() const = 0;
      /// factory method
      static Manager *create(DB *db);      
    };    
  }
}

#endif // _BANKING_H_
