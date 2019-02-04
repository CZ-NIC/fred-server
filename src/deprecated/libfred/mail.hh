#ifndef MAIL_HH_8C9378EE39F54B9695A7961ED83CA3DA
#define MAIL_HH_8C9378EE39F54B9695A7961ED83CA3DA

#include "src/deprecated/libfred/common_object.hh"
#include "src/deprecated/libfred/object.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"

namespace LibFred {
namespace Mail {

enum MemberType {
  MT_CRDATE,
  MT_TYPE,
  MT_STATUS
};


class Mail : virtual public LibFred::CommonObject {
public:
  virtual const Database::DateTime& getCreateTime() const = 0;
  virtual const Database::DateTime& getModTime() const = 0;
  virtual const long &getType() const = 0;
  virtual const std::string& getTypeDesc() const = 0;
  virtual const long &getStatus() const = 0;
  virtual const std::string& getContent() const = 0;
  
  virtual void addHandle(const std::string& _handle) = 0;
  virtual unsigned getHandleSize() const = 0;
  virtual const std::string& getHandle(unsigned _idx) const = 0;
  
  virtual void addAttachment(const LibFred::OID& _oid) = 0;
  virtual unsigned getAttachmentSize() const = 0;
  virtual const LibFred::OID& getAttachment(unsigned _idx) const = 0;
};


class List : virtual public LibFred::CommonList {
public:
  virtual Mail* get(unsigned _idx) const = 0;
  virtual void reload(Database::Filters::Union& _filter) = 0;
  virtual void sort(MemberType _member, bool _asc) = 0;
  
  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const = 0;
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual void reload() = 0;
};


class Manager {
public:
  virtual ~Manager() {
  }
  
  virtual List* createList() const = 0;
  static Manager* create();
};

}
}

#endif /*MAIL_H_*/
