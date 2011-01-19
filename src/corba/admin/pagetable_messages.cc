#include "pagetable_messages.h"

#include <iostream>
#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>

ccReg_Messages_i::ccReg_Messages_i(
        Fred::Messages::Manager::MessageListPtr message_list)
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
        new Database::Filters::MessageImpl(true);
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

    COLHEAD(ch, 0, "Creation date", CT_OTHER);
    COLHEAD(ch, 1, "Channel", CT_OTHER);
    COLHEAD(ch, 2, "Message type", CT_OTHER);
    COLHEAD(ch, 3, "Status", CT_OTHER);
    COLHEAD(ch, 4, "Modification date", CT_OTHER);
    COLHEAD(ch, 5, "Attempt", CT_OTHER);

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
    throw (Registry::Table::INVALID_ROW)
{

    Logging::Context ctx(base_context_);

    Fred::Messages::MessagePtr msg;
    try
    {
        msg = ml->get(row);
    }
    catch(std::exception& ex)
    {
    	LOGGER(PACKAGE).error("ccReg_Messages_i::getRow invalid row");
        throw Registry::Table::INVALID_ROW();
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
    % msg->conv_get(Fred::Messages::MessageMetaInfo::MT_CRDATE)
    % msg->conv_get(Fred::Messages::MessageMetaInfo::MT_MODDATE)
    % msg->conv_get(Fred::Messages::MessageMetaInfo::MT_ATTEMPT)
    % msg->conv_get(Fred::Messages::MessageMetaInfo::MT_STATUS)
    % msg->conv_get(Fred::Messages::MessageMetaInfo::MT_COMMTYPE)
    % msg->conv_get(Fred::Messages::MessageMetaInfo::MT_MSGTYPE)
    );

    Registry::TableRow *tr = new Registry::TableRow;
    tr->length(numColumns());

    (*tr)[0] <<= C_STR(msg->conv_get(Fred::Messages::MessageMetaInfo::MT_CRDATE));
    (*tr)[1] <<= C_STR(msg->conv_get(Fred::Messages::MessageMetaInfo::MT_COMMTYPE));
    (*tr)[2] <<= C_STR(msg->conv_get(Fred::Messages::MessageMetaInfo::MT_MSGTYPE));
    (*tr)[3] <<= C_STR(msg->conv_get(Fred::Messages::MessageMetaInfo::MT_STATUS));
    (*tr)[4] <<= C_STR(msg->conv_get(Fred::Messages::MessageMetaInfo::MT_MODDATE));
    (*tr)[5] <<= C_STR(msg->conv_get(Fred::Messages::MessageMetaInfo::MT_ATTEMPT));

    return tr;
    }//try
    catch(std::exception& ex)
    {
        LOGGER(PACKAGE).error(boost::format("ccReg_Messages_i::getRow ex: %1%")
    		% ex.what());
        throw Registry::Table::INVALID_ROW();
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::getRow unknown exception");
        throw Registry::Table::INVALID_ROW();
    }
}

CORBA::Short
ccReg_Messages_i::numColumns()
{
	try
	{
    LOGGER(PACKAGE).debug(boost::format("ccReg_Messages_i::numColumns"
    		" numColumns %1%")
		% (Fred::Messages::MessageMetaInfo::columns - 1));

    return Fred::Messages::MessageMetaInfo::columns - 1;
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

    switch (column)
    {
    case 0 :
        ml->sort(Fred::Messages::MessageMetaInfo::MT_CRDATE, dir);
        break;
    case 1 :
        ml->sort(Fred::Messages::MessageMetaInfo::MT_COMMTYPE, dir);
        break;
    case 2 :
        ml->sort(Fred::Messages::MessageMetaInfo::MT_MSGTYPE, dir);
        break;
    case 3 :
        ml->sort(Fred::Messages::MessageMetaInfo::MT_STATUS, dir);
        break;
    case 4 :
        ml->sort(Fred::Messages::MessageMetaInfo::MT_MODDATE, dir);
        break;
    case 5 :
        ml->sort(Fred::Messages::MessageMetaInfo::MT_ATTEMPT, dir);
        break;
    default:
        throw std::runtime_error("ccReg_Messages_i::sortByColumn invalid column");
    }

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
    throw (Registry::Table::INVALID_ROW)
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
        throw Registry::Table::INVALID_ROW();
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("ccReg_Messages_i::getRowId unknown exception");
        throw Registry::Table::INVALID_ROW();
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
ccReg_Messages_i::reload_worker()
{
	try
	{
    Logging::Context ctx(base_context_);
    ConnectionReleaser releaser;

    LOGGER(PACKAGE).debug("ccReg_Messages_i::reload");
    ml->setTimeout(query_timeout);
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

    std::auto_ptr<Fred::Filter::Manager> tmp_filter_manager(
            Fred::Filter::Manager::create());
    tmp_filter_manager->save(Fred::Filter::FT_MESSAGE, name, uf);
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


Fred::Messages::Message *
ccReg_Messages_i::findId(ccReg::TID id)
{

    Logging::Context ctx(base_context_);
    try
    {
        Fred::Messages::MessagePtr msg
            = ml->findId(id);
        return msg.get();
    } catch (Fred::NOT_FOUND) {
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
