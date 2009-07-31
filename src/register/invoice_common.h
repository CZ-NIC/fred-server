#ifndef _INVOICE_COMMON_H_
#define _INVOICE_COMMON_H_

#include "db_settings.h"
#include <map>

namespace Register {
namespace Invoicing {

class Subject {
private:
    Database::ID m_id;
    std::string m_handle;
    std::string m_name;
    std::string m_fullname;
    std::string m_street;
    std::string m_city;
    std::string m_zip;
    std::string m_country;
    std::string m_ico;
    std::string m_vatNumber;
    std::string m_registration;
    std::string m_reclamation;
    std::string m_email;
    std::string m_url;
    std::string m_phone;
    std::string m_fax;
    bool m_vatApply;
public:
    Subject()
    { }
    Subject(Database::ID _id, const std::string& _handle,
            const std::string& _name, const std::string& _fullname, 
            const std::string& _street, const std::string& _city, 
            const std::string& _zip, const std::string& _country, 
            const std::string& _ico, const std::string& _vatNumber, 
            const std::string& _registration, const std::string& _reclamation, 
            const std::string& _email, const std::string& _url, 
            const std::string& _phone, const std::string& _fax, bool _vatApply):
        m_id(_id), m_handle(_handle), m_name(_name), m_fullname(_fullname),
        m_street(_street), m_city(_city), m_zip(_zip), m_country(_country),
        m_ico(_ico), m_vatNumber(_vatNumber), m_registration(_registration),
        m_reclamation(_reclamation), m_email(_email), m_url(_url), m_phone(_phone),
        m_fax(_fax), m_vatApply(_vatApply)
    { }
    Subject operator=(const Subject &sec)
    {
        m_id = sec.getId();
        m_handle = sec.getHandle();
        m_name = sec.getName();
        m_fullname = sec.getFullname();
        m_street = sec.getStreet();
        m_city = sec.getCity();
        m_zip = sec.getZip();
        m_country = sec.getCountry();
        m_ico = sec.getICO();
        m_vatNumber = sec.getVatNumber();
        m_registration = sec.getRegistration();
        m_reclamation = sec.getReclamation();
        m_email = sec.getEmail();
        m_url = sec.getURL();
        m_phone = sec.getPhone();
        m_fax = sec.getFax();
        m_vatApply = sec.getVatApply();
        return sec;
    }
    Database::ID getId() const 
    {
        return m_id;
    }
    const std::string& getHandle() const 
    {
        return m_handle;
    }
    const std::string& getName() const 
    {
        return m_name;
    }
    const std::string& getFullname() const 
    {
        return m_fullname;
    }
    const std::string& getStreet() const 
    {
        return m_street;
    }
    const std::string& getCity() const 
    {
        return m_city;
    }
    const std::string& getZip() const 
    {
        return m_zip;
    }
    const std::string& getCountry() const 
    {
        return m_country;
    }
    const std::string& getICO() const 
    {
        return m_ico;
    }
    const std::string& getVatNumber() const 
    {
        return m_vatNumber;
    }
    bool getVatApply() const 
    {
        return m_vatApply;
    }
    const std::string& getRegistration() const 
    {
        return m_registration;
    }
    const std::string& getReclamation() const 
    {
        return m_reclamation;
    }
    const std::string& getEmail() const 
    {
        return m_email;
    }
    const std::string& getURL() const 
    {
        return m_url;
    }
    const std::string& getPhone() const 
    {
        return m_phone;
    }
    const std::string& getFax() const 
    {
        return m_fax;
    }
}; // class Subject;
class Payment {
private:
    Database::Money     m_price;
    unsigned int        m_vatRate;
    Database::Money     m_vat;
public:
    Payment();
    Payment(const Payment *sec);
    void setPrice(const Database::Money &price);
    void setVatRate(const unsigned int &vatRate);
    void setVat(const Database::Money &vat);

    Database::Money getPrice() const;
    unsigned int getVatRate() const;
    Database::Money getVat() const;
    Database::Money getPriceWithVat() const;

    bool operator==(unsigned vatRate) const;
    void add(const Payment *sec);
};

class PaymentSource:
    public Payment {
private:
    Database::ID        m_id;
    unsigned long long  m_number;
    Database::Money     m_credit;
    Database::Money     m_totalPrice;
    Database::Money     m_totalVat;
    Database::DateTime  m_crTime;
public:
    void setId(const unsigned long long &id);
    void setNumber(const unsigned long long &number);
    void setCredit(const Database::Money &credit);
    void setTotalPrice(const Database::Money &totalPrice);
    void setTotalVat(const Database::Money &totalVat);
    void setCrTime(const Database::DateTime &crTime);
    unsigned long long getId() const;
    unsigned long long getNumber() const;
    Database::Money getCredit() const;
    Database::Money getTotalPrice() const;
    Database::Money getTotalVat() const;
    Database::Money getTotalPriceWithVat() const;
    Database::DateTime getCrTime() const;
};

enum PaymentActionType {
    PAT_CREATE_DOMAIN = 1,
    PAT_RENEW_DOMAIN
};

class PaymentAction:
    public Payment {
private:
    unsigned long long  m_objectId;
    std::string         m_objectName;
    Database::DateTime  m_actionTime;
    Database::Date      m_exDate;
    PaymentActionType   m_action;
    unsigned int        m_unitsCount;
    Database::Money     m_pricePerUnit;
public:
    void setObjectId(const unsigned long long &objectId);
    void setObjectName(const std::string &objectName);
    void setActionTime(const Database::DateTime &actionTime);
    void setExDate(const Database::Date &exDate);
    void setAction(PaymentActionType action);
    void setUnitsCount(const unsigned int &unitsCount);
    void setPricePerUnit(const Database::Money &pricePerUnit);
    unsigned long long getObjectId() const;
    const std::string &getObjectName() const;
    Database::DateTime getActionTime() const;
    Database::Date getExDate() const;
    PaymentActionType getAction() const;
    std::string getActionStr() const;
    unsigned int getUnitsCount() const;
    Database::Money getPricePerUnit() const;
};

class Manager;

class AnnualPartitioning:
    public Payment {
private:
    typedef std::map<unsigned int, Database::Money> RecordsType;
    typedef std::map<unsigned int, RecordsType> vatRatesRecordsType;

    RecordsType::const_iterator         m_i;
    vatRatesRecordsType::const_iterator m_j;
    vatRatesRecordsType                 m_records;
    Manager                             *m_manager;
    bool                                m_noVatRate;
public:
    AnnualPartitioning(Manager *manager = NULL);
    void addAction(PaymentAction *action);
    void resetIterator(const unsigned &vatRate);
    bool end() const;
    void next();
    unsigned int getYear() const;
    Database::Money getPrice() const;
    unsigned int getVatRate() const;
    Database::Money getVat() const;
    Database::Money getPriceWithVat() const;
};

}
}

#endif // _INVOICE_COMMON_H_
