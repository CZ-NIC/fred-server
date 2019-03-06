/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef FILE_HH_4C69AF92C64E465AB978B45C57FF04E3
#define FILE_HH_4C69AF92C64E465AB978B45C57FF04E3

#include "src/deprecated/libfred/common_impl_new.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/file_transferer.hh"

namespace LibFred {
namespace File {

enum MemberType {
  MT_NAME,
  MT_CRDATE,
  MT_TYPE,
  MT_SIZE
};

class File:
    virtual public LibFred::CommonObjectImplNew {
public:
    File(unsigned long long _id,
            const std::string& _name,
            const std::string& _path,
            const std::string& _mimeType,
            const Database::DateTime& _crDate,
            unsigned long long _filesize,
            unsigned long long _fileTypeId,
            const std::string& _fileTypeDesc):
        CommonObjectImplNew(),
        id_(_id),
        name_(_name),
        path_(_path),
        mimeType_(_mimeType),
        crDate_(_crDate),
        filesize_(_filesize),
        fileTypeId_(_fileTypeId),
        fileTypeDesc_(_fileTypeDesc)
    {
    }

    unsigned long long getId() const;
    const std::string& getName() const;
    const std::string& getPath() const;
    const std::string& getMimeType() const;
    const Database::DateTime& getCrDate() const;
    unsigned long long getFilesize() const;
    unsigned long long getFileTypeId() const;
    const std::string& getFileTypeDesc() const;

private:
    unsigned long long id_;
    std::string name_;
    std::string path_;
    std::string mimeType_;
    Database::DateTime crDate_;
    unsigned long long filesize_;
    unsigned long long fileTypeId_;
    std::string fileTypeDesc_;
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
