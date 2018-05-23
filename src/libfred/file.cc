#include "src/libfred/file.hh"
#include "src/util/log/logger.hh"

#include <algorithm>
#include <boost/utility.hpp>

namespace LibFred {
namespace File {

unsigned long long File::getId() const
{
    return id_;
}
const std::string& File::getName() const
{
    return name_;
}
const std::string& File::getPath() const
{
    return path_;
}
const std::string& File::getMimeType() const
{
    return mimeType_;
}
const Database::DateTime& File::getCrDate() const
{
    return crDate_;
}
unsigned long long File::getFilesize() const
{
    return filesize_;
}
unsigned long long File::getFileTypeId() const
{
    return fileTypeId_;
}
const std::string &File::getFileTypeDesc() const
{
    return fileTypeDesc_;
}

COMPARE_CLASS_IMPL_NEW(File, CrDate);
COMPARE_CLASS_IMPL_NEW(File, Name);
COMPARE_CLASS_IMPL_NEW(File, FileTypeId);
COMPARE_CLASS_IMPL_NEW(File, Filesize);

const char *
List::getTempTableName() const
{
    return "tmp_file_filter_result";
}

void
List::reload(Database::Filters::Union &filter)
{
    TRACE("[CALL] LibFred::File::ListImpl::reload()");
    clear();
    filter.clearQueries();

    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator fit = filter.begin();
    for (; fit != filter.end(); ++fit) {
        Database::Filters::File *ff = dynamic_cast<Database::Filters::File* >(*fit);
        if (!ff)
            continue;
        Database::SelectQuery *tmp = new Database::SelectQuery();
        tmp->addSelect(new Database::Column("id", ff->joinFileTable()));
        filter.addQuery(tmp);
    }

    id_query.order_by() << "id DESC";
    if(getLimit() != 0) id_query.limit(getLimit());
    filter.serialize(id_query);

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

    Database::Connection conn = Database::Manager::acquire();
    try {
        fillTempTable(tmp_table_query);

        Database::Result r_info = conn.exec(object_info_query);
        for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            unsigned long long id       = *col;
            std::string        name     = *(++col);
            std::string        path     = *(++col);
            std::string        mimetype = *(++col);
            Database::DateTime crdate   = *(++col);
            unsigned long long size     = *(++col);
            unsigned long long type     = *(++col);
            std::string        typedesc = *(++col);

            File *file = new File(id, name, path, mimetype, crdate, size, type, typedesc);

            appendToList(file);
        }
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

File *
List::get(unsigned int index) const
{
    try {
        File *file = dynamic_cast<File *>(CommonListImplNew::get(index));
        if (file) {
            return file;
        } else {
            throw std::exception();
        }
    } catch (...) {
        throw std::exception();
    }
}

void
List::sort(MemberType member, bool asc)
{
    switch (member) {
        case MT_NAME:
            stable_sort(m_data.begin(), m_data.end(), CompareName(asc));
            break;
        case MT_CRDATE:
            stable_sort(m_data.begin(), m_data.end(), CompareCrDate(asc));
            break;
        case MT_TYPE:
            stable_sort(m_data.begin(), m_data.end(), CompareFileTypeId(asc));
            break;
        case MT_SIZE:
            stable_sort(m_data.begin(), m_data.end(), CompareFilesize(asc));
            break;
    }    
}

LibFred::File::File* List::findId(Database::ID _id)
{
	  std::vector<LibFred::CommonObjectNew*>::const_iterator it = std::find_if(m_data.begin(),
	  m_data.end(),
	  CheckIdNew<LibFred::File::File>(_id));

	  if (it != m_data.end())
	  {
		  LOGGER(PACKAGE).debug(boost::format("object list hit! object id=%1% found")
		  % _id);
		  return dynamic_cast<LibFred::File::File*>(*it);
	  }
	  LOGGER(PACKAGE).debug(boost::format("object list miss! object id=%1% not found")
	  % _id);
	  throw LibFred::NOT_FOUND();
}


class ManagerImpl: virtual public Manager
{
private:
    Transferer *transferer_;


public:
    ManagerImpl(Transferer *_transferer)
              : transferer_(_transferer)
    {
    }

    virtual ~ManagerImpl()
    {
    }

    File *createFile() const
    {
        TRACE("[CALL] LibFred::File::ManagerImpl::createFile()");
        File *file = new File();
        return file;
    }

    List* createList() const 
    {
        return new List();
    }

    unsigned long long upload(const std::string &_name,
                              const std::string &_mime_type,
                              const unsigned int &_file_type)
    {
        Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("ManagerImpl::upload _name: ")
        +_name + std::string(" _mime_type: ") + _mime_type + std::string(" _file_type: ")
         + boost::lexical_cast<std::string>(_file_type));

        return transferer_->upload(_name, _mime_type, _file_type);
    }

    void download(const unsigned long long _id,
                  std::vector<char> &_buffer)
    {
        transferer_->download(_id, _buffer);
    }

};

Manager* Manager::create(Transferer *_transferer)
{
    TRACE("[CALL] LibFred::File::Manager::create()");
    return new ManagerImpl(_transferer);
};

}
}
