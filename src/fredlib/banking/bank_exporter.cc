#include "bank_exporter.h"
#include "bank_statement.h"
#include "bank_payment.h"

namespace Fred {
namespace Banking {

ExporterXML::~ExporterXML()
{
    m_xml.end();
    finalize();
}

void
ExporterXML::doExport(Statement *_data)
{
    TRACE("[CALL] Fred::Banking::ExporterXML::doExport(Statement)");
    m_xml.start(STATEMENT_STATEMENT);
    m_xml.text(STATEMENT_ACCOUNT_NUMBER, _data->getAccountId());
    m_xml.text(STATEMENT_NUMBER, _data->getNum());
    m_xml.text(STATEMENT_DATE, _data->getCreateDate());
    m_xml.text(STATEMENT_BALANCE, _data->getBalanceNew());
    m_xml.text(STATEMENT_OLD_DATE, _data->getBalanceOldDate());
    m_xml.text(STATEMENT_OLD_BALANCE, _data->getBalanceOld());
    m_xml.text(STATEMENT_CREDIT, _data->getBalanceCredit());
    m_xml.text(STATEMENT_DEBET, _data->getBalanceDebet());
    m_xml.start(STATEMENT_ITEMS);
    for (int i = 0; i < (int)_data->getPaymentCount(); i++) {
        doExport(_data->getPaymentByIdx(i));
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
ExporterXML::doExport(const Payment *_data)
{
    TRACE("[CALL] Fred::Banking::ExporterXML::writeStatementItem()");
    m_xml.start(ITEM_ITEM);
    m_xml.text(ITEM_IDENT, _data->getAccountEvid());
    m_xml.text(ITEM_ACCOUNT_NUMBER, _data->getAccountNumber());
    m_xml.text(ITEM_ACCOUNT_BANK_CODE, _data->getBankCode());
    m_xml.text(ITEM_CONST_SYMBOL, _data->getKonstSym());
    m_xml.text(ITEM_VAR_SYMBOL, _data->getVarSymb());
    m_xml.text(ITEM_SPEC_SYMBOL, _data->getSpecSymb());
    m_xml.text(ITEM_PRICE, _data->getPrice());
    m_xml.text(ITEM_TYPE, _data->getType());
    m_xml.text(ITEM_CODE, _data->getCode());
    m_xml.text(ITEM_MEMO, _data->getAccountMemo());
    m_xml.text(ITEM_DATE, _data->getAccountDate());
    m_xml.text(ITEM_CRTIME, _data->getCrTime());
    m_xml.text(ITEM_NAME, "");
    m_xml.end();
}

} // namespace Banking
} // namespace Fred

