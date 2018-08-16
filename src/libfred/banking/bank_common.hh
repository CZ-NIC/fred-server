#ifndef BANK_COMMON_HH_600B23674C6343DDB75AD74E697FF953
#define BANK_COMMON_HH_600B23674C6343DDB75AD74E697FF953

#include <istream>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>

#include "src/util/types/data_types.hh"
#include "src/util/types/money.hh"
#include "src/util/decimal/decimal.hh"

#include "config.h"

#ifdef HAVE_LOGGER
#include "src/util/log/logger.hh"
#endif

namespace LibFred {
namespace Banking {

#define XML_NODE_NONE
#define XML_NODE_ELEMENT
#define XML_NODE_ATTRIBUTE
#define XML_NODE_TEXT
#define XML_NODE_CDATA
#define XML_NODE_ENTITY_REFERENCE
#define XML_NODE_ENTITY
#define XML_NODE_PROCESSING_INSTRUCTION
#define XML_NODE_COMMENT
#define XML_NODE_DOCUMENT
#define XML_NODE_DOCUMENT_TYPE
#define XML_NODE_DOCUMENT_FRAGMENT
#define XML_NODE_NOTATION
#define XML_NODE_WHITESPACE
#define XML_NODE_SIGNIFICANT_WHITESPACE
#define XML_NODE_END_ELEMENT
#define XML_NODE_END_ENTITY
#define XML_NODE_XML_DECLARATION


const std::string STATEMENTS_ROOT               =  "statements";
const std::string STATEMENT_STATEMENT           =  "statement";
const std::string STATEMENT_ACCOUNT_NUMBER      =  "account_number";
const std::string STATEMENT_ACCOUNT_BANK_CODE   =  "account_bank_code";
const std::string STATEMENT_NUMBER              =  "number";
const std::string STATEMENT_DATE                =  "date";
const std::string STATEMENT_BALANCE             =  "balance";
const std::string STATEMENT_OLD_DATE            =  "old_date";
const std::string STATEMENT_OLD_BALANCE         =  "old_balance";
const std::string STATEMENT_CREDIT              =  "credit";
const std::string STATEMENT_DEBET               =  "debet";
const std::string STATEMENT_ITEMS               =  "items";
const std::string ITEM_ITEM                     =  "item";
const std::string ITEM_IDENT                    =  "ident";
const std::string ITEM_ACCOUNT_NUMBER           =  "account_number";
const std::string ITEM_ACCOUNT_BANK_CODE        =  "account_bank_code";
const std::string ITEM_CONST_SYMBOL             =  "const_symbol";
const std::string ITEM_VAR_SYMBOL               =  "var_symbol";
const std::string ITEM_SPEC_SYMBOL              =  "spec_symbol";
const std::string ITEM_PRICE                    =  "price";
const std::string ITEM_TYPE                     =  "type";
const std::string ITEM_CODE                     =  "code";
const std::string ITEM_STATUS                   =  "status";
const std::string ITEM_MEMO                     =  "memo";
const std::string ITEM_DATE                     =  "date";
const std::string ITEM_CRTIME                   =  "crtime";
const std::string ITEM_NAME                     =  "name";

#define TEST_NODE_PRESENCE(parent, name)                                \
    if (!parent.hasChild(name)) {                                       \
        LOGGER(PACKAGE).error(boost::format("``%1%'' node not found")   \
                % name);                                                \
        return false;                                                   \
    }

std::string loadInStream(std::istream &in);

class XMLcreator {
private:
    xmlBuffer       *m_buffer;
    xmlTextWriter   *m_writer;
    bool            m_writeXmlHeader;
public:
    XMLcreator(): m_buffer(NULL), m_writer(NULL)
    { }
    ~XMLcreator();
    bool init(bool writeXmlHeader = false);
    void start(const std::string &name);
    void start(const char *name);
    void end();
    void text(const std::string &name, const std::string &value);
    void text(const std::string &name, const char *value);
    void text(const std::string &name, int value);
    void text(const std::string &name, unsigned long long value);
    void text(const std::string &name, double value);
    void text(const std::string &name, Database::ID value);
    void text(const std::string &name, Database::Date value);
    void text(const std::string &name, Database::DateTime value);
    void text(const std::string &name, Money value);
    std::string finalize();
}; // class XMLcreator;

class XMLnode {
private:
    std::string             m_name;
    std::string             m_value;
    std::vector<XMLnode>    m_children;
public:
    XMLnode()
    { }
    XMLnode(const XMLnode &sec);
    ~XMLnode()
    { }
    XMLnode operator=(const XMLnode &sec);
    void setName(const std::string &name);
    void setValue(const std::string &value);
    void addChild(XMLnode node);
    /* return true if node value (as string) length is zero characters */
    bool isEmpty() const;
    std::string getName() const;
    std::string getValue() const;
    int getChildrenSize() const;
    int getChildrenSize(const std::string &name) const;
    XMLnode getChild(int i) const;
    bool hasChild(const std::string &name) const;
    XMLnode getChild(const std::string &name) const;
}; // class XMLnode;

class XMLparser {
private:
    xmlDoc      *m_doc;
    xmlNode     *m_root;
    XMLnode     m_rootNode;
    XMLnode parseNode(xmlNode *node);
public:
    XMLparser(): m_doc(NULL), m_root(NULL)
    { }
    ~XMLparser()
    { }
    bool parse(const std::string &xml);
    XMLnode getRootNode() const;
}; // class XMLparse;

} // namespace Banking
} // namespace LibFred

#endif
