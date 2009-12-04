#ifndef _MODEL_FILES_H_
#define _MODEL_FILES_H_

#include "db_settings.h"
#include "model.h"

class ModelFiles:
    public Model::Base {
public:
    ModelFiles()
    { }
    virtual ~ModelFiles()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const std::string &getName() const
    {
        return m_name.get();
    }
    const std::string &getPath() const
    {
        return m_path.get();
    }
    const std::string &getMimeType() const
    {
        return m_mimeType.get();
    }
    const Database::DateTime &getCrDate() const
    {
        return m_crDate.get();
    }
    const int &getFilesize() const
    {
        return m_filesize.get();
    }
    const unsigned long long &getFileTypeId() const
    {
        return m_fileTypeId.get();
    }
    /*
    ModelEnumFileType *getFileType()
    {
        return fileType.getRelated(this);
    }
    */
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setName(const std::string &name)
    {
        m_name = name;
    }
    void setPath(const std::string &path)
    {
        m_path = path;
    }
    void setMimeType(const std::string &mimeType)
    {
        m_mimeType = mimeType;
    }
    void setCrDate(const Database::DateTime &crDate)
    {
        m_crDate = crDate;
    }
    void setFilesize(const int &filesize)
    {
        m_filesize = filesize;
    }
    void setFileTypeId(const unsigned long long &fileTypeId)
    {
        m_fileTypeId = fileTypeId;
    }
    /*
    void setFileType(ModelEnumFileType *foreign_value)
    {
        fileType.setRelated(this, foreign_value);
    }
     */
    friend class Model::Base;

    void insert()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::insert(this);
        tx.commit();
    }

    void update()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::update(this);
        tx.commit();
    }

    void reload()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::reload(this);
        tx.commit();
    }

    void load(const Database::Row &_data)
    {
        Model::Base::load(this, _data);
    }

    typedef Model::Field::List<ModelFiles>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_name;
    Field::Field<std::string> m_path;
    Field::Field<std::string> m_mimeType;
    Field::Field<Database::DateTime> m_crDate;
    Field::Field<int> m_filesize;
    Field::Field<unsigned long long> m_fileTypeId;

    //Field::Lazy::Field<ModelEnumFileType *> m_fileType;

public:
    static Model::Field::PrimaryKey<ModelFiles, unsigned long long> id;
    static Model::Field::Basic<ModelFiles, std::string> name;
    static Model::Field::Basic<ModelFiles, std::string> path;
    static Model::Field::Basic<ModelFiles, std::string> mimeType;
    static Model::Field::Basic<ModelFiles, Database::DateTime> crDate;
    static Model::Field::Basic<ModelFiles, int> filesize;
    static Model::Field::Basic<ModelFiles, unsigned long long> fileTypeId;

    //static Model::Field::Related::OneToOne<ModelFiles, unsigned long long, ModelEnumFileType> fileType;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelFiles

#endif // _MODEL_FILES_H_

