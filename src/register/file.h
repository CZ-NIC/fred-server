#ifndef _FILE_H_
#define _FILE_H_

#include "common_impl_new.h"
#include "db_settings.h"
#include "model/model_filters.h"
#include "model_files.h"

namespace Register {
namespace File {

enum MemberType {
  MT_NAME,
  MT_CRDATE,
  MT_TYPE,
  MT_SIZE
};

class File:
    public ModelFiles,
    virtual public Register::CommonObjectImplNew {
private:
    std::string m_typeDesc;
public:
    File():
        CommonObjectImplNew(),
        ModelFiles()
    { }
    bool save();
    void setFileTypeDesc(const std::string &desc);
    const std::string &getFileTypeDesc() const;
};

class List: 
    virtual public Register::CommonListImplNew {
public:
    void reload(Database::Filters::Union &filter);
    File *get(unsigned int index) const throw (std::exception);
    const char* getTempTableName() const;
    void sort(MemberType member, bool asc);
};

class Manager {
public:
    virtual ~Manager() {
    }

    virtual File *createFile() const = 0;
    virtual List *createList() const = 0;
    static Manager *create();
};

}
}

#endif // _FILE_H_
