#include "invoice_common.h"
#include "invoice_manager.h"

namespace Register {
namespace Invoicing {

//-----------------------------------------------------------------------------
//
// Payment
//
//-----------------------------------------------------------------------------
Payment::Payment()
{
}
Payment::Payment(const Payment *sec):
    m_price(sec->m_price), m_vatRate(sec->m_vatRate), m_vat(sec->m_vat)
{
}
void
Payment::setPrice(const Database::Money &price)
{
    m_price = price;
}
void
Payment::setVatRate(const unsigned int &vatRate)
{
    m_vatRate = vatRate;
}
void
Payment::setVat(const Database::Money &vat)
{
    m_vat = vat;
}
Database::Money 
Payment::getPrice() const
{
    return m_price;
}
unsigned int 
Payment::getVatRate() const
{
    return m_vatRate;
}
Database::Money 
Payment::getVat() const
{
    return m_vat;
}
Database::Money 
Payment::getPriceWithVat() const
{
    return m_price + m_vat;
}
bool 
Payment::operator==(unsigned vatRate) const
{
    return m_vatRate == vatRate;
}
void 
Payment::add(const Payment *sec)
{
    m_price = m_price + sec->m_price;
    m_vat = m_vat + sec->m_vat;
}

//-----------------------------------------------------------------------------
//
// PaymentSource
//
//-----------------------------------------------------------------------------
void
PaymentSource::setId(const unsigned long long &id)
{
    m_id = id;
}
void
PaymentSource::setNumber(const unsigned long long &number)
{
    m_number = number;
}
void
PaymentSource::setCredit(const Database::Money &credit)
{
    m_credit = credit;
}
void
PaymentSource::setTotalPrice(const Database::Money &totalPrice)
{
    m_totalPrice = totalPrice;
}
void
PaymentSource::setTotalVat(const Database::Money &totalVat)
{
    m_totalVat = totalVat;
}
void
PaymentSource::setCrTime(const Database::DateTime &crTime)
{
    m_crTime = crTime;
}
unsigned long long
PaymentSource::getId() const
{
    return m_id;
}

unsigned long long 
PaymentSource::getNumber() const
{
    return m_number;
}

Database::Money 
PaymentSource::getCredit() const
{
    return m_credit;
}

Database::Money 
PaymentSource::getTotalPrice() const
{
    return m_totalPrice;
}

Database::Money 
PaymentSource::getTotalVat() const
{
    return m_totalVat;
}

Database::Money 
PaymentSource::getTotalPriceWithVat() const
{
    return m_totalPrice + m_totalVat;
}

Database::DateTime 
PaymentSource::getCrTime() const
{
    return m_crTime;
}

//-----------------------------------------------------------------------------
//
// PaymentAction
//
//-----------------------------------------------------------------------------
void
PaymentAction::setObjectId(const unsigned long long &objectId)
{
    m_objectId = objectId;
}
void
PaymentAction::setObjectName(const std::string &objectName)
{
    m_objectName = objectName;
}
void
PaymentAction::setActionTime(const Database::DateTime &actionTime)
{
    m_actionTime = actionTime;
}
void
PaymentAction::setExDate(const Database::Date &exDate)
{
    m_exDate = exDate;
}
void
PaymentAction::setAction(PaymentActionType action)
{
    m_action = action;
}
void
PaymentAction::setUnitsCount(const unsigned int &unitsCount)
{
    m_unitsCount = unitsCount;
}
void
PaymentAction::setPricePerUnit(const Database::Money &pricePerUnit)
{
    m_pricePerUnit = pricePerUnit;
}
unsigned long long
PaymentAction::getObjectId() const 
{
    return m_objectId;
}
const std::string &
PaymentAction::getObjectName() const 
{
    return m_objectName;
}
Database::DateTime
PaymentAction::getActionTime() const 
{
    return m_actionTime;
}
Database::Date
PaymentAction::getExDate() const
{
    return m_exDate;
}
PaymentActionType
PaymentAction::getAction() const 
{
    return m_action;
}
std::string
PaymentAction::getActionStr() const
{
    switch (getAction()) {
        case PAT_CREATE_DOMAIN:
            return "CREATE";
            break;
        case PAT_RENEW_DOMAIN:
            return "RENEW";
            break;
        default:
            return "UNKNOWN";
            break;
    }
}
unsigned int
PaymentAction::getUnitsCount() const 
{
    return m_unitsCount;
}
Database::Money
PaymentAction::getPricePerUnit() const 
{
    return m_pricePerUnit;
}

//-----------------------------------------------------------------------------
//
// AnnualPartitioning
//
//-----------------------------------------------------------------------------
AnnualPartitioning::AnnualPartitioning(Manager *manager):
    m_manager(manager), m_noVatRate(true)
{ }

void 
AnnualPartitioning::addAction(PaymentAction *action)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    if (!action->getUnitsCount() || action->getExDate().is_special()) {
        return;
    }
    date lastdate = action->getExDate().get();
    int dir = (action->getPrice() > Database::Money(0)) ? 1 : -1;
    date firstdate = action->getExDate().get() - months(dir * action->getUnitsCount());
    if (dir) {
        date pom = lastdate;
        lastdate = firstdate;
        firstdate = lastdate;
        // std::swap(lastdate, firstdate);
    }
    Database::Money remains = action->getPrice();
    while (remains) {
        Database::Money part;
        unsigned int year = lastdate.year();
        if (year == firstdate.year()) {
            part = remains;
        } else {
            date newdate = date(year, 1, 1) - days(1);
            part = remains * (lastdate - newdate).days() /
                (lastdate - firstdate).days();
            lastdate = newdate;
        }
        remains = remains - part;
        m_records[action->getVatRate()][year] =
            m_records[action->getVatRate()][year] + part;
    }
} // AnnualPartitioningImpl::addAction();

void 
AnnualPartitioning::resetIterator(const unsigned int &vatRate)
{
    m_j = m_records.find(vatRate);
    if (m_j == m_records.end()) {
        m_noVatRate = true;
    } else {
        m_noVatRate = false;
        m_i = m_j->second.begin();
    }
}

bool 
AnnualPartitioning::end() const
{
    return m_noVatRate || m_i == m_j->second.end();
}

void 
AnnualPartitioning::next()
{
    m_i++;
}
unsigned int 
AnnualPartitioning::getYear() const
{
    return end() ? 0 : m_i->first;
}
Database::Money 
AnnualPartitioning::getPrice() const
{
    return end() ? Database::Money(0) : m_i->second;
}

unsigned int 
AnnualPartitioning::getVatRate() const
{
    return end() ? 0 : m_j->first;
}

Database::Money 
AnnualPartitioning::getVat() const
{
    return m_manager->countVat(getPrice(), getVatRate(), true);
}

Database::Money 
AnnualPartitioning::getPriceWithVat() const
{
    return getPrice() + getVat();
}

} // namespace Invoicing
} // namespace Register

