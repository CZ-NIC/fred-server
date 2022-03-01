/*
 * Copyright (C) 2014-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/admin/contact/verification/test_impl/test_cz_address_exists.hh"

#include "libfred/opexception.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <libxml/parser.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/regex.hpp>

#include <algorithm> // std::reverse, std::next_permutaion
#include <cctype>
#include <iterator>
#include <locale>
#include <memory>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

namespace {

void erase_chars(
        std::string& _input,
        const std::vector<char>& _chars_to_erase);

TestCzAddress::T_words_shortened parse_multiword(
        const std::string& _registry_data,
        const std::string& _delimiters,
        const std::string& _shortening_delimiters);

TestCzAddress::T_street_data parse_street1(
        const std::string& _street1,
        const std::string& _delimiters,
        const std::string& _shortening_delimiters);

std::string parse_postal_code(const std::string& _postalcode);

TestCzAddress::T_words_shortened rtrim_numbers(
        const TestCzAddress::T_words_shortened& _input,
        unsigned _nondigit_chars_tolerance_per_word = 0);

std::string xpath_normalize_chars(const std::string& _input);

std::string xpath_multiword_match(
        TestCzAddress::T_words_shortened _shortened_words,
        const std::string& _delimiters,
        const std::string& _xpath_lhs);

std::string xpath_city_match(
        TestCzAddress::T_words_shortened _shortened_words,
        const std::string& _delimiters,
        const std::string& _xpath_lhs);

std::string xpath_district_match(
        TestCzAddress::T_words_shortened _shortened_words,
        const std::string& _delimiters,
        const std::string& _xpath_lhs);

std::string xpath_postal_code_match(
        const std::string& _postal_code,
        const std::string& _xpath_min,
        const std::string& _xpath_max);

std::string xpath_house_number_match(
        std::vector<std::string> _numbers,
        std::vector<std::string> _xpath_lhs);

struct XmlXPathObjectDeleter
{
    void operator()(xmlXPathObjectPtr ptr) const
    {
        xmlXPathFreeObject(ptr);
    }
};

}//namespace Fred::Backend::Admin::Contact::Verification::{anonymous}

const std::string TestCzAddress::street_delimiters_("\\/-,()| \t\n\r");
const std::string TestCzAddress::street_shortened_word_signs_(".");
const std::string TestCzAddress::city_delimiters_("-()\\, \t\n\r");
const std::string TestCzAddress::city_shortened_word_signs_("./");

struct ExceptionMissingHouseNumber : virtual LibFred::OperationException
{
    const char* what() const noexcept
    {
        return "missing house number";
    }
};

struct ExceptionInvalidPostalCode : virtual LibFred::OperationException
{
    const char* what() const noexcept
    {
        return "invalid postalcode";
    }
};

TestCzAddress& TestCzAddress::set_mvcr_address_xml_filename(const std::string& _mvcr_address_xml_filename)
{
    /* Load XML document */
    doc_ = xmlParseFile(_mvcr_address_xml_filename.c_str());
    if (doc_ == nullptr)
    {
        throw LibFred::InternalError("Error: unable to parse file " + _mvcr_address_xml_filename);
    }

    /* Create xpath evaluation context */
    xpathCtx_ = xmlXPathNewContext(doc_);
    if (xpathCtx_ == nullptr)
    {
        xmlFreeDoc(doc_);
        throw LibFred::InternalError("Error: unable to create new XPath context");
    }

    return *this;
}

TestCzAddress::~TestCzAddress()
{
    xmlXPathFreeContext(xpathCtx_);
    xmlFreeDoc(doc_);
}

