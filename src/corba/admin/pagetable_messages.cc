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
	try
	{

	TRACE("[CALL] ccReg_Messages_i::add()");
    Database::Filters::Message *filter =
        new Database::Filters::MessageImpl();
    uf.addFilter(filter);
    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::add uf.empty %1%")
		% uf.empty());
    return it.addE(filter);

    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::add ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::add unknown exception");
        throw;
    }
}

Registry::Table::ColumnHeaders *
ccReg_Messages_i::getColumnHeaders()
{
	try
	{

    Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
    ch->length(numColumns());
    COLHEAD(ch, 0, "Message", CT_OID);
    COLHEAD(ch, 1, "Create date", CT_OTHER);
    COLHEAD(ch, 2, "Modification date", CT_OTHER);
    COLHEAD(ch, 3, "Attempt", CT_OTHER);
    COLHEAD(ch, 4, "Status", CT_OTHER);
    COLHEAD(ch, 5, "Communication channel", CT_OTHER);
    COLHEAD(ch, 6, "Message type", CT_OTHER);

    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::getColumnHeaders"
    		" numColumns %1%")
		% numColumns());
    return ch;
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::getColumnHeaders ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::getColumnHeaders unknown exception");
        throw;
    }
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
    	LOGGER(PACKAGE).error("ccReg_Messages_i::getRow invalid row");
        throw ccReg::Table::INVALID_ROW();
    }

	try
	{
    LOGGER(PACKAGE).debug(boost::format(
    		"getRow %1%"
    		" MT_ID %2%"
    		" MT_CRDATE %3%"
    		" MT_MODDATE %4%"
    		" MT_ATTEMPT %5%"
    		" MT_STATUS %6%"
    		" MT_COMMTYPE  %7%"
    		" MT_MSGTYPE  %8%"
		)
    % row
    % msg->get_id()
    % msg->get(Register::Messages::MessageMetaInfo::MT_CRDATE)
    % msg->get(Register::Messages::MessageMetaInfo::MT_MODDATE)
    % msg->get(Register::Messages::MessageMetaInfo::MT_ATTEMPT)
    % msg->get(Register::Messages::MessageMetaInfo::MT_STATUS)
    % msg->get(Register::Messages::MessageMetaInfo::MT_COMMTYPE)
    % msg->get(Register::Messages::MessageMetaInfo::MT_MSGTYPE)
    );

    Registry::TableRow *tr = new Registry::TableRow;

    tr->length(numColumns());

    MAKE_OID(oid_message, msg->get_id(), "", FT_MESSAGE)

    (*tr)[0] <<= oid_message;
    (*tr)[1] <<= C_STR(msg->get(Register::Messages::MessageMetaInfo::MT_CRDATE));
    (*tr)[2] <<= C_STR(msg->get(Register::Messages::MessageMetaInfo::MT_MODDATE));
    (*tr)[3] <<= C_STR(msg->get(Register::Messages::MessageMetaInfo::MT_ATTEMPT));
    (*tr)[4] <<= C_STR(msg->get(Register::Messages::MessageMetaInfo::MT_STATUS));
    (*tr)[5] <<= C_STR(msg->get(Register::Messages::MessageMetaInfo::MT_COMMTYPE));
    (*tr)[6] <<= C_STR(msg->get(Register::Messages::MessageMetaInfo::MT_MSGTYPE));
    return tr;
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::getRow ex: %1%")
    		% ex.what());
        throw ccReg::Table::INVALID_ROW();
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::getRow unknown exception");
        throw ccReg::Table::INVALID_ROW();
    }
}

CORBA::Short
ccReg_Messages_i::numColumns()
{
	try
	{
    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::numColumns"
    		" numColumns %1%")
		% Register::Messages::MessageMetaInfo::columns);

    return Register::Messages::MessageMetaInfo::columns;
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::numColumns ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::numColumns unknown exception");
        throw;
    }

}

void
ccReg_Messages_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
	try
	{
    Logging::Context ctx(base_context_);

    TRACE(boost::format(
                "[CALL] ccReg_Messages_i::sortByColumn(%1%, %2%)")
            % column % dir);
    ccReg_PageTable_i::sortByColumn(column, dir);

    ml->sort(static_cast<Register::Messages::MessageMetaInfo::MemberType>(column), dir);
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::sortByColumn ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::sortByColumn unknown exception");
        throw;
    }
}

ccReg::TID
ccReg_Messages_i::getRowId(CORBA::UShort row)
    throw (ccReg::Table::INVALID_ROW)
{
	try
	{
    Logging::Context ctx(base_context_);

    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::getRowId"
    		" row %1% id %2%")
		% row
		% ml->get(row)->get_id());

    return ml->get(row)->get_id();
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::getRowId ex: %1%")
    		% ex.what());
        throw ccReg::Table::INVALID_ROW();
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::getRowId unknown exception");
        throw ccReg::Table::INVALID_ROW();
    }
}

char *
ccReg_Messages_i::outputCSV()
{
    return CORBA::string_dup("1,1,1");
}

CORBA::Short
ccReg_Messages_i::numRows()
{
	try
	{
    Logging::Context ctx(base_context_);
    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::numRows"
    		" numRows %1%")
		% ml->size()
		);
    return ml->size();
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::numRows ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::numRows unknown exception");
        throw;
    }
}

void
ccReg_Messages_i::reload()
{
	try
	{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    LOGGER(PACKAGE).debug("ccReg_Messages_i::reload");
    ml->reload(uf);
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::reload ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::reload unknown exception");
        throw;
    }
}

void
ccReg_Messages_i::clear()
{
	try
	{
    Logging::Context ctx(base_context_);

    ccReg_PageTable_i::clear();
    ml->clear();
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::clear ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::clear unknown exception");
        throw;
    }
}

CORBA::ULongLong
ccReg_Messages_i::resultSize()
{
	try
	{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE("[CALL] ccReg_Messages_i::resultSize()");

    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::resultSize"
    		" getRealCount %1%")
		% ml->getRealCount(uf)
		);

    return ml->getRealCount(uf);
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::resultSize ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::resultSize unknown exception");
        throw;
    }
}

void
ccReg_Messages_i::loadFilter(ccReg::TID id)
{
	try
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
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::loadFilter ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::loadFilter unknown exception");
        throw;
    }
}

void
ccReg_Messages_i::saveFilter(const char *name)
{
	try
	{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    TRACE(boost::format("[CALL] ccReg_Messages_i::saveFilter(%1%)")
            % name);

    std::auto_ptr<Register::Filter::Manager> tmp_filter_manager(
            Register::Filter::Manager::create());
    tmp_filter_manager->save(Register::Filter::FT_MESSAGE, name, uf);
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::saveFilter ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::saveFilter unknown exception");
        throw;
    }
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
	catch(std::exception& ex)
	{
		LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::findId ex: %1%")
			% ex.what());
		throw;
	}
	catch(...)
	{
		LOGGER(PACKAGE).error("ccReg_Messages_i::findId unknown exception");
		throw;
	}
}


CORBA::Boolean
ccReg_Messages_i::numRowsOverLimit()
{
	try
	{
    Logging::Context ctx(base_context_);
    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::numRowsOverLimit"
        		" %1%")
    		% ml->isLimited()
    		);
    return ml->isLimited();
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::numRowsOverLimit ex: %1%")
    		% ex.what());
        throw;
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::numRowsOverLimit unknown exception");
        throw;
    }
}