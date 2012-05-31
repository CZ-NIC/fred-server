#include "pagetable_logger.h"

const int ccReg_Logger_i::NUM_COLUMNS = 8;


ccReg_Logger_i::ccReg_Logger_i(Fred::Logger::List *_list) : m_lel (_list)  {
}

ccReg_Logger_i::~ccReg_Logger_i() {
  TRACE("[CALL] ccReg_Logger_i::~ccReg_Logger_i()");
}

ccReg::Filters::Compound_ptr ccReg_Logger_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Logger_i::add()");
  Database::Filters::Request *f = new Database::Filters::RequestImpl(true);
  uf.addFilter(f);
  return it.addE(f);
}

Registry::Table::ColumnHeaders* ccReg_Logger_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(NUM_COLUMNS);
  COLHEAD(ch,0,"time_begin",CT_OTHER);
  COLHEAD(ch,1,"time_end",CT_OTHER);
  COLHEAD(ch,2,"serv_type",CT_OTHER);
  COLHEAD(ch,3,"source_ip",CT_OTHER);
  COLHEAD(ch,4,"request_type_id",CT_OTHER);  
  COLHEAD(ch,5,"user_name",CT_OTHER);
  COLHEAD(ch,6,"is_monitoring",CT_OTHER);
  COLHEAD(ch,7,"result",CT_OTHER);

  return ch;
}

Registry::TableRow* ccReg_Logger_i::getRow(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  try {
    const Fred::Logger::Request *a = m_lel->get(row);
    Registry::TableRow *tr = new Registry::TableRow;
    tr->length(NUM_COLUMNS);

    (*tr)[0] <<= C_STR(a->getTimeBegin());
    (*tr)[1] <<= C_STR(a->getTimeEnd());
    (*tr)[2] <<= C_STR(a->getServiceType());
    (*tr)[3] <<= C_STR(a->getSourceIp());
    (*tr)[4] <<= C_STR(a->getActionType());    
    (*tr)[5] <<= C_STR(a->getUserName());
    (*tr)[6] <<= C_STR(a->getIsMonitoring());
    (*tr)[7] <<= C_STR((a->getResultCode()).second);
    return tr;
  }
  catch (...) {
    throw Registry::Table::INVALID_ROW();
  }
}

void ccReg_Logger_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Logger_i::sortByColumn(%1%, %2%)")
      % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);

  switch (column) {
    case 0:
      m_lel->sort(Fred::Logger::MT_TIME_BEGIN, dir);
      break;
    case 1:
      m_lel->sort(Fred::Logger::MT_TIME_END, dir);
      break;
    case 2:
      m_lel->sort(Fred::Logger::MT_SERVICE, dir);
      break;
    case 3:
      m_lel->sort(Fred::Logger::MT_SOURCE_IP, dir);
      break;
    case 4:
      m_lel->sort(Fred::Logger::MT_ACTION, dir);
      break;    
    case 5:
      m_lel->sort(Fred::Logger::MT_USER_NAME, dir);
      break;
    case 6:
      m_lel->sort(Fred::Logger::MT_MONITORING, dir);
      break;
    case 7:
      m_lel->sort(Fred::Logger::MT_RESULT_CODE, dir);
      break;

  }
}

ccReg::TID ccReg_Logger_i::getRowId(CORBA::UShort row)
    throw (Registry::Table::INVALID_ROW) {
  Logging::Context ctx(base_context_);

  const Fred::Logger::Request *a = m_lel->get(row);
  if (!a)
    throw Registry::Table::INVALID_ROW();
  return a->getId();
}

char* ccReg_Logger_i::outputCSV() {
  return CORBA::string_dup("1,1,1,1,1,1");
}

CORBA::Short ccReg_Logger_i::numRows() {
  Logging::Context ctx(base_context_);

  return m_lel->size();
}

CORBA::Short ccReg_Logger_i::numColumns() {
  Logging::Context ctx(base_context_);

  return NUM_COLUMNS;
}

void ccReg_Logger_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_Logger_i::reload_worker()");
  m_lel->setPartialLoad(true);
//  m_lel->reload(uf);

  // CustomPartitioningTweak::process_filters(uf.begin(), uf.end()); 
  m_lel->setTimeout(query_timeout);
  m_lel->reload(uf);
}

void ccReg_Logger_i::clear() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Logger_i::clear()");
  ccReg_PageTable_i::clear();
  m_lel->clear();
}

CORBA::ULongLong ccReg_Logger_i::resultSize() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("ccReg_Logger_i::resultSize()");
  return m_lel->getRealCount(uf);
}

void ccReg_Logger_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Logger_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Request *tmp = dynamic_cast<Database::Filters::Request* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Logger_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_Logger_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Logger_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Fred::Filter::Manager>
      tmp_filter_manager(Fred::Filter::Manager::create());
  tmp_filter_manager->save(Fred::Filter::FT_LOGGER, _name, uf);
}

Fred::Logger::Request* ccReg_Logger_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Fred::Logger::Request *request = dynamic_cast<Fred::Logger::Request* >(m_lel->findId(_id));
    if (request) {
      return request;
    }
    return 0;
  }
  catch (Fred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Logger_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return m_lel->isLimited();
}

void ccReg_Logger_i::setOffset(CORBA::Long _offset)
{
    m_lel->setOffset(_offset);
}

void ccReg_Logger_i::setLimit(CORBA::Long _limit)
{
    m_lel->setLimit(_limit);
}


