#ifndef BANK_EXPORTER_HH_D1F1B4C8F68846B697FA0A792D29C944
#define BANK_EXPORTER_HH_D1F1B4C8F68846B697FA0A792D29C944

#include <ostream>
#include "src/libfred/banking/bank_common.hh"

namespace LibFred {
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
} // namespace LibFred

#endif
