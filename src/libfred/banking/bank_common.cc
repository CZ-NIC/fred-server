#include "src/libfred/banking/bank_common.hh"

#include "src/libfred/banking/bank_payment.hh"
#include "src/libfred/db_settings.hh"
#include "src/util/types/stringify.hh"
#include "src/util/types/money.hh"

#include <boost/date_time/posix_time/ptime.hpp>

#include <utility>

namespace LibFred {
namespace Banking {

static std::string ltrim(const std::string &_input)
{
    std::string output(_input);
    std::string::size_type i = 0;
    i = output.find_first_not_of(" ", i);
    output.erase(0, i);
    return output;
}

static std::string rtrim(const std::string &_input)
{
    std::string output(_input);
    std::string::size_type i = output.npos;
    i = output.find_last_not_of(" ", i);
    output.erase(i + 1, output.size());
    return output;
}

static std::string trim(const std::string &_input)
{
    return ltrim(rtrim(_input));
}



std::string
loadInStream(std::istream &in)
{
    char ch;
    std::stringstream ss;
    while (!in.eof()) {
        in.get(ch);
        ss << ch;
    }
    return ss.str();
}

XMLcreator::~XMLcreator()
{
    if (m_writer != NULL) {
        xmlFreeTextWriter(m_writer);
    }
    xmlBufferFree(m_buffer);
}
bool 
XMLcreator::init(bool writeXmlHeader)
{
    m_writeXmlHeader = writeXmlHeader;
    m_buffer = xmlBufferCreate();
    if (m_buffer == NULL) {
        return false;
    }
    m_writer = xmlNewTextWriterMemory(m_buffer, 0);
    if (m_writer == NULL) {
        return false;
    }
    if (m_writeXmlHeader) {
        xmlTextWriterStartDocument(m_writer, NULL, NULL, NULL);
    }
    return true;
}
void 
XMLcreator::start(const std::string &name)
{
    if (xmlTextWriterStartElement(
                m_writer, (const xmlChar *)name.c_str()) < 0) {
        throw std::exception();
    }
}
void 
XMLcreator::start(const char *name)
{
    if (xmlTextWriterStartElement(
                m_writer, (const xmlChar *)name) < 0) {
        throw std::exception();
    }
}
void 
XMLcreator::end()
{
    if (xmlTextWriterEndElement(m_writer) < 0) {
        throw std::exception();
    }
}
void 
XMLcreator::text(const std::string &name, const std::string &value)
{
    if (xmlTextWriterWriteFormatElement(
                m_writer, (const xmlChar *)name.c_str(), "%s", value.c_str()) < 0) {
        throw std::exception();
    }
}
void 
XMLcreator::text(const std::string &name, const char *value)
{
    if (xmlTextWriterWriteFormatElement(
                m_writer, (const xmlChar *)name.c_str(), "%s", value) < 0) {
        throw std::exception();
    }
}
void 
XMLcreator::text(const std::string &name, int value)
{
    if (xmlTextWriterWriteFormatElement(
                m_writer, (const xmlChar *)name.c_str(), "%d", value) < 0) {
        throw std::exception();
    }
}
void
XMLcreator::text(const std::string &name, unsigned long long value)
{
    if (xmlTextWriterWriteFormatElement(
                m_writer, (const xmlChar *)name.c_str(), "%lld", value) < 0) {
        throw std::exception();
    }
}
void 
XMLcreator::text(const std::string &name, double value)
{
    if (xmlTextWriterWriteFormatElement(
                m_writer, (const xmlChar *)name.c_str(), "%lf", value) < 0) {
        throw std::exception();
    }
}
void 
XMLcreator::text(const std::string &name, Database::ID value)
{
    this->text(name, stringify(value));
}
void 
XMLcreator::text(const std::string &name, Database::Date value)
{
    this->text(name, stringify(value));
}
void 
XMLcreator::text(const std::string &name, Database::DateTime value)
{
    this->text(name, stringify(value));
}
void 
XMLcreator::text(const std::string &name, Money value)
{
    this->text(name, value.get_string(".2f"));//money
}
std::string 
XMLcreator::finalize()
{
    xmlFreeTextWriter(m_writer);
    m_writer = NULL;
    return std::string((const char *)m_buffer->content);
}

XMLnode::XMLnode(const XMLnode &sec)
{
    m_name = sec.getName();
    m_value = sec.getValue();
    for (int i = 0; i < sec.getChildrenSize(); i++) {
        addChild(sec.getChild(i));
    }
}
XMLnode 
XMLnode::operator=(const XMLnode &sec)
{
    m_name = sec.getName();
    m_value = sec.getValue();
    for (int i = 0; i < sec.getChildrenSize(); i++) {
        addChild(sec.getChild(i));
    }
    return sec;
}
void 
XMLnode::setName(const std::string &name)
{
    m_name = name;
}
void 
XMLnode::setValue(const std::string &value)
{
    m_value = trim(value);
}
void 
XMLnode::addChild(XMLnode node)
{
    m_children.push_back(node);
}
bool 
XMLnode::isEmpty() const
{
    return m_value.empty();
}
std::string 
XMLnode::getName() const
{
    return m_name;
}
std::string 
XMLnode::getValue() const
{
    return m_value;
}
int 
XMLnode::getChildrenSize() const
{
    return m_children.size();
}
int 
XMLnode::getChildrenSize(const std::string &name) const
{
    int size = 0;
    for (int i = 0; i < getChildrenSize(); i++) {
        if (name.compare(getChild(i).getName()) == 0) {
            size++;
        }
    }
    return size;
}
XMLnode 
XMLnode::getChild(int i) const
{
    if (i > (int)m_children.size()) {
        throw std::exception();
    }
    return m_children[i];
}
bool 
XMLnode::hasChild(const std::string &name) const
{
    for (int i = 0; i < getChildrenSize(); i++) {
        if (name.compare(getChild(i).getName()) == 0) {
            return true;
        }
    }
    return false;
}
XMLnode 
XMLnode::getChild(const std::string &name) const
{
    for (int i = 0; i < getChildrenSize(); i++) {
        if (name.compare(getChild(i).getName()) == 0) {
            return getChild(i);
        }
    }
    return XMLnode();
}

XMLnode 
XMLparser::parseNode(xmlNode *node)
{
    XMLnode ret;
    xmlNode *child;
    ret.setName((const char *)node->name);
    child = node->children;
    if (child) {
        if (child->type == XML_TEXT_NODE) {
            if (*child->content != '\n') {
                ret.setValue((const char *)child->content);
            }
        }
        for (; child; child = child->next) {
            if (child->type == XML_ELEMENT_NODE) {
                ret.addChild(parseNode(child));
            } else if (child->type == XML_TEXT_NODE) {
            }
        }
    }
    return ret;
}
bool 
XMLparser::parse(const std::string &xml)
{
    m_doc = xmlReadMemory(xml.c_str(), xml.length(), "", NULL, XML_PARSE_NOCDATA);
    if (m_doc == NULL) {
        return false;
    }
    m_root = xmlDocGetRootElement(m_doc);
    if (m_root == NULL) {
        return false;
    }
    try {
        m_rootNode = parseNode(m_root);
    } catch (...) {
        return false;
    }
    xmlFreeDoc(m_doc);
    xmlCleanupParser();
    return true;
}
XMLnode 
XMLparser::getRootNode() const
{
    return m_rootNode;
}

} // namespace LibFred::Banking
} // namespace LibFred
