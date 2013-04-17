#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "mail.h"
#include "log/logger.h"


namespace Fred {
namespace Mail {

class MailImpl : public Fred::CommonObjectImpl,
                 virtual public Mail {
private:
  Database::DateTime create_time_;
  Database::DateTime mod_time_;
  long type_;
  std::string type_desc_;
  long status_;
  std::string content_;
  std::vector<std::string> handles_;
  std::vector<Fred::OID> attachments_;

public:
  MailImpl(Database::ID& _id, Database::DateTime& _create_time, Database::DateTime& _mod_time,
           long _type, std::string& _type_desc, long _status, std::string& _content) :
             CommonObjectImpl(_id),
             create_time_(_create_time),
             mod_time_(_mod_time),
             type_(_type),
             type_desc_(_type_desc),
             status_(_status),
             content_(_content) {
  }
  
  virtual const Database::DateTime& getCreateTime() const {
    return create_time_;
  }
  
  virtual const Database::DateTime& getModTime() const {
    return mod_time_;
  }
  
  virtual const long &getType() const {
    return type_;
  }
  
  virtual const std::string& getTypeDesc() const {
    return type_desc_;
  }
  
  virtual const long &getStatus() const {
    return status_;
  }
  
  virtual const std::string& getContent() const {
    return content_;
  }
  
  virtual void addHandle(const std::string& _handle) {
    handles_.push_back(_handle);
  }
  
  virtual unsigned getHandleSize() const {
    return handles_.size();
  }
  
  virtual const std::string& getHandle(unsigned _idx) const {
    return handles_.at(_idx);
  }
  
  virtual void addAttachment(const Fred::OID& _oid) {
    attachments_.push_back(_oid);
  }
  
  virtual unsigned getAttachmentSize() const {
    return attachments_.size();
  }
  
  virtual const Fred::OID& getAttachment(unsigned _idx) const {
    return attachments_.at(_idx);
  }
};

COMPARE_CLASS_IMPL(MailImpl, CreateTime)
COMPARE_CLASS_IMPL(MailImpl, Type)
COMPARE_CLASS_IMPL(MailImpl, Status)


class ListImpl : public Fred::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;
  
public:
  ListImpl(Manager *_manager) : CommonListImpl(),
                                                          manager_(_manager) {
  }
  
  virtual Mail* get(unsigned _idx) const {
    try {
      Mail *mail = dynamic_cast<Mail* >(data_.at(_idx));
      if (mail) 
        return mail;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    }
  }
  
  virtual void reload(Database::Filters::Union& _filter) {
    TRACE("[CALL] Fred::Mail::ListImpl::reload()");
    clear();
    _filter.clearQueries();
    
    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::Mail *mf = dynamic_cast<Database::Filters::Mail* >(*fit);
      if (!mf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("id", mf->joinMailTable(), "DISTINCT"));
      _filter.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    id_query.order_by() << "id DESC";
    id_query.limit(load_limit_);
    _filter.serialize(id_query);
    
    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, t_1.mailtype, t_1.crdate, t_1.moddate, "
                               << "t_1.status, t_1.message, t_2.name";
    object_info_query.from() << getTempTableName() << " tmp "
                             << "JOIN mail_archive t_1 ON (tmp.id = t_1.id) "
                             << "JOIN mail_type t_2 ON (t_1.mailtype = t_2.id)";
    object_info_query.order_by() << "tmp.id";
    try {
      fillTempTable(tmp_table_query);
      
      Database::Connection conn = Database::Manager::acquire();
      Database::Result r_info = conn.exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID       id        = *col;
        long               type      = *(++col);
        Database::DateTime crdate    = *(++col);
        Database::DateTime moddate   = *(++col);
        long               status    = *(++col);
        std::string        msg       = *(++col);
        std::string        type_desc = *(++col);
        
        data_.push_back(new MailImpl(id,
                                     crdate,
                                     moddate,
                                     type,
                                     type_desc,
                                     status,
                                     msg));
      }
      
      if (data_.empty()) {
        return;
      }
      
      /*
       * load objects details for mail
       * 
       * object handles associated with mail
       */      
      resetIDSequence();
      
      Database::SelectQuery handles_query;
      handles_query.select() << "t_1.mailid, t_1.associd";
      handles_query.from() << getTempTableName() << " tmp "
                           << "JOIN mail_handles t_1 ON (tmp.id = t_1.mailid)";
      handles_query.order_by() << "tmp.id";
      
      Database::Result r_handles = conn.exec(handles_query);
      for (Database::Result::Iterator it = r_handles.begin(); it != r_handles.end(); ++it) {
        Database::ID mail_id = (*it)[0];
        std::string  handle  = (*it)[1];
             
        MailImpl *mail_ptr = dynamic_cast<MailImpl *>(findIDSequence(mail_id));
        if (mail_ptr)
          mail_ptr->addHandle(handle);
      }
      
      /*
       * file attachments
       */      
      resetIDSequence();
      
      Database::SelectQuery files_query;
      files_query.select() << "t_1.mailid, t_1.attachid, t_2.name";
      files_query.from() << getTempTableName() << " tmp "
                         << "JOIN mail_attachments t_1 ON (tmp.id = t_1.mailid) "
                         << "JOIN files t_2 ON (t_1.attachid = t_2.id)";
      files_query.order_by() << "tmp.id";
      
      Database::Result r_files = conn.exec(files_query);
      for (Database::Result::Iterator it = r_files.begin(); it != r_files.end(); ++it) {
        Database::ID mail_id   = (*it)[0];
        Database::ID file_id   = (*it)[1];
        std::string file_name  = (*it)[2];
             
        MailImpl *mail_ptr = dynamic_cast<MailImpl *>(findIDSequence(mail_id));
        if (mail_ptr)
          mail_ptr->addAttachment(Fred::OID(file_id, file_name, FT_FILE));
      }
      /* checks if row number result load limit is active and set flag */ 
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
        std::string message = ex.what();
        if (message.find(Database::Connection::getTimeoutString())
                != std::string::npos) {
            LOGGER(PACKAGE).info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }
  }
  
  virtual void sort(MemberType _member, bool _asc) {
    switch(_member) {
      case MT_CRDATE:
        stable_sort(data_.begin(), data_.end(), CompareCreateTime(_asc));
        break;
      case MT_TYPE:
        stable_sort(data_.begin(), data_.end(), CompareType(_asc));
        break;
      case MT_STATUS:
        stable_sort(data_.begin(), data_.end(), CompareStatus(_asc));
        break;
    }
  }
  
  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const {
    return "tmp_mail_filter_result";
  }
  
  virtual void makeQuery(bool, bool, std::stringstream&) const {
  }
  
  virtual void reload() {
  }
};

class ManagerImpl : virtual public Manager {
public:
  ManagerImpl() {
  }
  
  virtual ~ManagerImpl() {
  }  
  
  List* createList() const {
    return new ListImpl((Manager *)this);
  }
  
};

Manager* Manager::create() {
    TRACE("[CALL] Fred::Mail::Manager::create()");
    return new ManagerImpl();
}


}
}
