#ifndef DISCLOSE_SETTINGS_HH_0AE6127E4AE9497878CEE1B36A92EAC5//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define DISCLOSE_SETTINGS_HH_0AE6127E4AE9497878CEE1B36A92EAC5

#include "tools/disclose_flags_updater/disclose_value.hh"

namespace Tools {
namespace DiscloseFlagsUpdater {


struct DiscloseSettings
{
    DiscloseValue name;
    DiscloseValue org;
    DiscloseAddressValue addr;
    DiscloseValue voice;
    DiscloseValue fax;
    DiscloseValue email;
    DiscloseValue vat;
    DiscloseValue ident;
    DiscloseValue notify_email;

    bool is_empty() const
    {
        return name == DiscloseValue::not_set
            && org == DiscloseValue::not_set
            && addr == DiscloseAddressValue::not_set
            && voice == DiscloseValue::not_set
            && fax == DiscloseValue::not_set
            && email == DiscloseValue::not_set
            && vat == DiscloseValue::not_set
            && ident == DiscloseValue::not_set
            && notify_email == DiscloseValue::not_set;
    }
};


std::ostream& operator<<(std::ostream& _os, DiscloseSettings _value);


}
}

#endif//DISCLOSE_SETTINGS_HH_0AE6127E4AE9497878CEE1B36A92EAC5