Test::TestRunResult TestCzAddress::run(unsigned long long _history_id) const
{
    TestDataProvider<TestCzAddress> data;
    data.init_data(_history_id);

    // can use boost to_lower because in there should be no utf8 chars in country_code
    std::string country = boost::algorithm::to_lower_copy(data.country_);

    if (country != "cz")
    {
        return TestRunResult{
                LibFred::ContactTestStatus::SKIPPED,
                "this test is intended for CZ addresses only"};
    }

    // registry data
    T_street_data street;
    T_words_shortened city;
    std::string postal_code;

    try
    {
        // TODO erase č.p. variants
        try
        {
            street = parse_street1(
                    boost::algorithm::trim_copy(data.street1_),
                    street_delimiters_,
                    street_shortened_word_signs_);
        }
        catch (const ExceptionMissingHouseNumber&)
        {
            try
            {
                street = parse_street1(
                        boost::algorithm::trim_copy(data.street1_ + " " + data.street2_),
                        street_delimiters_,
                        street_shortened_word_signs_);
            }
            catch (const ExceptionMissingHouseNumber&)
            {
                street = parse_street1(
                        boost::algorithm::trim_copy(
                                data.street1_ + " " + data.street2_ + " " + data.street3_),
                        street_delimiters_,
                        street_shortened_word_signs_);
            }
        }
        street.first = rtrim_numbers(
                street.first,
                2);

        std::string normalized_city(data.city_);
        boost::algorithm::trim(normalized_city);
        // TODO spise vyhodit jen 2ciferna cisla
        erase_chars(
                normalized_city,
                Util::vector_of<char>('0')('1')('2')('3')('4')('5')('6')('7')('8')('9'));

        city = parse_multiword(
                normalized_city,
                city_delimiters_,
                city_shortened_word_signs_);

        if (city.size() < 1)
        {
            return TestRunResult{
                    LibFred::ContactTestStatus::FAIL,
                    "city is missing content"};
        }
        postal_code = parse_postal_code(data.postalcode_);
    }
    catch (const ExceptionMissingHouseNumber& e)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::FAIL,
                e.what()};

    }
    catch (const ExceptionInvalidPostalCode& e)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::FAIL,
                e.what()};

    }
    catch (...)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::FAIL,
                "exception during parsing"};
    }

    try
    {
        if (is_address_valid(
                    street,
                    city,
                    postal_code))
        {
            return TestRunResult{
                    LibFred::ContactTestStatus::OK,
                    ""};
        }
    }
    catch (...)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::FAIL,
                "exception during validation"};
    }

    std::string error_msg;
    try
    {
        error_msg = diagnose_problem(
                street,
                city,
                postal_code);
    }
    catch (...)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::FAIL,
                "exception during diagnostics"};
    }

    return TestRunResult{
            LibFred::ContactTestStatus::FAIL,
            error_msg};
}

bool TestCzAddress::is_address_valid(
        const TestCzAddress::T_street_data& street,
        const TestCzAddress::T_words_shortened& city,
        const std::string& postal_code) const
{
    std::vector<std::string> xpath_queries = generate_xpath_queries(
            street,
            city,
            postal_code);

    xmlErrorPtr error = nullptr;

    for (const std::string& query : xpath_queries)
    {
        const auto xpathObj = std::unique_ptr<xmlXPathObject, XmlXPathObjectDeleter>{
                xmlXPathEvalExpression(
                        reinterpret_cast<const unsigned char*>(query.c_str()),
                        xpathCtx_)};

        error = xmlGetLastError();
        if (error != nullptr)
        {
            const std::string error_msg_str{error->message};
            xmlResetLastError();
            throw LibFred::InternalError("Error: xpath error " + error_msg_str);
        }

        if (xpathObj == nullptr)
        {
            throw LibFred::InternalError("Error: unable to evaluate xpath expression " + query);
        }

        if (xmlXPathCastToBoolean(xpathObj.get()))
        {
            return true;
        }
    }
    return false;
}

