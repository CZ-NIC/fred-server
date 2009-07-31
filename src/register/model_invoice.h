#ifndef _MODEL_INVOICE_H_
#define _MODEL_INVOICE_H_

#include "db_settings.h"
#include "model.h"
#include "model_zone.h"
#include "model_registrar.h"
#include "model_invoiceprefix.h"
#include "model_files.h"
#include "model_files.h"


class ModelInvoice:
    public Model::Base {
public:
    ModelInvoice()
    { }
    virtual ~ModelInvoice()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getZoneId() const
    {
        return m_zoneId.get();
    }
    ModelZone *getZone()
    {
        return zone.getRelated(this);
    }
    const Database::DateTime &getCrDate() const
    {
        return m_crDate.get();
    }
    const Database::Date &getTaxDate() const
    {
        return m_taxDate.get();
    }
    const unsigned long long &getPrefix() const
    {
        return m_prefix.get();
    }
    const unsigned long long &getRegistrarId() const
    {
        return m_registrarId.get();
    }
    ModelRegistrar *getRegistrar()
    {
        return registrar.getRelated(this);
    }
    const Database::Money &getCredit() const
    {
        return m_credit.get();
    }
    const Database::Money &getPrice() const
    {
        return m_price.get();
    }
    const int &getVat() const
    {
        return m_vat.get();
    }
    const Database::Money &getTotal() const
    {
        return m_total.get();
    }
    const Database::Money &getTotalVat() const
    {
        return m_totalVat.get();
    }
    const unsigned long long &getPrefixTypeId() const
    {
        return m_prefixTypeId.get();
    }
    ModelInvoicePrefix *getPrefixType()
    {
        return prefixType.getRelated(this);
    }
    const unsigned long long &getFileId() const
    {
        return m_fileId.get();
    }
    ModelFiles *getFile()
    {
        return file.getRelated(this);
    }
    const unsigned long long &getFileXmlId() const
    {
        return m_fileXmlId.get();
    }
    ModelFiles *getFileXml()
    {
        return fileXml.getRelated(this);
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setZoneId(const unsigned long long &zoneId)
    {
        m_zoneId = zoneId;
    }
    void setZone(ModelZone *foreign_value)
    {
        zone.setRelated(this, foreign_value);
    }
    void setCrDate(const Database::DateTime &crDate)
    {
        m_crDate = crDate;
    }
    void setTaxDate(const Database::Date &taxDate)
    {
        m_taxDate = taxDate;
    }
    void setPrefix(const unsigned long long &prefix)
    {
        m_prefix = prefix;
    }
    void setRegistrarId(const unsigned long long &registrarId)
    {
        m_registrarId = registrarId;
    }
    void setRegistrar(ModelRegistrar *foreign_value)
    {
        registrar.setRelated(this, foreign_value);
    }
    void setCredit(const Database::Money &credit)
    {
        m_credit = credit;
    }
    void setPrice(const Database::Money &price)
    {
        m_price = price;
    }
    void setVat(const int &vat)
    {
        m_vat = vat;
    }
    void setTotal(const Database::Money &total)
    {
        m_total = total;
    }
    void setTotalVat(const Database::Money &totalVat)
    {
        m_totalVat = totalVat;
    }
    void setPrefixTypeId(const unsigned long long &prefixTypeId)
    {
        m_prefixTypeId = prefixTypeId;
    }
    void setPrefixType(ModelInvoicePrefix *foreign_value)
    {
        prefixType.setRelated(this, foreign_value);
    }
    void setFileId(const unsigned long long &fileId)
    {
        m_fileId = fileId;
    }
    void setFile(ModelFiles *foreign_value)
    {
        file.setRelated(this, foreign_value);
    }
    void setFileXmlId(const unsigned long long &fileXmlId)
    {
        m_fileXmlId = fileXmlId;
    }
    void setFileXml(ModelFiles *foreign_value)
    {
        fileXml.setRelated(this, foreign_value);
    }

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

    typedef Model::Field::List<ModelInvoice>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_zoneId;
    Field::Field<Database::DateTime> m_crDate;
    Field::Field<Database::Date> m_taxDate;
    Field::Field<unsigned long long> m_prefix;
    Field::Field<unsigned long long> m_registrarId;
    Field::Field<Database::Money> m_credit;
    Field::Field<Database::Money> m_price;
    Field::Field<int> m_vat;
    Field::Field<Database::Money> m_total;
    Field::Field<Database::Money> m_totalVat;
    Field::Field<unsigned long long> m_prefixTypeId;
    Field::Field<unsigned long long> m_fileId;
    Field::Field<unsigned long long> m_fileXmlId;

    Field::Lazy::Field<ModelZone *> m_zone;
    Field::Lazy::Field<ModelRegistrar *> m_registrar;
    Field::Lazy::Field<ModelInvoicePrefix *> m_prefixType;
    Field::Lazy::Field<ModelFiles *> m_file;
    Field::Lazy::Field<ModelFiles *> m_fileXml;

public:
    static Model::Field::PrimaryKey<ModelInvoice, unsigned long long> id;
    static Model::Field::ForeignKey<ModelInvoice, unsigned long long, ModelZone> zoneId;
    static Model::Field::Basic<ModelInvoice, Database::DateTime> crDate;
    static Model::Field::Basic<ModelInvoice, Database::Date> taxDate;
    static Model::Field::Basic<ModelInvoice, unsigned long long> prefix;
    static Model::Field::ForeignKey<ModelInvoice, unsigned long long, ModelRegistrar> registrarId;
    static Model::Field::Basic<ModelInvoice, Database::Money> credit;
    static Model::Field::Basic<ModelInvoice, Database::Money> price;
    static Model::Field::Basic<ModelInvoice, int> vat;
    static Model::Field::Basic<ModelInvoice, Database::Money> total;
    static Model::Field::Basic<ModelInvoice, Database::Money> totalVat;
    static Model::Field::ForeignKey<ModelInvoice, unsigned long long, ModelInvoicePrefix> prefixTypeId;
    static Model::Field::ForeignKey<ModelInvoice, unsigned long long, ModelFiles> fileId;
    static Model::Field::ForeignKey<ModelInvoice, unsigned long long, ModelFiles> fileXmlId;

    static Model::Field::Related::OneToOne<ModelInvoice, unsigned long long, ModelZone> zone;
    static Model::Field::Related::OneToOne<ModelInvoice, unsigned long long, ModelRegistrar> registrar;
    static Model::Field::Related::OneToOne<ModelInvoice, unsigned long long, ModelInvoicePrefix> prefixType;
    static Model::Field::Related::OneToOne<ModelInvoice, unsigned long long, ModelFiles> file;
    static Model::Field::Related::OneToOne<ModelInvoice, unsigned long long, ModelFiles> fileXml;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelInvoice

#endif // _MODEL_INVOICE_H_

