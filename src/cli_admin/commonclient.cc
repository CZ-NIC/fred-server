/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
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

#include "commonclient.h"


const char *corbaOpts[][2] = {
    {"nativeCharCodeSet", "UTF-8"},
    {NULL, NULL},
};
const char g_prog_name[]    = "fred-admin";
const char g_version[]      = "0.2";

std::vector<std::string>
separateSpaces(const char *string)
{
    std::vector<std::string> ret;
    char *tok;
    char *str;

    if (strlen(string) == 0) {
        return ret;
    }

    if ((str = (char *)std::malloc(sizeof(char) * std::strlen(string))) == NULL) {
        std::cerr << "Cannot alocate memory (separateSpace)" << std::endl;
        exit(1);
    }
    if ((str = std::strcpy(str, string)) == NULL) {
        std::cerr << "Cannot alocate memory (separateSpace)" << std::endl;
        exit(1);
    }

    tok = std::strtok(str, " ");
    while (tok != NULL) {
        ret.push_back(std::string(tok));
        tok = std::strtok(NULL, " ");
    }
    std::free(str);
    
    return ret;
}

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
        << "Copyright (c) 2008 CZ.NIC z.s.p.o.\n"
        << "This file is part of FRED (for details see fred.nic.cz)\n"
        << "Licence information:\n"
        << "FRED is free software: you can redistribute it and/or modify\n"
        << "it under the terms of the GNU General Public License as published by\n"
        << "the Free Software Foundation, version 2 of the License.\n\n"
        << "FRED is distributed in the hope that it will be useful,\n"
        << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        << "GNU General Public License for more details.\n\n"
        << "You should have received a copy of the GNU General Public License\n"
        << "along with FRED.  If not, see <http://www.gnu.org/licenses/>."
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

