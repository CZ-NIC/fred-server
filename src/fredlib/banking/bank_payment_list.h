#ifndef BANK_PAYMENT_LIST_H_
#define BANK_PAYMENT_LIST_H_

#include "common_new.h"
#include "bank_payment.h"
#include "model/model_filters.h"

namespace Fred {
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

class PaymentList : virtual public Fred::CommonListNew
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
typedef std::auto_ptr<PaymentList> PaymentListPtr;

}
}


#endif /*BANK_PAYMENT_LIST_H_*/

