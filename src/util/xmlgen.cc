/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 *  xml generator
 */

#include "src/util/xmlgen.hh"

#include <boost/algorithm/string/replace.hpp>

namespace Util
{
    std::string XmlEscapeCdata(std::string data)
    {
        boost::replace_all(data, "]]>", "]]]]><![CDATA[>");
        return data;
    }


    std::string XmlUnparsedCData(const std::string& data)
    {
        std::string ret("<![CDATA[");
        ret+=data;
        ret+="]]>";
        return ret;
    }

    std::string XmlEscapeTag(std::string data)
    {
        boost::replace_all(data, "&", "&amp;");
        boost::replace_all(data, "<", "&lt;");
        boost::replace_all(data, ">", "&gt;");
        return data;
    }

    std::string XmlEscapeAttr(std::string data)
    {
        boost::replace_all(data, "\"", "&quot;");
        boost::replace_all(data, "'", "&apos;");
        return data;
    }


} // namespace Util
