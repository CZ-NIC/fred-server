/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <cstring>
#include <utility>

#include "src/bin/cli/commonclient.hh"

#include "src/util/corba_wrapper_decl.hh"
#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"



const char *corbaOpts[][2] = {
    {"nativeCharCodeSet", "UTF-8"},
    {NULL, NULL},
};
const char g_prog_name[]    = "fred-admin";
const char g_version[]      = "0.2";

std::vector<std::string>
separate(const std::string str, int ch)
{
    size_t pos = 0;
    size_t initial = 0;
    std::vector<std::string> ret;

    if (str.empty()) {
        return ret;
    }
    
    pos = str.find(ch, initial);
    if (pos == std::string::npos) {
        ret.push_back(str);
        return ret;
    }
    while (pos != std::string::npos) {
        ret.push_back(str.substr(initial, pos - initial));
        initial = pos + 1;
        pos = str.find(ch, initial);
    }
    ret.push_back(str.substr(initial));
    return ret;
}

void
print_version()
{
    std::cout
        << g_prog_name << " version " << g_version << "\nFor list of all options type: ``$ "
        << g_prog_name << " --help''\n\n"
        << "Copyright (C) 2008-2018  CZ.NIC, z. s. p. o.\n\n"
        << "This file is part of FRED (for details see fred.nic.cz).\n\n"
        << "FRED is free software: you can redistribute it and/or modify\n"
        << "it under the terms of the GNU General Public License as published by\n"
        << "the Free Software Foundation, either version 3 of the License, or\n"
        << "(at your option) any later version.\n\n"
        << "FRED is distributed in the hope that it will be useful,\n"
        << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        << "GNU General Public License for more details.\n\n"
        << "You should have received a copy of the GNU General Public License\n"
        << "along with FRED.  If not, see <https://www.gnu.org/licenses/>.\n"
        << std::endl;
}
void
print_moo()
{
    std::cout
        << "         (__) \n"
        << "         (oo) \n"
        << "   /------\\/ \n"
        << "  / |    ||   \n"
        << " *  /\\---/\\ \n"
        << "    ~~   ~~   \n"
        << "....\"Have you mooed today?\"..."
        << std::endl;
}

void
help_dates()
{
    std::cout
        << "Possible dates format: (shown for create date option)\n"
        << " ``--crdate=\"2008-10-16\"''        - one specific day (time is from 00:00:00 to 23:59:59)\n"
        << " ``--crdate=\"2008-10-16;\"''       - interval from '2008-10-16 00:00:00' to the biggest valid date\n"
        << " ``--crdate=\";2008-10-16\"''       - interval from the lowest valid date to '2008-10-16 23:59:59'\n"
        << " ``--crdate=\"2008-10-16;2008-10-20\"'' - interval from '2008-10-16 00:00:00' to '2008-10-20 23:59:59'\n"
        << " ``--crdate=\"last_week;-1\"''      - this mean whole week before this one\n"
        << "\nPossible relative options: last_day, last_week, last_month, last_year, past_hour, past_week, past_month, past_year\n"
        << "Is also possible to input incomplete date e.g: ``--crdate=\"2008-10\"'' means the whole october 2008\n"
        << "\t (same as ``--crdate=\"2008-10-01 00:00:00;2008-10-31 23:59:59\"'')\n"
        << std::endl;
}

char dnyMesice[12][3] = { "31", "28", "31", "30", "31", "30", "31", "31", "30", "31", "30", "31" };

std::string
createDateTime(std::string datetime, int interval, int type)
{
    std::string ret;
    ret = datetime;
    switch (datetime.length()) {
        case 3:
            if (datetime.compare("now") == 0) {
                if (type == PARSE_DATETIME) {
                    ret = Database::DateTime(Database::NOW).to_string();
                } else {
                    ret = Database::Date(Database::NOW).to_string();
                }
            }
            break;
        case 4:
            // "2008"
            if (interval == DATETIME_TO) {
                if (type == PARSE_DATETIME) {
                    ret += "-12-31 23:59:59";
                } else {
                    ret += "-12-31";
                }
            } else if (interval == DATETIME_FROM) {
                if (type == PARSE_DATETIME) {
                    ret += "-01-01 00:00:00";
                } else {
                    ret += "-01-01";
                }
            }
            break;
        case 7:
            // "2008-05"
            {
                int mesic   = atoi((datetime.substr(5, 2)).c_str()) - 1;
                int rok     = atoi((datetime.substr(0, 4)).c_str());
                if (interval == DATETIME_TO) {
                    ret += "-";
                    if (rok % 100 == 0 && rok % 400 != 0) {
                        ret += dnyMesice[mesic];
                    } else if (rok % 4 == 0) {
                        ret += "29";
                    } else {
                        ret += dnyMesice[mesic];
                    }
                    if (type == PARSE_DATETIME) {
                        ret += " 23:59:59";
                    }
                } else if (interval == DATETIME_FROM) {
                    if (type == PARSE_DATETIME) {
                        ret += "-01 00:00:00";
                    } else {
                        ret += "-01";
                    }
                }
            }
            break;
        case 10:
            // "2008-05-15"
            if (type == PARSE_DATEONLY) {
                break;
            }
            if (interval == DATETIME_TO) {
                ret += " 23:59:59";
            } else if (interval == DATETIME_FROM) {
                ret += " 00:00:00";
            }
            break;
        case 13:
            // "2008-05-15 19"
            if (type == PARSE_DATEONLY) {
                ret = datetime.substr(0, 10);
                break;
            }
            if (interval == DATETIME_TO) {
                ret += ":59:59";
            } else if (interval == DATETIME_FROM) {
                ret += ":00:00";
            }
            break;
        case 16:
            // "2008-05-15 19:22"
            if (type == PARSE_DATEONLY) {
                ret = datetime.substr(0, 10);
                break;
            }
            if (interval == DATETIME_TO) {
                ret += ":59";
            } else if (interval == DATETIME_FROM) {
                ret += ":00";
            }
            break;
        case 19:
            // "2008-05-15 19:22:10"
            break;
        default:
            // some crap
            break;
    }
    return ret;
}

