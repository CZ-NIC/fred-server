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
  Database::DateTime create_time_;
  unsigned long size_;
  
public:
  FileImpl(Database::ID _id, const std::string& _name, const std::string& _path,
           const std::string& _mimetype, unsigned _type, 
           const std::string& _type_desc, const Database::DateTime& _create_time,
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
  
  virtual const Database::DateTime& getCreateTime() const {
    return create_time_;
  }
  
  virtual const unsigned long getSize() const {
    return size_;
  }
};

COMPARE_CLASS_IMPL(FileImpl, CreateTime)
COMPARE_CLASS_IMPL(FileImpl, Name)
COMPARE_CLASS_IMPL(FileImpl, Type)
COMPARE_CLASS_IMPL(FileImpl, Size)


class ListImpl : public Register::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;
  
public:
  ListImpl(Database::Connection *_conn, Manager *_manager) : CommonListImpl(_conn),
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

  virtual void reload(Database::Filters::Union& _filter) {
    TRACE("[CALL] Register::File::ListImpl::reload()");
    clear();
    _filter.clearQueries();
    
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::File *ff = dynamic_cast<Database::Filters::File* >(*fit);
      if (!ff)
        continue;
      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("id", ff->joinFileTable()));
      _filter.addQuery(tmp);
    }
    id_query.limit(load_limit_);
    _filter.serialize(id_query);
    
    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select() << "t_1.id, t_1.name, t_1.path, t_1.mimetype, "
                               << "t_1.crdate, t_1.filesize, t_1.filetype, t_2.name";
    object_info_query.from() << getTempTableName() << " tmp "
                             << "JOIN files t_1 ON (tmp.id = t_1.id) "
                             << "JOIN enum_filetype t_2 ON (t_1.filetype = t_2.id)";
    object_info_query.order_by() << "tmp.id";
    
    try {
      fillTempTable(tmp_table_query);
      
      Database::Result r_info = conn_->exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID       id       = *col;
        std::string        name     = *(++col);
        std::string        path     = *(++col);
        std::string        mimetype = *(++col);
        Database::DateTime crdate   = *(++col);
        unsigned long      size     = *(++col);
        unsigned           type     = *(++col);
        std::string        typedesc = *(++col);
        
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
      /* checks if row number result load limit is active and set flag */ 
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }
  }
  
  virtual void sort(MemberType _member, bool _asc) {
    switch (_member) {
      case MT_NAME:
        stable_sort(data_.begin(), data_.end(), CompareName(_asc));
        break;
      case MT_CRDATE:
        stable_sort(data_.begin(), data_.end(), CompareCreateTime(_asc));
        break;
      case MT_TYPE:
        stable_sort(data_.begin(), data_.end(), CompareType(_asc));
        break;
      case MT_SIZE:
        stable_sort(data_.begin(), data_.end(), CompareSize(_asc));
        break;
    }    
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
  Database::Manager *db_manager_;
  Database::Connection *conn_;
  
public:
  ManagerImpl(Database::Manager *_db_manager) : db_manager_(_db_manager),
                                             conn_(db_manager_->getConnection()) {
  }
  
  virtual ~ManagerImpl() {
    boost::checked_delete<Database::Connection>(conn_);
  }  
  
  List* createList() const {
    return new ListImpl(conn_, (Manager *)this);
  }
  
};

Manager* Manager::create(Database::Manager *_db_manager) {
    TRACE("[CALL] Register::File::Manager::create()");
    return new ManagerImpl(_db_manager);
};

}
}
