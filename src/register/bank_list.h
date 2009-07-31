#ifndef _BANK_LIST_H_
#define _BANK_LIST_H_

#include "bank_head.h"
#include "common_impl_new.h"
#include "model/model_filters.h"
#include "bank_exporter.h"

namespace Register {
namespace Banking {

enum MemberType {
    MT_CREATE_DATE,
    MT_BALANCE_OLD_DATE,
    MT_BALANCE_OLD,
    MT_BALANCE_NEW,
    MT_BALANCE_CREDIT,
    MT_BALANCE_DEBET,
    MT_ZONE_FQDN,
    // MT_ACCOUNT_NUMBER,
    // MT_BANK_CODE,
};

class List:
    virtual public Register::CommonListImplNew {
private:
    void doExport(Exporter *exp);
public:
    StatementHead *get(const unsigned int &index) const throw (std::exception);
    StatementHead *getById(const unsigned long long &id) const;
    void reload(Database::Filters::Union &filter);
    void sort(MemberType member, bool asc);
    const char *getTempTableName() const;
    bool exportXML(std::ostream &out);
};

} // namespace Banking
} // namespace Register

#endif // _BANK_LIST_H_