std::string TestCzAddress::diagnose_problem(
        const TestCzAddress::T_street_data& _street,
        const TestCzAddress::T_words_shortened& _city,
        const std::string& _postal_code) const
{
    std::vector<std::string> xpath_queries;
    std::map<std::string, std::string> suspicious_data_map;

    std::vector<std::string> testing_xpath_queries;

    testing_xpath_queries = generate_xpath_queries(
            _street,
            _city,
            _postal_code,
            house_number);
    for (const std::string& query : testing_xpath_queries)
    {
        xpath_queries.push_back(query);
        suspicious_data_map[xpath_queries.back()] = "house number";
    }

    testing_xpath_queries = generate_xpath_queries(
            _street,
            _city,
            _postal_code,
            street);
    for (const std::string& query : testing_xpath_queries)
    {
        xpath_queries.push_back(query);
        suspicious_data_map[xpath_queries.back()] = "street";
    }

    testing_xpath_queries = generate_xpath_queries(
            _street,
            _city,
            _postal_code,
            city);
    for (const std::string& query : testing_xpath_queries)
    {
        xpath_queries.push_back(query);
        suspicious_data_map[xpath_queries.back()] = "city";
    }

    testing_xpath_queries = generate_xpath_queries(
            _street,
            _city,
            _postal_code,
            postal_code);
    for (const std::string& query : testing_xpath_queries)
    {
        xpath_queries.push_back(query);
        suspicious_data_map[xpath_queries.back()] = "postal code";
    }

    std::set<std::string> suspicious_data;

    xmlErrorPtr error = nullptr;
    for (const std::string& query : xpath_queries)
    {
        const auto xpathObj = std::unique_ptr<xmlXPathObject, XmlXPathObjectDeleter>{
                xmlXPathEvalExpression(
                        reinterpret_cast<const unsigned char*>(query.c_str()),
                        xpathCtx_)};

        error = xmlGetLastError();
        if (error != nullptr)
        {
            const std::string error_msg_str{error->message};
            xmlResetLastError();
            throw LibFred::InternalError("Error: xpath error " + error_msg_str);
        }

        if (xpathObj == nullptr)
        {
            throw LibFred::InternalError("Error: unable to evaluate xpath expression " + query);
        }

        if (xmlXPathCastToBoolean(xpathObj.get()))
        {
            suspicious_data.insert(suspicious_data_map[query]);
        }
    }

    if (!suspicious_data.empty())
    {
        return boost::join(suspicious_data, " or ") + " might be invalid";
    }

    return std::string();
}

std::vector<std::string> TestCzAddress::generate_xpath_queries(
        TestCzAddress::T_street_data _street,
        TestCzAddress::T_words_shortened _city,
        std::string _postal_code,
        Optional<address_part> _to_omit) const
{
    std::string postal_code_condition = xpath_postal_code_match(
            _postal_code,
            "@MinPSC",
            "@MaxPSC");
    std::string city_condition = xpath_city_match(
            _city,
            city_delimiters_,
            "@nazev");
    std::string district_condition_city = xpath_district_match(
            _city,
            city_delimiters_ + "0123456789",
            "@nazev");
    std::string district_condition_concat = xpath_district_match(
            _city,
            city_delimiters_ + "0123456789",
            "concat(concat(../obec/@nazev, ' '), @nazev)");
    std::string street_condition = xpath_multiword_match(
            _street.first,
            street_delimiters_,
            "@nazev");
    std::string house_number_condition = xpath_house_number_match(
            _street.second,
            {"@o", "@p"});

    static const std::string default_true_value("(1=1)");
    static const std::string default_false_value("(1=0)");

    // default values because of xpath syntax
    postal_code_condition =
        (postal_code_condition.size() == 0)     ? default_false_value : postal_code_condition;
    city_condition =
        (city_condition.size() == 0)            ? default_false_value : city_condition;
    district_condition_city =
        (district_condition_city.size() ==
         0)   ? default_false_value : district_condition_city;
    district_condition_concat =
        (district_condition_concat.size() ==
         0) ? default_false_value : district_condition_concat;
    street_condition =
        (street_condition.size() == 0)          ? default_false_value : street_condition;
    house_number_condition =
        (house_number_condition.size() ==
         0)    ? default_false_value : house_number_condition;

    if (_to_omit.isset())
    {
        switch (_to_omit.get_value())
        {
            case postal_code:
                postal_code_condition = default_true_value;
                break;

            case city:
                city_condition = default_true_value;
                district_condition_city = default_true_value;
                district_condition_concat = default_true_value;
                break;

            case street:
                street_condition = default_true_value;
                break;

            case house_number:
                house_number_condition = default_true_value;
                break;
        }
    }

    std::vector<std::string> result;

    // strict - city ~ city
    result.push_back(
            "count(/adresy/oblast"
            "/obec[" + city_condition + "]"
            "/cast[" + postal_code_condition + "]"
            "/ulice[" + street_condition + "]"
            "/a[" + house_number_condition + "])"
            "> 0");

    // city ~ city district
    result.push_back(
            "count(/adresy/oblast"
            "/obec"
            "/cast["
            "  (" + district_condition_city + ") "
            "  and (" + postal_code_condition + ") "
            "]"
            "/ulice[" + street_condition + "]"
            "/a[" + house_number_condition + "])"
            "> 0");

    // city cca ~ city + city district
    result.push_back(
            "count(/adresy/oblast"
            "/obec"
            "/cast["
            "  (" + district_condition_concat + ") "
            "  and (" + postal_code_condition + ") "
            "]"
            "/ulice[" + street_condition + "]"
            "/a[" + house_number_condition + "])"
            "> 0");

    return result;
}

