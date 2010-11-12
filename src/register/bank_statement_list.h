#ifndef BANK_STATEMENT_LIST_H_
#define BANK_STATEMENT_LIST_H_

#include "common_new.h"
#include "bank_statement.h"
#include "model/model_filters.h"

namespace Fred {
namespace Banking {


enum MemberType {
    MT_ACCOUNT_ID,
    MT_NUM,
    MT_CREATE_DATE,
    MT_BALANCE_OLD_DATE,
    MT_BALANCE_OLD,
    MT_BALANCE_NEW,
    MT_BALANCE_CREDIT,
    MT_BALANCE_DEBET,
    MT_FILE_ID,
    MT_ZONE_FQDN,
};


class StatementList : virtual public Fred::CommonListNew
{
public:
    virtual ~StatementList()
    {
    }

    virtual Statement *get(const unsigned int &index) const = 0; 
    virtual Statement *getById(const unsigned long long &id) const = 0;
    virtual void reload(Database::Filters::Union &filter) = 0;
    virtual void sort(MemberType member, bool asc) = 0;
    virtual const char *getTempTableName() const = 0;

};

// smart pointert
typedef std::auto_ptr<StatementList> StatementListPtr;

}
}

#endif /*BANK_STATEMENT_LIST_H_*/

