#ifndef FILE_HH_4C69AF92C64E465AB978B45C57FF04E3
#define FILE_HH_4C69AF92C64E465AB978B45C57FF04E3

#include "src/libfred/common_impl_new.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/file_transferer.hh"
#include "src/deprecated/model/model_filters.hh"
#include "src/libfred/model_files.hh"

namespace LibFred {
namespace File {

enum MemberType {
  MT_NAME,
  MT_CRDATE,
  MT_TYPE,
  MT_SIZE
};

class File:
    public ModelFiles,
    virtual public LibFred::CommonObjectImplNew {
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
    virtual public LibFred::CommonListImplNew {
public:
    void reload(Database::Filters::Union &filter);
    File *get(unsigned int index) const;
    const char* getTempTableName() const;
    void sort(MemberType member, bool asc);
    LibFred::File::File* findId(Database::ID _id);

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

typedef std::unique_ptr<Manager> ManagerPtr;

}
}

#endif
