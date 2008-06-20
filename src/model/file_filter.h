#ifndef FILE_FILTER_H_
#define FILE_FILTER_H_

#include "db/base_filters.h"

namespace DBase {
namespace Filters {

class File : virtual public Compound {
public:
  virtual ~File() {
  }
  
  virtual Table& joinFileTable() = 0;
  virtual Value<DBase::ID>& addId() = 0;
  virtual Value<std::string>& addName() = 0;
  virtual Value<std::string>& addPath() = 0;
  virtual Value<std::string>& addMimeType() = 0;
  virtual Interval<DBase::DateTimeInterval>& addCreateTime() = 0;
  virtual Value<int>& addSize() = 0;
  virtual Value<int>& addType() = 0;
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class FileImpl : virtual public File {
public:
  FileImpl(bool _set_active = false);
  virtual ~FileImpl();
  
  virtual Table& joinFileTable();
  virtual Value<DBase::ID>& addId();
  virtual Value<std::string>& addName();
  virtual Value<std::string>& addPath();
  virtual Value<std::string>& addMimeType();
  virtual Interval<DBase::DateTimeInterval>& addCreateTime();
  virtual Value<int>& addSize();
  virtual Value<int>& addType();
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(File);
  }

};


}
}


#endif /*FILE_FILTER_H_*/
