#include "src/libfred/banking/bank_common.hh"

#include "src/libfred/banking/bank_payment.hh"
#include "src/libfred/banking/bank_payment_impl.hh"
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

EnumList getBankAccounts()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT  id, "
            " account_name || ' ' || account_number || '/' || bank_code "
            " FROM bank_account "
            " ORDER BY id");
    EnumList el;
    for(unsigned i=0;i<res.size();i++)
    {
        EnumListItem eli;
        eli.id = res[i][0];
        eli.name = std::string(res[i][1]);
        el.push_back(eli);
    }
    return el;
}//getBankAccounts

PaymentImplPtr parse_xml_payment_part(const XMLnode &_node)
{
    TRACE("[CALL] LibFred::Banking::payment_from_xml(...)");

    /* manual xml validation */
    if (!_node.hasChild(ITEM_IDENT)
            || !_node.hasChild(ITEM_ACCOUNT_NUMBER)
            || !_node.hasChild(ITEM_ACCOUNT_BANK_CODE)
            || !_node.hasChild(ITEM_CONST_SYMBOL)
            || !_node.hasChild(ITEM_VAR_SYMBOL)
            || !_node.hasChild(ITEM_SPEC_SYMBOL)
            || !_node.hasChild(ITEM_PRICE)
            || !_node.hasChild(ITEM_TYPE)
            || !_node.hasChild(ITEM_CODE)
            || !_node.hasChild(ITEM_MEMO)
            || !_node.hasChild(ITEM_DATE)
            || !_node.hasChild(ITEM_CRTIME)
            || !_node.hasChild(ITEM_NAME)) {

        throw std::runtime_error("not valid payment xml");
    }
    PaymentImplPtr payment(new PaymentImpl());
    if (!_node.getChild(ITEM_IDENT).isEmpty()) {
        std::string value = _node.getChild(ITEM_IDENT).getValue();
        payment->setAccountEvid(value);
    }
    else {
        throw std::runtime_error("no valid payment xml; "
                "cannot identify payment with no accountevid");
    }
    // if (!_node.getChild(ITEM_ACCOUNT_NUMBER).isEmpty()) {
    {
        std::string value = _node.getChild(ITEM_ACCOUNT_NUMBER).getValue();
        payment->setAccountNumber(value);
    }
    // }
    // if (!_node.getChild(ITEM_ACCOUNT_BANK_CODE).isEmpty()) {
    {
        std::string value = _node.getChild(ITEM_ACCOUNT_BANK_CODE).getValue();
        payment->setBankCode(value);
    }
        /* test if is in databaase? */
    // }
    // if (!_node.getChild(ITEM_PRICE).isEmpty()) {
    {
        Money value(_node.getChild(ITEM_PRICE).getValue());
        payment->setPrice(value);
    }
    // }

    if (!_node.getChild(ITEM_CONST_SYMBOL).isEmpty()) {
        std::string value = _node.getChild(ITEM_CONST_SYMBOL).getValue();
        payment->setKonstSym(value);
    }
    if (!_node.getChild(ITEM_VAR_SYMBOL).isEmpty()) {
        std::string value = _node.getChild(ITEM_VAR_SYMBOL).getValue();
        payment->setVarSymb(value);
    }
    if (!_node.getChild(ITEM_SPEC_SYMBOL).isEmpty()) {
        std::string value = _node.getChild(ITEM_SPEC_SYMBOL).getValue();
        payment->setSpecSymb(value);
    }
    if (!_node.getChild(ITEM_MEMO).isEmpty()) {
        std::string value = _node.getChild(ITEM_MEMO).getValue();
        payment->setAccountMemo(value);
    }
    if (!_node.getChild(ITEM_DATE).isEmpty()) {
        Database::Date value = _node.getChild(ITEM_DATE).getValue();
        payment->setAccountDate(value);
    }
    if (!_node.getChild(ITEM_CRTIME).isEmpty()) {
        Database::DateTime value = _node.getChild(ITEM_CRTIME).getValue();
        payment->setCrTime(value);
    }
    if (!_node.getChild(ITEM_NAME).isEmpty()) {
        std::string value = _node.getChild(ITEM_NAME).getValue();
        payment->setAccountName(value);
    }


    if (!_node.getChild(ITEM_TYPE).isEmpty()) {
        int value = atoi(_node.getChild(ITEM_TYPE).getValue().c_str());
        payment->setType(value);
    }
    if (!_node.getChild(ITEM_CODE).isEmpty()) {
        int value = atoi(_node.getChild(ITEM_CODE).getValue().c_str());
        payment->setCode(value);
    }
    if (!_node.getChild(ITEM_STATUS).isEmpty()) {
        int value = atoi(_node.getChild(ITEM_STATUS).getValue().c_str());
        payment->setStatus(value);
    }

    return payment;
}

PaymentImplPtr payment_from_params(
        const std::string& _bank_payment,
        const std::string& _uuid,
        const std::string& _account_number,
        const std::string& _bank_code,
        const std::string& _counter_account_number,
        const std::string& _counter_account_name,
        const std::string& _constant_symbol,
        const std::string& _variable_symbol,
        const std::string& _specific_symbol,
        const Money& _price,
        const boost::gregorian::date& _date,
        const std::string& _memo,
        const boost::posix_time::ptime& _creation_time)
{
    TRACE("[CALL] LibFred::Banking::payment_from_params(...)");

    PaymentImplPtr payment(new PaymentImpl());

    payment->setAccountNumber(_account_number);
    payment->setBankCode(_bank_code);
    payment->setPrice(_price);

    if (!_constant_symbol.empty()) {
        payment->setKonstSym(_constant_symbol);
    }
    if (!_variable_symbol.empty()) {
        payment->setVarSymb(_variable_symbol);
    }
    if (!_specific_symbol.empty()) {
        payment->setSpecSymb(_specific_symbol);
    }
    if (!_memo.empty()) {
        payment->setAccountMemo(_memo);
    }
    //if (!_date.is_special()) {
        payment->setAccountDate(_date);
    //}
    //if (!_creation_time.empty()) {
        payment->setCrTime(_creation_time);
    //}
    if (!_counter_account_name.empty()) {
        payment->setAccountName(_counter_account_name);
    }

    // TODO FIXME
    payment->setType(1); // transfer type (1-not decided (not processed), 2-from/to registrar, 3-from/to bank, 4-between our own accounts, 5-related to academia, 6-other transfers
    payment->setCode(1); // operation code (1-debet item, 2-credit item, 4-cancel debet, 5-cancel credit)
    payment->setStatus(1); // payment status (1-Realized (only this should be further processed), 2-Partially realized, 3-Not realized, 4-Suspended, 5-Ended, 6-Waiting for clearing )

    return payment;
}


} // namespace Banking
} // namespace LibFred
