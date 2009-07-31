#ifndef _BANK_EXPORTER_H_
#define _BANK_EXPORTER_H_

#include <ostream>
#include "bank_common.h"

namespace Register {
namespace Banking {

class StatementHead;
class StatementItem;

class Exporter {
public:
    virtual ~Exporter()
    { }
    virtual void doExport(StatementHead *) = 0;
};

class ExporterXML:
    public Exporter {
private:
    XMLcreator          m_xml;
    std::ostream        &m_out;
    void doExport(const StatementItem *item);
    void finalize();
public:
    ExporterXML(std::ostream &out):
        Exporter(),
        m_out(out)
    { 
        m_xml.init();
        m_xml.start(STATEMENTS_ROOT);
    }
    ~ExporterXML();
    void doExport(StatementHead *);
};

} // namespace Banking
} // namespace Register

#endif // _BANK_EXPORTER_H_
