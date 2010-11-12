#ifndef _BANK_EXPORTER_H_
#define _BANK_EXPORTER_H_

#include <ostream>
#include "bank_common.h"

namespace Fred {
namespace Banking {

class Statement;
class Payment;

class Exporter {
public:
    virtual ~Exporter()
    { }
    virtual void doExport(Statement *_data) = 0;
};

class ExporterXML:
    public Exporter {
private:
    XMLcreator          m_xml;
    std::ostream        &m_out;
    void doExport(const Payment *_data);
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
    void doExport(Statement *_data);
};

} // namespace Banking
} // namespace Fred

#endif // _BANK_EXPORTER_H_
