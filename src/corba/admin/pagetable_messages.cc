#include "pagetable_messages.h"

#include <iostream>
#include <sstream>

ccReg_Messages_i::ccReg_Messages_i(
        Register::Messages::Manager::MessageListPtr message_list)
	: ml(message_list)
{
}

ccReg_Messages_i::~ccReg_Messages_i()
{
    TRACE("[CALL] ccReg_Messages_i::~ccReg_Messages_i()");
}


ccReg::Filters::Compound_ptr
ccReg_Messages_i::add()
{
    TRACE("[CALL] ccReg_Messages_i::add()");
    Database::Filters::Message *filter =
        new Database::Filters::MessageImpl();
    uf.addFilter(filter);
    return it.addE(filter);
}

Registry::Table::ColumnHeaders *
ccReg_Messages_i::getColumnHeaders()
{
    Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
    ch->length(7);
    COLHEAD(ch, 0, "Message", CT_OID);
    COLHEAD(ch, 1, "Create date", CT_OTHER);
    COLHEAD(ch, 2, "Modification date", CT_OTHER);
    COLHEAD(ch, 3, "Attempt", CT_OTHER);
    COLHEAD(ch, 4, "Status", CT_OTHER);
    COLHEAD(ch, 5, "Communication channel", CT_OTHER);
    COLHEAD(ch, 6, "Message type", CT_OTHER);
    return ch;
}

Registry::TableRow *
ccReg_Messages_i::getRow(CORBA::UShort row)
    throw (ccReg::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    Register::Messages::MessagePtr msg;
    try
    {
        msg = ml->get(row);
    }
    catch(std::exception& ex)
    {
        throw ccReg::Table::INVALID_ROW();
    }

    Registry::TableRow *tr = new Registry::TableRow;

    tr->length(7);

    MAKE_OID(oid_message, msg->id, "", FT_MESSAGE)

    (*tr)[0] <<= oid_message;
    (*tr)[1] <<= C_STR(msg->crdate);
    (*tr)[2] <<= C_STR(msg->moddate);
    (*tr)[3] <<= C_STR(msg->attempt);
    (*tr)[4] <<= C_STR(msg->status);
    (*tr)[5] <<= C_STR(msg->comm_type);
    (*tr)[6] <<= C_STR(msg->message_type);
    return tr;
}

void
ccReg_Messages_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
    Logging::Context ctx(base_context_);

    TRACE(boost::format(
                "[CALL] ccReg_Messages_i::sortByColumn(%1%, %2%)")
            % column % dir);
    ccReg_PageTable_i::sortByColumn(column, dir);

    switch (column) {
        case 0:
            ml->sort(Register::Messages::MT_ID, dir);
            break;
        case 1:
            ml->sort(Register::Messages::MT_CRDATE, dir);
            break;
        case 2:
            ml->sort(Register::Messages::MT_MODDATE, dir);
            break;
        case 3:
            ml->sort(Register::Messages::MT_ATTEMPT, dir);
            break;
        case 4:
            ml->sort(Register::Messages::MT_STATUS, dir);
            break;
        case 5:
            ml->sort(Register::Messages::MT_COMMTYPE, dir);
            break;
        case 6:
            ml->sort(Register::Messages::MT_MSGTYPE, dir);
            break;
    }
}

ccReg::TID
ccReg_Messages_i::getRowId(CORBA::UShort row)
    throw (ccReg::Table::INVALID_ROW)
{
    Logging::Context ctx(base_context_);

    return ml->get(row)->id;

}

char *
ccReg_Messages_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_Messages_i::numRows()
{
    Logging::Context ctx(base_context_);

    return ml->size();
}

CORBA::Short
ccReg_Messages_i::numColumns()
{
    return 7;
}

void
ccReg_Messages_i::reload()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    ml->reload(uf);
}

void
ccReg_Messages_i::clear()
{
    Logging::Context ctx(base_context_);

    ccReg_PageTable_i::clear();
    ml->clear();
}

CORBA::ULongLong
ccReg_Messages_i::resultSize()
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_Messages_i::resultSize()");
    return ml->getRealCount(uf);
}

void
ccReg_Messages_i::loadFilter(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Messages_i::loadFilter(%1%)") % id);

    ccReg_PageTable_i::loadFilter(id);

    Database::Filters::Union::iterator uit = uf.begin();
    for (; uit != uf.end(); ++uit) {
        Database::Filters::Message *tmp =
            dynamic_cast<Database::Filters::Message *>(*uit);
        if (tmp) {
            it.addE(tmp);
            TRACE(boost::format("[IN] ccReg_Messages_i::loadFilter(%1%): "
                        "loaded filter content = %2%")
                    % id % tmp->getContent());
        }
    }
}

void
ccReg_Messages_i::saveFilter(const char *name)
{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Messages_i::saveFilter(%1%)")
            % name);

    std::auto_ptr<Register::Filter::Manager> tmp_filter_manager(
            Register::Filter::Manager::create());
    tmp_filter_manager->save(Register::Filter::FT_MESSAGE, name, uf);
}


Register::Messages::Message *
ccReg_Messages_i::findId(ccReg::TID id)
{
    Logging::Context ctx(base_context_);
    try
    {
        Register::Messages::MessagePtr msg
            = ml->findId(id);
        return msg.get();
    } catch (Register::NOT_FOUND) {
        return 0;
    }
}


CORBA::Boolean
ccReg_Messages_i::numRowsOverLimit()
{
    Logging::Context ctx(base_context_);

    return ml->isLimited();
}
