#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "file.h"
#include "log/logger.h"


namespace Register {
namespace File {

class FileImpl : public Register::CommonObjectImpl,
                 virtual public File {
private:
  std::string name_;
  std::string path_;
  std::string mimetype_;
  unsigned type_;
  std::string type_desc_;
  DBase::DateTime create_time_;
  unsigned long size_;
  
public:
  FileImpl(DBase::ID _id, const std::string& _name, const std::string& _path,
           const std::string& _mimetype, unsigned _type, 
           const std::string& _type_desc, const DBase::DateTime& _create_time,
           const unsigned long _size) : CommonObjectImpl(_id),
                                        name_(_name),
                                        path_(_path),
                                        mimetype_(_mimetype),
                                        type_(_type),
                                        type_desc_(_type_desc),
                                        create_time_(_create_time),
                                        size_(_size) {
  }
  
  virtual const std::string& getName() const {
    return name_;
  }
  
  virtual const std::string& getPath() const {
    return path_;
  }
  
  virtual const std::string& getMimeType() const {
    return mimetype_;
  }
  
  virtual const unsigned getType() const {
    return type_;
  }
  
  virtual const std::string& getTypeDesc() const {
    return type_desc_;
  }
  
  virtual const DBase::DateTime& getCreateTime() const {
    return create_time_;
  }
  
  virtual const unsigned long getSize() const {
    return size_;
  }
};


class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;
  
public:
  ListImpl(DBase::Connection *_conn, Manager *_manager) : CommonListImpl(_conn),
                                                          manager_(_manager) {
  }
  
  virtual File* get(unsigned _idx) const {
    try {
      File *file = dynamic_cast<File* >(data_.at(_idx));
      if (file) 
        return file;
      else
        throw std::exception();
    }
    catch (...) {
      throw std::exception();
    }
  }

  virtual void reload(DBase::Filters::Union& _filter) {
    TRACE("[CALL] Register::File::ListImpl::reload()");
    clear();
    _filter.clearQueries();
    
    DBase::SelectQuery id_query;
    DBase::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      DBase::Filters::File *ff = dynamic_cast<DBase::Filters::File* >(*fit);
      if (!ff)
        continue;
      DBase::SelectQuery *tmp = new DBase::SelectQuery();
      tmp->addSelect(new DBase::Column("id", ff->joinFileTable()));
      _filter.addQuery(tmp);
    }
    id_query.limit(load_limit_);
    _filter.serialize(id_query);
    
    DBase::InsertQuery tmp_table_query = DBase::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER("db").debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    DBase::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, t_1.name, t_1.path, t_1.mimetype, "
                               << "t_1.crdate, t_1.filesize, t_1.filetype, t_2.name";
    object_info_query.from() << getTempTableName() << " tmp "
                             << "JOIN files t_1 ON (tmp.id = t_1.id) "
                             << "JOIN enum_filetype t_2 ON (t_1.filetype = t_2.id)";
    object_info_query.order_by() << "tmp.id";
    
    try {
      fillTempTable(tmp_table_query);
      
      std::auto_ptr<DBase::Result> r_info(conn_->exec(object_info_query));
      std::auto_ptr<DBase::ResultIterator> it(r_info->getIterator());
      for (it->first(); !it->isDone(); it->next()) {
        DBase::ID id = it->getNextValue();
        std::string name = it->getNextValue();
        std::string path = it->getNextValue();
        std::string mimetype = it->getNextValue();
        DBase::DateTime crdate = it->getNextValue();
        unsigned long size = it->getNextValue();
        unsigned type = it->getNextValue();
        std::string typedesc = it->getNextValue();
        
        data_.push_back(new FileImpl(id,
                                     name,
                                     path,
                                     mimetype,
                                     type,
                                     typedesc,
                                     crdate,
                                     size
                                     ));
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
    
  }
  
  
  
  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const {
    return "tmp_file_filter_result";
  }
  
  virtual void makeQuery(bool, bool, std::stringstream&) const {
    /* dummy implementation */
  }
  
  virtual void reload() {
    /* dummy implementation */
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
    TRACE("[CALL] Register::File::Manager::create()");
    return new ManagerImpl(_db_manager);
};

}
}
