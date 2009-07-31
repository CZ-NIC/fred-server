#include "bank_exporter.h"
#include "bank_head.h"
#include "bank_item.h"

namespace Register {
namespace Banking {

ExporterXML::~ExporterXML()
{
    m_xml.end();
    finalize();
}

void
ExporterXML::doExport(StatementHead *head)
{
    TRACE("[CALL] Register::Banking::ExporterXML::writeStatementHead()");
    m_xml.start(STATEMENT_STATEMENT);
    m_xml.text(STATEMENT_ACCOUNT_NUMBER, head->getAccountId());
    m_xml.text(STATEMENT_NUMBER, head->getNum());
    m_xml.text(STATEMENT_DATE, head->getCreateDate());
    m_xml.text(STATEMENT_BALANCE, head->getBalanceNew());
    m_xml.text(STATEMENT_OLD_DATE, head->getBalanceOldDate());
    m_xml.text(STATEMENT_OLD_BALANCE, head->getBalanceOld());
    m_xml.text(STATEMENT_CREDIT, head->getBalanceCredit());
    m_xml.text(STATEMENT_DEBET, head->getBalanceDebet());
    m_xml.start(STATEMENT_ITEMS);
    for (int i = 0; i < (int)head->getStatementItemCount(); i++) {
        doExport(head->getStatementItemByIdx(i));
    }
    m_xml.end();
    m_xml.end();
}

void
ExporterXML::finalize()
{
    m_out << m_xml.finalize();
}

void
ExporterXML::doExport(const StatementItem *item)
{
    TRACE("[CALL] Register::Banking::ExporterXML::writeStatementItem()");
    m_xml.start(ITEM_ITEM);
    m_xml.text(ITEM_IDENT, item->getAccountEvid());
    m_xml.text(ITEM_ACCOUNT_NUMBER, item->getAccountNumber());
    m_xml.text(ITEM_ACCOUNT_BANK_CODE, item->getBankCodeId());
    m_xml.text(ITEM_CONST_SYMBOL, item->getKonstSym());
    m_xml.text(ITEM_VAR_SYMBOL, item->getVarSymb());
    m_xml.text(ITEM_SPEC_SYMBOL, item->getSpecSymb());
    m_xml.text(ITEM_PRICE, item->getPrice());
    m_xml.text(ITEM_TYPE, item->getType());
    m_xml.text(ITEM_CODE, item->getCode());
    m_xml.text(ITEM_MEMO, item->getAccountMemo());
    m_xml.text(ITEM_DATE, item->getAccountDate());
    m_xml.text(ITEM_CRTIME, item->getCrTime());
    m_xml.text(ITEM_NAME, "");
    m_xml.end();
}

} // namespace Banking
} // namespace Register

