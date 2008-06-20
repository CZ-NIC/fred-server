#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "mail.h"
#include "log/logger.h"


namespace Register {
namespace Mail {

class MailImpl : public Register::CommonObjectImpl,
                 virtual public Mail {
private:
  DBase::DateTime create_time_;
  DBase::DateTime mod_time_;
  long type_;
  std::string type_desc_;
  long status_;
  std::string content_;
  std::vector<std::string> handles_;
  std::vector<DBase::ID> attachments_;   

public:
  MailImpl(DBase::ID& _id, DBase::DateTime& _create_time, DBase::DateTime& _mod_time,
           long _type, std::string& _type_desc, long _status, std::string& _content) :
             CommonObjectImpl(_id),
             create_time_(_create_time),
             mod_time_(_mod_time),
             type_(_type),
             type_desc_(_type_desc),
             status_(_status),
             content_(_content) {
  }
  
  virtual const DBase::DateTime& getCreateTime() const {
    return create_time_;
  }
  
  virtual const DBase::DateTime& getModTime() const {
    return mod_time_;
  }
  
  virtual const long getType() const {
    return type_;
  }
  
  virtual const std::string& getTypeDesc() const {
    return type_desc_;
  }
  
  virtual const long getStatus() const {
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
  
  virtual void addAttachment(const DBase::ID& _id) {
    attachments_.push_back(_id);
  }
  
  virtual unsigned getAttachmentSize() const {
    return attachments_.size();
  }
  
  virtual const DBase::ID& getAttachment(unsigned _idx) const {
    return attachments_.at(_idx);
  }
};

COMPARE_CLASS_IMPL(MailImpl, CreateTime)
COMPARE_CLASS_IMPL(MailImpl, Type)
COMPARE_CLASS_IMPL(MailImpl, Status)


class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;
  
public:
  ListImpl(DBase::Connection *_conn, Manager *_manager) : CommonListImpl(_conn),
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
  
  virtual void reload(DBase::Filters::Union& _filter) {
    TRACE("[CALL] Register::Mail::ListImpl::reload()");
    clear();
    _filter.clearQueries();
    
    DBase::SelectQuery id_query;
    DBase::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      DBase::Filters::Mail *mf = dynamic_cast<DBase::Filters::Mail* >(*fit);
      if (!mf)
        continue;
      DBase::SelectQuery *tmp = new DBase::SelectQuery();
      tmp->addSelect(new DBase::Column("id", mf->joinMailTable(), "DISTINCT"));
      _filter.addQuery(tmp);
    }
    id_query.limit(load_limit_);
    _filter.serialize(id_query);
    
    DBase::InsertQuery tmp_table_query = DBase::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER("db").debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    DBase::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, t_1.mailtype, t_1.crdate, t_1.moddate, "
                               << "t_1.status, t_1.message, t_2.name";
    object_info_query.from() << getTempTableName() << " tmp "
                             << "JOIN mail_archive t_1 ON (tmp.id = t_1.id) "
                             << "JOIN mail_type t_2 ON (t_1.mailtype = t_2.id)";
    object_info_query.order_by() << "tmp.id";
    try {
      fillTempTable(tmp_table_query);
      
      std::auto_ptr<DBase::Result> r_info(conn_->exec(object_info_query));
      std::auto_ptr<DBase::ResultIterator> it(r_info->getIterator());
      for (it->first(); !it->isDone(); it->next()) {
        DBase::ID id = it->getNextValue();
        long type = (long)it->getNextValue();
        DBase::DateTime crdate = it->getNextValue();
        DBase::DateTime moddate = it->getNextValue();
        long status = (long)it->getNextValue();
        std::string msg = it->getNextValue();
        std::string type_desc = it->getNextValue();
        
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
      
      DBase::SelectQuery handles_query;
      handles_query.select() << "t_1.mailid, t_1.associd";
      handles_query.from() << getTempTableName() << " tmp "
                          << "JOIN mail_handles t_1 ON (tmp.id = t_1.mailid)";
      handles_query.order_by() << "tmp.id";
      
      std::auto_ptr<DBase::Result> r_handles(conn_->exec(handles_query));
      std::auto_ptr<DBase::ResultIterator> hit(r_handles->getIterator());
      for (hit->first(); !hit->isDone(); hit->next()) {
        DBase::ID mail_id = hit->getNextValue();
        std::string handle = hit->getNextValue();
             
        MailImpl *mail_ptr = dynamic_cast<MailImpl *>(findIDSequence(mail_id));
        if (mail_ptr)
          mail_ptr->addHandle(handle);
      }
      
      /*
       * file attachments
       */      
      resetIDSequence();
      
      DBase::SelectQuery files_query;
      files_query.select() << "t_1.mailid, t_1.attachid";
      files_query.from() << getTempTableName() << " tmp "
                          << "JOIN mail_attachments t_1 ON (tmp.id = t_1.mailid)";
      files_query.order_by() << "tmp.id";
      
      std::auto_ptr<DBase::Result> r_files(conn_->exec(files_query));
      std::auto_ptr<DBase::ResultIterator> ait(r_files->getIterator());
      for (ait->first(); !ait->isDone(); ait->next()) {
        DBase::ID mail_id = ait->getNextValue();
        DBase::ID file_id = ait->getNextValue();
             
        MailImpl *mail_ptr = dynamic_cast<MailImpl *>(findIDSequence(mail_id));
        if (mail_ptr)
          mail_ptr->addAttachment(file_id);
      }
    }
    catch (DBase::Exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
      clear();
    }
    catch (std::exception& ex) {
      LOGGER("db").error(boost::format("%1%") % ex.what());
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
private:
  DBase::Manager *db_manager_;
  DBase::Connection *conn_;
  
public:
  ManagerImpl(DBase::Manager *_db_manager) : db_manager_(_db_manager),
                                             conn_(db_manager_->getConnection()) {
  }
  
  virtual ~ManagerImpl() {
    boost::checked_delete<DBase::Connection>(conn_);
  }  
  
  List* createList() const {
    return new ListImpl(conn_, (Manager *)this);
  }
  
};

Manager* Manager::create(DBase::Manager *_db_manager) {
    TRACE("[CALL] Register::Mail::Manager::create()");
    return new ManagerImpl(_db_manager);
}


}
}
