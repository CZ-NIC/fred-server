#ifndef BANK_PAYMENT_LIST_H_
#define BANK_PAYMENT_LIST_H_

#include "src/libfred/common_new.hh"
#include "src/libfred/banking/bank_payment.hh"
#include "src/deprecated/model/model_filters.hh"

namespace LibFred {
namespace Banking {

enum ItemMemberType {
    IMT_ID,
    IMT_STATEMENT_ID,
    IMT_ACCOUNT_NUMBER,
    IMT_BANK_CODE,
    IMT_TYPE,
    IMT_CODE,
    IMT_CONSTSYMB,
    IMT_VARSYMB,
    IMT_SPECSYMB,
    IMT_PRICE,
    IMT_ACCOUNT_EVID,
    IMT_ACCOUNT_DATE,
    IMT_ACCOUNT_MEMO,
    IMT_INVOICE,
    IMT_ACCOUNT_NAME,
    IMT_CREATE_TIME,
    IMT_DEST_ACCOUNT
};

class PaymentList : virtual public LibFred::CommonListNew
{
public:
    virtual ~PaymentList()
    {
    }

    virtual Payment *get(const unsigned int &index) const = 0; 
    virtual Payment *getById(const unsigned long long &id) const = 0;
    virtual void reload(Database::Filters::Union &filter) = 0;
    virtual void sort(ItemMemberType member, bool asc) = 0;
    virtual const char *getTempTableName() const = 0;

};

// smart pointer
typedef std::unique_ptr<PaymentList> PaymentListPtr;

}
}


#endif /*BANK_PAYMENT_LIST_H_*/

