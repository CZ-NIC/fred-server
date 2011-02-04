#include "common_impl_new.h"

namespace Fred {

CommonObjectImplNew::CommonObjectImplNew()
{
}

CommonObjectImplNew::~CommonObjectImplNew()
{
}

CommonListImplNew::CommonListImplNew():
    m_realSizeInitialized(false),
    m_realSize(0),
    m_loadLimitActive()
{
    setLimit(1000);
}

CommonListImplNew::~CommonListImplNew()
{
    for (unsigned int i = 0; i < getSize(); i++) {
        delete m_data[i];
    }
}

CommonObjectNew *
CommonListImplNew::get(unsigned int index) const
{
    return m_data.at(index);
}

unsigned int
CommonListImplNew::getSize() const
{
    return m_data.size();
}

unsigned int
CommonListImplNew::size() const
{
    return getSize();
}


void 
CommonListImplNew::clear()
{
    for (unsigned int i = 0; i < getSize(); i++) {
        delete m_data[i];
    }
    m_data.clear();
    m_realSize = 0;
    m_realSizeInitialized = false;
}

bool
CommonListImplNew::isEmpty()
{
    return m_data.empty();
}

void
CommonListImplNew::appendToList(CommonObjectNew * object)
{
    m_data.push_back(object);
}

void
CommonListImplNew::setLimit(unsigned int limit)
{
    m_limit = limit;
}

unsigned int
CommonListImplNew::getLimit() const
{
    return m_limit;
}

void
CommonListImplNew::fillTempTable(Database::InsertQuery &query)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction trans(conn);
    try {
        Database::Query create_tmp_table("SELECT create_tmp_table('" +
                std::string(getTempTableName()) + "')");
        conn.exec(create_tmp_table);
        conn.exec(query);

        Database::Query analyze("ANALYZE " + std::string(getTempTableName()));
        conn.exec(analyze);
    }
    catch (Database::Exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        throw;
    }
    catch (std::exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        throw;
    }
    trans.commit();
}
void
CommonListImplNew::makeRealCount(Database::Filters::Union &filter)
{
    TRACE("[CALL] CommonListImplNew::makeRealCount(Database::Filters::Union &)");

    if (filter.empty()) {
        m_realSize = 0;
        m_realSizeInitialized = false;
        LOGGER(PACKAGE).warning(
                "can't make real filter data count -- no filter specified...");
        return;
    }

    filter.clearQueries();

    Database::Filters::Union::iterator it = filter.begin();
    for (; it != filter.end(); ++it) {
        Database::SelectQuery *tmp = new Database::SelectQuery();
        tmp->select() << "COUNT(*)";
        filter.addQuery(tmp);
    }

    Database::SelectQuery count_query;
    filter.serialize(count_query);

    Database::Connection conn = Database::Manager::acquire();

    try {
        Database::Result r_count = conn.exec(count_query);
        m_realSize = (*(r_count.begin()))[0];
        m_realSizeInitialized = true;
    }
    catch (Database::Exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
    catch (std::exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
    }
}

unsigned long long 
CommonListImplNew::getRealCount(Database::Filters::Union &filter)
{
    if (!m_realSizeInitialized) {
        makeRealCount(filter);
    }
    return m_realSize;
}

void
CommonListImplNew::reload()
{
    m_loadLimitActive = false;
    if (getSize() > getLimit()) {
        CommonObjectNew *tmp = m_data.back();
        m_data.pop_back();
        delete tmp;
        m_loadLimitActive = true;
    }
}
bool
CommonListImplNew::isLimited() const
{
    return m_loadLimitActive;
}

void
CommonListImplNew::setTimeout(unsigned _timeout)
{
  Database::Connection conn = Database::Manager::acquire();

  conn.setQueryTimeout(_timeout);
}

} // namespace Fred
