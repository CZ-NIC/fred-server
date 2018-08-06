#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/util.hh"

namespace Epp {
namespace Contact {

ContactData::ContactData()
    : name(make_data_with_unspecified_privacy(boost::optional<std::string>())),
      organization(make_data_with_unspecified_privacy(boost::optional<std::string>())),
      address(make_data_with_unspecified_privacy(boost::optional<Address>())),
      telephone(make_data_with_unspecified_privacy(boost::optional<std::string>())),
      fax(make_data_with_unspecified_privacy(boost::optional<std::string>())),
      email(make_data_with_unspecified_privacy(boost::optional<std::string>())),
      notify_email(make_data_with_unspecified_privacy(boost::optional<std::string>())),
      vat(make_data_with_unspecified_privacy(boost::optional<std::string>())),
      ident(make_data_with_unspecified_privacy(boost::optional<ContactIdent>()))
{ }

ContactData ContactData::get_trimmed_copy()const
{
    ContactData dst;
    dst.name = trim(this->name);
    dst.organization = trim(this->organization);
    dst.address = trim(this->address);
    dst.mailing_address = trim(this->mailing_address);
    dst.telephone = trim(this->telephone);
    dst.fax = trim(this->fax);
    dst.email = trim(this->email);
    dst.notify_email = trim(this->notify_email);
    dst.vat = trim(this->vat);
    dst.ident = trim(this->ident);
    dst.authinfopw = this->authinfopw;
    return dst;
};

}//namespace Epp::Contact
}//namespace Epp
