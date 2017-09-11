#include "src/epp/contact/contact_data.h"

#include "src/epp/contact/util.h"

namespace Epp {
namespace Contact {

ContactData trim(const ContactData& src)
{
    ContactData dst;
    dst.name = trim(src.name);
    dst.organization = trim(src.organization);
    dst.streets = trim(src.streets);
    dst.city = trim(src.city);
    dst.state_or_province = trim(src.state_or_province);
    dst.postal_code = trim(src.postal_code);
    dst.country_code = trim(src.country_code);
    dst.telephone = trim(src.telephone);
    dst.fax = trim(src.fax);
    dst.email = trim(src.email);
    dst.notify_email = trim(src.notify_email);
    dst.vat = trim(src.vat);
    dst.ident = trim(src.ident);
    dst.authinfopw = src.authinfopw;
    dst.disclose = src.disclose;
    return dst;
};

} // namespace Epp::Contact
} // namespace Epp
