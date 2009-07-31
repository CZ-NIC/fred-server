#ifndef _INVOICE_LIST_H_
#define _INVOICE_LIST_H_

#include "invoice.h"
#include "invoice_exporter.h"
#include "common_impl_new.h"
#include "db_settings.h"
#include "model/model_filters.h"

namespace Register {
namespace Invoicing {

enum MemberType {
    MT_ZONE,
    MT_CRTIME,
    MT_TAXDATE,
    MT_TODATE,
    MT_FROMDATE,
    MT_NUMBER,
    MT_REGISTRAR,
    MT_CREDIT,
    MT_PRICE,
    MT_VARSYMBOL,
    MT_FILEPDF,
    MT_FILEXML,
    MT_TYPE,
    MT_TOTAL,
};

//-----------------------------------------------------------------------------
//
// List
//
//-----------------------------------------------------------------------------
class List:
    virtual public Register::CommonListImplNew {
private:
    Manager *m_manager;
    bool    m_partialLoad;
public:
    List(Manager *manager = NULL):
        m_manager(manager),
        m_partialLoad(false)
    { }
    void reload(Database::Filters::Union &filter);
    Invoice *get(unsigned int index) const throw (std::exception);
    Invoice *getById(unsigned long long id) const;
    const char* getTempTableName() const;
    void sort(MemberType member, bool asc);
    void setPartialLoad(bool partialLoad);
    void exportXML(std::ostream &out);
    void doExport(Exporter *exp);
}; // class List

} // namespace Invoicing
} // namespace Register

#endif // _INVOICE_LIST_H_
