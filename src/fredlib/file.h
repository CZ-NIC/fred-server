#ifndef _FILE_H_
#define _FILE_H_

#include "common_impl_new.h"
#include "db_settings.h"
#include "file_transferer.h"
#include "model/model_filters.h"
#include "model_files.h"

namespace Fred {
namespace File {

enum MemberType {
  MT_NAME,
  MT_CRDATE,
  MT_TYPE,
  MT_SIZE
};

class File:
    public ModelFiles,
    virtual public Fred::CommonObjectImplNew {
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
    virtual public Fred::CommonListImplNew {
public:
    void reload(Database::Filters::Union &filter);
    File *get(unsigned int index) const throw (std::exception);
    const char* getTempTableName() const;
    void sort(MemberType member, bool asc);
    Fred::File::File* findId(Database::ID _id);

};

class Manager {
public:
    virtual ~Manager() {
    }

    virtual File *createFile() const = 0;

    virtual List *createList() const = 0;

    virtual unsigned long long upload(const std::string &_name,
                                      const std::string &_mime_type,
                                      const unsigned int &_file_type) = 0;

    virtual void download(const unsigned long long _id,
                  std::vector<char> &_buffer) = 0;


    static Manager* create(Transferer *_transferer);
};

typedef std::auto_ptr<Manager> ManagerPtr;

}
}

#endif // _FILE_H_
