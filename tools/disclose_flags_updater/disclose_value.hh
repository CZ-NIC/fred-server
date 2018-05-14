#ifndef DISCLOSE_VALUE_HH_4369340DCE268AB57517A8097A1F2D04//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define DISCLOSE_VALUE_HH_4369340DCE268AB57517A8097A1F2D04

#include <iostream>
#include <string>

namespace Tools {
namespace DiscloseFlagsUpdater {


enum class DiscloseValue
{
    not_set,
    hide,
    show
};


std::ostream& operator<<(std::ostream& _os, DiscloseValue _value);

std::istream& operator>>(std::istream& _in, DiscloseValue& _value);

std::string to_string(DiscloseValue _value);

bool to_db_value(DiscloseValue _value);


enum class DiscloseAddressValue
{
    not_set,
    hide,
    show,
    hide_verified
};


std::ostream& operator<<(std::ostream& _os, DiscloseAddressValue _value);

std::istream& operator>>(std::istream& _in, DiscloseAddressValue& _value);

std::string to_string(DiscloseAddressValue _value);

bool to_db_value(DiscloseAddressValue _value);


}
}

#endif//DISCLOSE_VALUE_HH_4369340DCE268AB57517A8097A1F2D04
