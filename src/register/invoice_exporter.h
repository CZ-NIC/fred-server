#ifndef _INVOICE_EXPORTER_H_
#define _INVOICE_EXPORTER_H_

#define TAGSTART(tag) "<"#tag">"
#define TAGEND(tag) "</"#tag">"

#include <ostream>
#include "documents.h"

namespace Register {
namespace Invoicing {

class Invoice;
class Subject;

class Exporter {
public:
    virtual ~Exporter()
    { }
    virtual void doExport(Invoice *) = 0;
};

class ExporterXML:
    public Exporter {
private:
    std::ostream    &m_out;
    bool            m_xmlDecl;
    std::ostream &doExport(const Subject *subj);
public:
    ExporterXML(std::ostream &out, bool xmlDecl):
        Exporter(),
        m_out(out), m_xmlDecl(xmlDecl)
    { }
    void doExport(Invoice *);
};

class ExporterArchiver:
    public Exporter {
private:
    Document::Manager *m_docMan;
    std::string makeFileName(Invoice *inv, const std::string &suffix);
public:
    ExporterArchiver(Document::Manager *docMan):
        Exporter(), 
        m_docMan(docMan)
    { }
    void doExport(Invoice *);
};

} // namespace Invoicing
} // namespace Register

#endif // _INVOICE_EXPORTER_H_