namespace {

std::string xpath_normalize_chars(const std::string& _input)
{
    static std::string cz_chars_upper
        ("AÁáBCČčDĎďEÉĚéěFGHIÍíJKLMNŇňOÓóPQRŘřSŠšTŤťUÚŮúůVWXYÝýZŽž");
    static std::string cz_chars_normalized
        ("aaabcccdddeeeeefghiiijklmnnnooopqrrrssstttuuuuuvwxyyyzzz");

    return "translate(" + _input + ",'" + cz_chars_upper + "','" + cz_chars_normalized + "')";
}

void erase_chars(
        std::string& _input,
        const std::vector<char>& _chars_to_erase)
{
    // _input.erase(std::remove_if(_input.begin(), _input.end(), boost::is_any_of(_chars_to_erase)) );

    for (std::vector<char>::const_iterator it = _chars_to_erase.begin();
         it != _chars_to_erase.end();
         ++it
         )
    {
        _input.erase(
                std::remove(
                        _input.begin(),
                        _input.end(),
                        *it),
                _input.end());
    }
}

/* splitting input _registry_data into parts containing words with the last word being shortened
 */
TestCzAddress::T_words_shortened parse_multiword(
        const std::string& _registry_data,
        const std::string& _delimiters,
        const std::string& _shortening_delimiters)
{
    TestCzAddress::T_words_shortened result;

    bool word_has_content = false;
    std::string::const_iterator word_begin = _registry_data.begin();

    for (std::string::const_iterator word_end = _registry_data.begin();
         word_end != _registry_data.end();
         ++word_end)
    {
        if (boost::is_any_of(_shortening_delimiters)(*word_end))
        {
            if (word_has_content)
            {
                result.push_back(
                        std::make_pair(
                                std::string(
                                        word_begin,
                                        word_end),
                                true));
            }
            word_begin = word_end;
            word_has_content = false;
        }
        else if (boost::is_any_of(_delimiters)(*word_end))
        {
            if (word_has_content)
            {
                result.push_back(
                        std::make_pair(
                                std::string(
                                        word_begin,
                                        word_end),
                                false));
            }
            word_begin = word_end;
            word_has_content = false;
        }
        else if (!word_has_content)
        {
            word_begin = word_end;
            word_has_content = true;
        }
    }
    // ...if the last character is no delimiter there might be last word left to be pushed to result
    if (word_has_content)
    {
        result.push_back(
                std::make_pair(
                        std::string(
                                word_begin,
                                _registry_data.end()),
                        false));
    }

    return result;
}

TestCzAddress::T_street_data parse_street1(
        const std::string& _street1,
        const std::string& _delimiters,
        const std::string& _shortening_delimiters)
{
    std::pair<TestCzAddress::T_words_shortened, std::vector<std::string> > result;

    result.first = parse_multiword(
            _street1,
            _delimiters,
            _shortening_delimiters);

    static boost::regex regex_house_number("(\\d+[[:alpha:]]?)(?:[\\s,\\.]|$)");

    std::vector<std::string> house_numbers;

    boost::sregex_token_iterator iter(_street1.begin(), _street1.end(), regex_house_number, 0);
    boost::sregex_token_iterator end;
    for (; iter != end; ++iter)
    {
        house_numbers.push_back(*iter);
    }

    if (house_numbers.size() < 1)
    {
        throw ExceptionMissingHouseNumber();
    }
    // using only last two "numbers"
    if (house_numbers.size() > 2)
    {
        std::vector<std::string> temp;
        std::vector<std::string>::const_reverse_iterator it = house_numbers.rbegin();
        temp.push_back(*it);
        ++it;
        if (it != house_numbers.rend())
        {
            temp.push_back(*it);
            std::reverse(
                    temp.begin(),
                    temp.end());
        }

        result.second = temp;
    }
    else
    {
        result.second = house_numbers;
    }

    return result;
}

std::string parse_postal_code(const std::string& _postalcode)
{
    std::string result;

    boost::regex postal_code_pattern("^(\\d{3,3})[\\s-]?(\\d{2,2})$");
    boost::match_results<std::string::const_iterator> captures;

    if (boost::regex_match(
                _postalcode,
                captures,
                postal_code_pattern))
    {
        result.append(
                captures[1].first,
                captures[1].second);
        result.append(
                captures[2].first,
                captures[2].second);
    return result;
    }
    throw ExceptionInvalidPostalCode{};
}

TestCzAddress::T_words_shortened rtrim_numbers(
        const TestCzAddress::T_words_shortened& _input,
        unsigned _nondigit_chars_tolerance_per_word)
{

    TestCzAddress::T_words_shortened result;
    unsigned nondigit_chars = 0;
    bool trimming_finished = false;
    bool has_digit = false;

    // walk throught words
    for (TestCzAddress::T_words_shortened::const_reverse_iterator it = _input.rbegin();
         it != _input.rend();
         ++it)
    {
        nondigit_chars = 0;

        // count non-digits in word
        for (std::string::const_iterator str_it = (*it).first.begin();
             str_it != (*it).first.end();
             ++str_it)
        {
            if (!isdigit(*str_it))
            {
                ++nondigit_chars;
            }
            else
            {
                has_digit = true;
            }

            if (nondigit_chars > _nondigit_chars_tolerance_per_word)
            {
                break;
            }
        }

        if (// trimming from right ends at first nondigit value
            !has_digit
            // if there are more nondigits than limit, we don't trim the word
            || nondigit_chars > _nondigit_chars_tolerance_per_word
            // trimming is continuous, no trimming after first non-trimmed word
            || trimming_finished)
        {
            result.push_back(*it);
            trimming_finished = true;
        }
    }
    std::reverse(
            result.begin(),
            result.end());

    return result;
}

std::string xpath_multiword_match(
        TestCzAddress::T_words_shortened _shortened_words,
        const std::string& _delimiters,
        const std::string& _xpath_lhs)
{
    std::vector<std::string> result;

    std::string get_nth_word_pre;
    std::string get_nth_word_suff;
    std::string nth_word;
    std::string translate_delimiters_pre = "translate(";
    std::string translate_delimiters_suff = ", '" + _delimiters + "', '";
    for (unsigned i = 0; i < _delimiters.size(); ++i)
    {
        // delimiter -> space
        translate_delimiters_suff += " ";
    }
    translate_delimiters_suff += "')";

    std::string lhs_processed =
        "normalize-space("
        + translate_delimiters_pre
        + xpath_normalize_chars(_xpath_lhs)
        + translate_delimiters_suff
        + ")";

    for (TestCzAddress::T_words_shortened::iterator it = _shortened_words.begin();
         it != _shortened_words.end();
         ++it
         )
    {
        nth_word = get_nth_word_pre + lhs_processed + get_nth_word_suff;
        TestCzAddress::T_words_shortened::iterator it_test = it;
        ++it_test;

        // first word in string
        if (it == _shortened_words.begin())
        {
            // ... and at the same time last word in string
            if (it_test == _shortened_words.end())
            {
                if (it->second)
                {
                    // TODO tohle asi neni moc vypovidajici - jednoslovnych nazvu se stejnym pismenem na zacatku jsou mraky
                    result.push_back(
                            " starts-with( " + lhs_processed + ", " +
                            xpath_normalize_chars("'" + it->first + "'") + " ) ");
                }
                else
                {
                    result.push_back(
                            " " + lhs_processed + " = " +
                            xpath_normalize_chars("'" + it->first + "'") + " ");
                }
            }
            else
            {
                if (it->second)
                {
                    result.push_back(
                            " starts-with( substring-before( " + lhs_processed + ", ' ' ), " +
                            xpath_normalize_chars(
                                    "'" + it->first + "'") + " ) ");
                }
                else
                {
                    result.push_back(
                            " substring-before( " + lhs_processed + ", ' ' ) = " +
                            xpath_normalize_chars(
                                    "'" + it->first + "'") + " ");
                }
            }

            // in between words
        }
        else if (it_test != _shortened_words.end())
        {
            if (it->second)
            {
                result.push_back(
                        " starts-with( substring-before(" + nth_word + ", ' '), " +
                        xpath_normalize_chars(
                                "'" + it->first + "'") + ") ");
            }
            else
            {
                result.push_back(
                        " substring-before(" + nth_word + ", ' ') = " +
                        xpath_normalize_chars("'" + it->first + "'") + " ");
            }

            // last word in string
        }
        else
        {
            if (it->second)
            {
                result.push_back(
                        " starts-with( " + nth_word + ", " +
                        xpath_normalize_chars("'" + it->first + "'") + ") ");
            }
            else
            {
                result.push_back(
                        " " + nth_word + " = " + xpath_normalize_chars(
                                "'" + it->first + "'") + " ");
            }
        }

        get_nth_word_pre += "substring-after(";
        get_nth_word_suff += ", ' ')";
    }

    return boost::join(result, " and ");
}

std::string xpath_city_match(
        TestCzAddress::T_words_shortened _shortened_words,
        const std::string& _delimiters,
        const std::string& _xpath_lhs)
{
    return
        xpath_multiword_match(
            _shortened_words,
            _delimiters,
            "translate(" + _xpath_lhs + ",'0123456789', '          ')");
}

std::string xpath_district_match(
        TestCzAddress::T_words_shortened _shortened_words,
        const std::string& _delimiters,
        const std::string& _xpath_lhs)
{
    std::vector<std::string> result;

    do
    {
        result.push_back(
                xpath_multiword_match(
                        _shortened_words,
                        _delimiters,
                        "translate(" + _xpath_lhs + ",'0123456789', '          ')"));
    } while (std::next_permutation(
                     _shortened_words.begin(),
                     _shortened_words.end()));

    return boost::join(result, " or ");
}

std::string xpath_postal_code_match(
        const std::string& _postal_code,
        const std::string& _xpath_min,
        const std::string& _xpath_max)
{
    return _postal_code + ">=" + _xpath_min + " and " + _postal_code + "<=" + _xpath_max;
}

std::string xpath_house_number_match(
        std::vector<std::string> _numbers,
        std::vector<std::string> _xpath_lhs)
{
    std::vector<std::string> result;

    for (const std::string& lhs :_xpath_lhs)
    {
        for (const std::string& value : _numbers)
        {
            // there should be no non-ascii letters in house number (e. g. 22a, 13c ...) so using boost::to_lower is ok here
            result.push_back(
                    xpath_normalize_chars(lhs) + "=" "'" + boost::algorithm::to_lower_copy(value) + "'");
        }
    }
    return boost::join(result, " or ");
}

} // namespace Fred::Backend::Admin::Contact::Verification::{anonymous}

void TestDataProvider<TestCzAddress>::store_data(const LibFred::InfoContactOutput& _data)
{
    street1_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street1);
    street2_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street2.get_value_or_default());
    street3_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street3.get_value_or_default());
    city_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().city);
    postalcode_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().postalcode);
    country_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().country);
}

std::vector<std::string> TestDataProvider<TestCzAddress>::get_string_data() const
{
    return {
            street1_,
            city_,
            postalcode_,
            country_};
}

template <>
std::string test_name<TestCzAddress>()
{
    return "cz_address_existence";
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
