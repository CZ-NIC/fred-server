#include "bank_item.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Register {
namespace Banking {


bool
StatementItem::save()
{
    TRACE("[CALL] Register::Banking::StatementItem::save()");
    try {
        if (getId() == Database::ID()) {
            insert();
        } else {
            update();
        }
    } catch (...) {
        LOGGER(PACKAGE).error("StatementItem: save() failed");
        return false;
    }
    return true;
}

/* returns id (or 0) of payment which is identical to this one,
 * in case, that there is more than one valid result, method gets
 * the first one (this is due to some payments were completely
 * same, except second in timestamp and unfortunately Raiffaissen
 * TXT reports don't differ seconds).
 */
Database::ID
StatementItem::getExisting()
{
    TRACE("[CALL] Register::Banking::StatementItem::getExisting()");
    Database::Query query;
    query.buffer()
        << "SELECT si.id FROM bank_item si "
        << "WHERE si.account_number = " << Database::Value(getAccountNumber())
        << " AND si.bank_code = " << Database::Value(getBankCodeId())
        << " AND si.price = " << Database::Value(getPrice())
        << " AND si.account_date = " << Database::Value(getAccountDate())
        << " AND statement_id IS NULL";
    Database::Connection conn = Database::Manager::acquire();
    Database::ID retval;
    try {
        Database::Result res = conn.exec(query);
        if (res.size() != 0) {
            if (res.size() > 1) {
                LOGGER(PACKAGE).warning("getExisting: more than one valid "
                        "result, using first");
            }
            retval = res[0][0];
        } else {
            LOGGER(PACKAGE).debug("getExisting: existing not found");
        }
    } catch (...) {
        LOGGER(PACKAGE).error("getExisting: an error has occured");
        return Database::ID();
    }
    return retval;
}

bool
StatementItem::moveItem(const Database::ID &paymentId)
{
    TRACE("Register::Banking::StatementItem::moveItem(Database::ID)");
    ModelBankStatementItem item;
    item.setId(paymentId);
    item.reload();
    item.setStatementId(getStatementId());
    try {
        item.update();
    } catch (...) {
        LOGGER(PACKAGE).error("Failed to move item to new header");
        return false;
    }
    return true;
}

void
StatementItem::testBankCode()
{
    Database::Query test;
    test.buffer()
        << "SELECT code FROM enum_bank_code WHERE code="
        << Database::Value(getBankCodeId());
    Database::Connection conn = Database::Manager::acquire();
    try {
        Database::Result res = conn.exec(test);
        if (res.size() == 0) {
            LOGGER(PACKAGE).warning(
                    boost::format("Bank code (%1%) does not exists in the database") %
                    getBankCodeId());
        } else {
            LOGGER(PACKAGE).debug(
                    boost::format("Bank code (%1%) exists in the database") % 
                    getBankCodeId());
        }
    } catch (...) {
        LOGGER(PACKAGE).error("An error has occured.");
    }
}

#define SET_IF_PRESENT(item, name, setter)                      \
    if (item.hasChild(name)) {                                  \
        std::string val = item.getChild(name).getValue();       \
        if (val.compare("") != 0) {                             \
            setter(item.getChild(name).getValue());             \
        }                                                       \
    }

bool 
StatementItem::fromXML(const XMLnode &item)
{
    TRACE("[CALL] Register::Banking::StatementItem::fromXML()");
    TEST_NODE_PRESENCE(item, ITEM_IDENT);
    TEST_NODE_PRESENCE(item, ITEM_ACCOUNT_NUMBER);
    TEST_NODE_PRESENCE(item, ITEM_ACCOUNT_BANK_CODE);
    TEST_NODE_PRESENCE(item, ITEM_PRICE);

    setAccountEvid(item.getChild(ITEM_IDENT).getValue());
    setAccountNumber(item.getChild(ITEM_ACCOUNT_NUMBER).getValue());
    setBankCodeId(item.getChild(ITEM_ACCOUNT_BANK_CODE).getValue());
    testBankCode();
    SET_IF_PRESENT(item, ITEM_CONST_SYMBOL, setKonstSym);
    SET_IF_PRESENT(item, ITEM_VAR_SYMBOL, setVarSymb);
    SET_IF_PRESENT(item, ITEM_SPEC_SYMBOL, setSpecSymb);
    Database::Money money;
    money.format(item.getChild(ITEM_PRICE).getValue());
    setPrice(money);
    if (item.hasChild(ITEM_TYPE)) {
        setType(atoi(item.getChild(ITEM_TYPE).getValue().c_str()));
    }
    if (item.hasChild(ITEM_CODE)) {
        setCode(atoi(item.getChild(ITEM_CODE).getValue().c_str()));
    }
    SET_IF_PRESENT(item, ITEM_MEMO, setAccountMemo);
    SET_IF_PRESENT(item, ITEM_DATE, setAccountDate);
    SET_IF_PRESENT(item, ITEM_CRTIME, setCrTime);
    SET_IF_PRESENT(item, ITEM_NAME, setAccountName);
    Database::ID existingPaymentId = getExisting();
    if (existingPaymentId != Database::ID()) {
        return moveItem(existingPaymentId);
    }
    bool retval = save();
    if (retval == false) {
        LOGGER(PACKAGE).error("Failed to insert Stamenet item");
    }
    return retval;
}

#undef SET_IF_PRESENT

} // namespace Banking
} // namespace Register
