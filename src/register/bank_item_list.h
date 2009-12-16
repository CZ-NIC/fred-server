#ifndef _BANK_ITEM_LIST_H_
#define _BANK_ITEM_LIST_H_

#include "bank_item.h"
#include "common_impl_new.h"
#include "model/model_filters.h"

namespace Register {
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
    IMT_CREATE_TIME
};

class ItemList:
    virtual public Register::CommonListImplNew {
public:
    StatementItem *get(const unsigned int &index) const throw (std::exception);
    StatementItem *getById(const unsigned long long &id) const;
    void reload(Database::Filters::Union &filter);
    void sort(ItemMemberType member, bool asc);
    const char *getTempTableName() const;
};

} // namespace Banking
} // namespace Register

#endif // _BANK_ITEM_LIST_H_