Database::DateInterval *
parseDate(std::string str)
{
    Database::DateInterval  *ret;
    Database::Date          dateFrom(Database::NEG_INF);
    Database::Date          dateTo(Database::POS_INF);

    if (str[0] == ';') {
        dateTo.from_string(
                createDateTime(
                    str.substr(1, std::string::npos),
                    DATETIME_TO, PARSE_DATEONLY)
                );
    } else if (str[str.length() - 1] == ';') {
        dateFrom.from_string(
                createDateTime(
                    str.substr(0, str.length() - 1),
                    DATETIME_FROM, PARSE_DATEONLY)
                );
    } else if (islower(str[0])) {
        std::string specStr = str.substr(0, str.find(';'));
        int offset = 0;
        if (str.find(';') != std::string::npos) {
            offset = atoi((str.substr(str.find(';') + 1, std::string::npos)).c_str());
        }
        Database::DateTimeIntervalSpecial spec = Database::NONE;
        if (specStr.compare("last_hour") == 0) {
            spec = Database::LAST_HOUR;
        } else if (specStr.compare("last_day") == 0) {
            spec = Database::LAST_DAY;
        } else if (specStr.compare("last_week") == 0) {
            spec = Database::LAST_WEEK;
        } else if (specStr.compare("last_month") == 0) {
            spec = Database::LAST_MONTH;
        } else if (specStr.compare("last_year") == 0) {
            spec = Database::LAST_YEAR;
        } else if (specStr.compare("past_hour") == 0) {
            spec = Database::PAST_HOUR;
        } else if (specStr.compare("past_day") == 0) {
            spec = Database::PAST_DAY;
        } else if (specStr.compare("past_week") == 0) {
            spec = Database::PAST_WEEK;
        } else if (specStr.compare("past_month") == 0) {
            spec = Database::PAST_MONTH;
        } else if (specStr.compare("past_year") == 0) {
            spec = Database::PAST_YEAR;
        }
        ret = new Database::DateInterval(spec, offset);
        return ret;
    } else if (str.find(';') == std::string::npos) {
        dateFrom.from_string(
                createDateTime(str, DATETIME_FROM, PARSE_DATEONLY));
        dateTo.from_string(
                createDateTime(str, DATETIME_TO, PARSE_DATEONLY));
    } else if (isdigit(str[0])) {
        dateFrom.from_string(
                createDateTime(
                    str.substr(0, str.find(';')),
                    DATETIME_FROM, PARSE_DATEONLY)
                );
        dateTo.from_string(
                createDateTime(
                    str.substr(str.find(';') + 1, std::string::npos),
                    DATETIME_TO, PARSE_DATEONLY)
                );
    }
    ret = new Database::DateInterval(dateFrom, dateTo);
    return ret;
}

Database::DateTimeInterval *
parseDateTime(std::string str)
{
    Database::DateTimeInterval  *ret;
    Database::DateTime          dateFrom(Database::NEG_INF);
    Database::DateTime          dateTo(Database::POS_INF);

    if (str[0] == ';') {
        dateTo.from_string(
                createDateTime(
                    str.substr(1, std::string::npos),
                    DATETIME_TO)
                );
    } else if (str[str.length() - 1] == ';') {
        dateFrom.from_string(
                createDateTime(
                    str.substr(0, str.length() - 1),
                    DATETIME_FROM)
                );
    } else if (islower(str[0])) {
        std::string specStr = str.substr(0, str.find(';'));
        int offset = 0;
        if (str.find(';') != std::string::npos) {
            offset = atoi((str.substr(str.find(';') + 1, std::string::npos)).c_str());
        }
        Database::DateTimeIntervalSpecial spec = Database::NONE;
        if (specStr.compare("last_hour") == 0) {
            spec = Database::LAST_HOUR;
        } else if (specStr.compare("last_day") == 0) {
            spec = Database::LAST_DAY;
        } else if (specStr.compare("last_week") == 0) {
            spec = Database::LAST_WEEK;
        } else if (specStr.compare("last_month") == 0) {
            spec = Database::LAST_MONTH;
        } else if (specStr.compare("last_year") == 0) {
            spec = Database::LAST_YEAR;
        } else if (specStr.compare("past_hour") == 0) {
            spec = Database::PAST_HOUR;
        } else if (specStr.compare("past_day") == 0) {
            spec = Database::PAST_DAY;
        } else if (specStr.compare("past_week") == 0) {
            spec = Database::PAST_WEEK;
        } else if (specStr.compare("past_month") == 0) {
            spec = Database::PAST_MONTH;
        } else if (specStr.compare("past_year") == 0) {
            spec = Database::PAST_YEAR;
        }
        ret = new Database::DateTimeInterval(spec, offset);
        return ret;
    } else if (str.find(';') == std::string::npos) {
        dateFrom.from_string(
                createDateTime(str, DATETIME_FROM));
        dateTo.from_string(
                createDateTime(str, DATETIME_TO));
    } else if (isdigit(str[0])) {
        dateFrom.from_string(
                createDateTime(
                    str.substr(0, str.find(';')),
                    DATETIME_FROM)
                );
        dateTo.from_string(
                createDateTime(
                    str.substr(str.find(';') + 1, std::string::npos),
                    DATETIME_TO)
                );
    }
    ret = new Database::DateTimeInterval(dateFrom, dateTo);
    return ret;
}
