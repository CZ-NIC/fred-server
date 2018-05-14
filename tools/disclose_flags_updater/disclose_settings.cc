#include "tools/disclose_flags_updater/disclose_settings.hh"

namespace Tools {
namespace DiscloseFlagsUpdater {


std::ostream& operator<<(std::ostream& _os, DiscloseSettings _value)
{
    return _os << "{"
        << " name='" << _value.name << "'"
        << " org='" << _value.org << "'"
        << " addr='" << _value.addr << "'"
        << " voice='" << _value.voice << "'"
        << " fax='" << _value.fax << "'"
        << " email='" << _value.email << "'"
        << " vat='" << _value.vat << "'"
        << " ident='" << _value.ident << "'"
        << " notify_email='" << _value.notify_email << "'"
        << " }";
}


}
}
