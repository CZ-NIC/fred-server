/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  xml generator
 */

#ifndef XMLGEN_HH_A6C4BC97CEC14C489DF5FF8E97BEEAE8
#define XMLGEN_HH_A6C4BC97CEC14C489DF5FF8E97BEEAE8

#include <string>
#include <vector>
#include <algorithm>

#include <boost/function.hpp>

#include "util/util.hh"
namespace Util
{
typedef boost::function<void (std::string&, unsigned)> XmlIndentingCallback;
typedef boost::function<void (std::string&)> XmlCallback;

std::string XmlEscapeCdata(std::string data);
std::string XmlUnparsedCData(const std::string& data);
std::string XmlEscapeTag(std::string data);
std::string XmlEscapeAttr(std::string data);

class XmlIndentingCallbackCaller
{
    std::string& xmlout_;
    unsigned indent_level_;
public:
    XmlIndentingCallbackCaller(std::string& xmlout, unsigned indent_level)
    : xmlout_(xmlout)
    , indent_level_(indent_level)
    {}

    void operator()(XmlIndentingCallback f)
    {
        f(xmlout_, indent_level_);
    }
};

class XmlCallbackCaller
{
    std::string& xmlout_;
public:
    XmlCallbackCaller(std::string& xmlout)
    : xmlout_(xmlout)
    {}

    void operator()(XmlCallback f)
    {
        f(xmlout_);
    }
};


class XmlIndentingTagPair
{
    const std::string tag_;
    const std::vector<XmlIndentingCallback> cb_content_;
    const std::string txt_content_;
public:

    XmlIndentingTagPair(const std::string& tag)
    : tag_(tag)
    {}

    XmlIndentingTagPair(const std::string& tag,const std::string& txt_content)
    : tag_(tag)
    , txt_content_(txt_content)
    {}

    XmlIndentingTagPair(const std::string& tag,const std::vector<XmlIndentingCallback>& cb_content)
    : tag_(tag)
    , cb_content_(cb_content)
    {}

    void operator()(std::string& xmlout, unsigned indent_level = 0)
    {
        static const std::string indentby("  ");

        for(unsigned i = 0; i < indent_level; ++i)
        {
            xmlout += indentby;
        }

        xmlout += "<";
        xmlout += tag_;
        xmlout += ">";

        if(!cb_content_.empty()) xmlout += "\n";

        if(cb_content_.empty())
        {
            xmlout += txt_content_;
        }
        else
        {
            std::for_each(cb_content_.begin(),cb_content_.end(),XmlIndentingCallbackCaller(xmlout, indent_level+1));
        }

        if(!cb_content_.empty())
            for(unsigned i = 0; i < indent_level; ++i)
        {
            xmlout += indentby;
        }

        xmlout += "</";
        xmlout += tag_;
        xmlout += ">\n";
    }
};

class XmlTagPair
{
    const std::string tag_;
    const std::vector<XmlCallback> cb_content_;
    const std::string txt_content_;
    const std::string attributes_;
public:

    XmlTagPair(const std::string& tag)
    : tag_(tag)
    {}

    XmlTagPair(const std::string& tag,const std::string& txt_content)
    : tag_(tag)
    , txt_content_(txt_content)
    {}

    XmlTagPair(const std::string& tag,const std::vector<XmlCallback>& cb_content)
    : tag_(tag)
    , cb_content_(cb_content)
    {}

    XmlTagPair(const std::string& tag, const std::string& txt_content, const std::string& attributes)
    : tag_(tag)
    , txt_content_(txt_content)
    , attributes_(attributes)
    {}

    XmlTagPair(const std::string& tag,const std::vector<XmlCallback>& cb_content, const std::string& attributes)
    : tag_(tag)
    , cb_content_(cb_content)
    , attributes_(attributes)
    {}


    void operator()(std::string& xmlout)
    {
        xmlout += "<";
        xmlout += tag_;
        if(!attributes_.empty())
        {
            xmlout += " ";
            xmlout += attributes_;
        }
        xmlout += ">";

        if(cb_content_.empty())
        {
            xmlout += txt_content_;
        }
        else
        {
            std::for_each(cb_content_.begin(),cb_content_.end(),XmlCallbackCaller(xmlout));
        }

        xmlout += "</";
        xmlout += tag_;
        xmlout += ">";
    }
};

} // namespace Util

#endif /* XMLGEN_H_ */
