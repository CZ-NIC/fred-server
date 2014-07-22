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

#include "src/admin/contact/verification/test_impl/test_cz_address_exists.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include "src/fredlib/opexception.h"

#include <fredlib/contact.h>

#include "util/util.h"

#include <cctype>
#include <iterator>
#include <locale>
#include <algorithm>    // std::reverse, std::next_permutaion

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/assign/list_of.hpp>

namespace Admin {
namespace ContactVerification {

    FACTORY_MODULE_INIT_DEFI(TestCzAddress_init)

    static void erase_chars(std::string& _input, const std::vector<char>& _chars_to_erase);

    static TestCzAddress::T_words_shortened parse_multiword(const std::string& _registry_data, const std::string& _delimiters, const std::string& _shortening_delimiters);
    static TestCzAddress::T_street_data parse_street1(const std::string& _street1, const std::string& _delimiters, const std::string& _shortening_delimiters);
    static std::string parse_postal_code(const std::string& _postalcode);

    static TestCzAddress::T_words_shortened rtrim_numbers(const TestCzAddress::T_words_shortened& _input, unsigned _nondigit_chars_tolerance_per_word = 0);

    static std::string xpath_normalize_chars(const std::string& _input);
    static std::string xpath_multiword_match(TestCzAddress::T_words_shortened _shortened_words, const std::string& _delimiters, const std::string& _xpath_lhs);
    static std::string xpath_city_match(TestCzAddress::T_words_shortened _shortened_words, const std::string& _delimiters, const std::string& _xpath_lhs);
    static std::string xpath_district_match(TestCzAddress::T_words_shortened _shortened_words, const std::string& _delimiters, const std::string& _xpath_lhs);
    static std::string xpath_postal_code_match(const std::string& _postal_code, const std::string& _xpath_min, const std::string& _xpath_max);
    static std::string xpath_house_number_match(std::vector<std::string> _numbers, std::vector<std::string> _xpath_lhs);

    const std::string TestCzAddress::street_delimiters_("\\/-,()| \t\n\r");
    const std::string TestCzAddress::street_shortened_word_signs_(".");
    const std::string TestCzAddress::city_delimiters_("-()\\, \t\n\r");
    const std::string TestCzAddress::city_shortened_word_signs_("./");

    TestCzAddress& TestCzAddress::set_mvcr_address_xml_filename(const std::string& _mvcr_address_xml_filename) {
        /* Load XML document */
        doc_ = xmlParseFile(_mvcr_address_xml_filename.c_str());
        if (doc_ == NULL) {
            throw Fred::InternalError("Error: unable to parse file " + _mvcr_address_xml_filename);
        }

        /* Create xpath evaluation context */
        xpathCtx_ = xmlXPathNewContext(doc_);
        if(xpathCtx_ == NULL) {
            xmlFreeDoc(doc_);
            throw Fred::InternalError("Error: unable to create new XPath context");
        }

        return *this;
    }

    TestCzAddress::~TestCzAddress() {
        xmlXPathFreeContext(xpathCtx_);
        xmlFreeDoc(doc_);
    }
    Test::TestRunResult TestCzAddress::run(unsigned long long _history_id) const {
        using std::string;
        using std::vector;

        TestDataProvider<TestCzAddress> data;
        data.init_data(_history_id);

        // can use boost to_lower because in there should be no utf8 chars in country_code
        string country =  boost::algorithm::to_lower_copy(data.country_);

        if(country != "cz") {
            return TestRunResult (Fred::ContactTestStatus::SKIPPED, string("this test is intended for CZ addresses only") );
        }

        // registry data
        T_street_data street;
        T_words_shortened city;
        string postal_code;

        try {
            string normalized_street1(data.street1_);
            boost::algorithm::trim(normalized_street1);
            // TODO erase č.p. variants
            street = parse_street1(
                normalized_street1,
                street_delimiters_,
                street_shortened_word_signs_);
            street.first = rtrim_numbers(street.first, 2);

            string normalized_city(data.city_);
            boost::algorithm::trim(normalized_city);
            // TODO spise vyhodit jen 2ciferna cisla
            erase_chars(
                normalized_city,
                Util::vector_of<char>('0')('1')('2')('3')('4')('5')('6')('7')('8')('9')
            );

            city = parse_multiword(
                normalized_city,
                city_delimiters_,
                city_shortened_word_signs_);

            if(city.size() < 1) {
                return TestRunResult (Fred::ContactTestStatus::FAIL, string("city is missing content") );
            }
            postal_code = parse_postal_code(static_cast<string>(data.postalcode_));
        } catch (...) {
            return TestRunResult (Fred::ContactTestStatus::FAIL, string("exception during parsing") );
        }

        try {
            if(is_address_valid(street, city, postal_code)) {
                return TestRunResult (Fred::ContactTestStatus::OK, string() );
            }
        } catch (...) {
            return TestRunResult (Fred::ContactTestStatus::FAIL, string("exception during validation") );
        }

        std::string error_msg;
        try {
            error_msg = diagnose_problem(street, city, postal_code);
        } catch (...) {
            return TestRunResult (Fred::ContactTestStatus::FAIL, string("exception during diagnostics") );
        }



        return TestRunResult (Fred::ContactTestStatus::FAIL, error_msg );
    }

    bool TestCzAddress::is_address_valid(
        const TestCzAddress::T_street_data& street,
        const TestCzAddress::T_words_shortened& city,
        const std::string& postal_code
    ) const {
        using std::string;
        using std::vector;
        using boost::assign::list_of;

        vector<string> xpath_queries = generate_xpath_queries(street, city, postal_code);

        xmlErrorPtr error = NULL;

        BOOST_FOREACH(const string& query, xpath_queries) {
            boost::scoped_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(
                reinterpret_cast<const unsigned char*>(query.c_str()),
                xpathCtx_)
            );

            error = xmlGetLastError();
            if(error != NULL) {
                string error_msg_str(error->message);
                xmlResetLastError();
                throw Fred::InternalError("Error: xpath error " + error_msg_str);
            }

            if(xpathObj == NULL) {
                throw Fred::InternalError("Error: unable to evaluate xpath expression " + query);
            }

            if(xmlXPathCastToBoolean(xpathObj.get())) {
                return true;
            }
        }

        return false;
    }

    std::string TestCzAddress::diagnose_problem(
        const TestCzAddress::T_street_data& _street,
        const TestCzAddress::T_words_shortened& _city,
        const std::string& _postal_code
    ) const {
        using std::string;
        using std::vector;
        using boost::assign::list_of;

        vector<string> xpath_queries;
        std::map<string, string> suspicious_data_map;

        vector<string> testing_xpath_queries;

        testing_xpath_queries = generate_xpath_queries(_street, _city, _postal_code, house_number);
        BOOST_FOREACH(const string& query, testing_xpath_queries) {
            xpath_queries.push_back(query);
            suspicious_data_map[xpath_queries.back()] = "house number";
        }

        testing_xpath_queries = generate_xpath_queries(_street, _city, _postal_code, street);
        BOOST_FOREACH(const string& query, testing_xpath_queries) {
            xpath_queries.push_back(query);
            suspicious_data_map[xpath_queries.back()] = "street";
        }

        testing_xpath_queries = generate_xpath_queries(_street, _city, _postal_code, city);
        BOOST_FOREACH(const string& query, testing_xpath_queries) {
            xpath_queries.push_back(query);
            suspicious_data_map[xpath_queries.back()] = "city";
        }

        testing_xpath_queries = generate_xpath_queries(_street, _city, _postal_code, postal_code);
        BOOST_FOREACH(const string& query, testing_xpath_queries) {
            xpath_queries.push_back(query);
            suspicious_data_map[xpath_queries.back()] = "postal code";
        }

        std::set<string> suspicious_data;

        xmlErrorPtr error = NULL;
        BOOST_FOREACH(const string& query, xpath_queries) {
            boost::scoped_ptr<xmlXPathObject> xpathObj(xmlXPathEvalExpression(
                reinterpret_cast<const unsigned char*>(query.c_str()),
                xpathCtx_)
            );

            error = xmlGetLastError();
            if(error != NULL) {
                string error_msg_str(error->message);
                xmlResetLastError();
                throw Fred::InternalError("Error: xpath error " + error_msg_str);
            }

            if(xpathObj == NULL) {
                throw Fred::InternalError("Error: unable to evaluate xpath expression " + query);
            }

            if(xmlXPathCastToBoolean(xpathObj.get())) {
                suspicious_data.insert(suspicious_data_map[query]);
            }
        }

        if( !suspicious_data.empty() ) {
            return boost::join(suspicious_data, " or ") + " might be invalid";
        }

        return std::string();
    }

    std::vector<std::string> TestCzAddress::generate_xpath_queries(
        TestCzAddress::T_street_data _street,
        TestCzAddress::T_words_shortened _city,
        std::string _postal_code,
        Optional<address_part> _to_ommit
    ) const {
        using std::string;
        using boost::assign::list_of;

        string postal_code_condition = xpath_postal_code_match(_postal_code, "@MinPSC", "@MaxPSC");
        string city_condition = xpath_city_match(_city, city_delimiters_, "@nazev");
        string district_condition_city = xpath_district_match(_city, city_delimiters_ + "0123456789", "@nazev");
        string district_condition_concat = xpath_district_match(_city, city_delimiters_ + "0123456789", "concat(concat(../obec/@nazev, ' '), @nazev)");
        string street_condition = xpath_multiword_match(_street.first, street_delimiters_, "@nazev");
        string house_number_condition = xpath_house_number_match(_street.second, list_of("@o")("@p") );

        static const string default_true_value("(1=1)");
        static const string default_false_value("(1=0)");

        // default values because of xpath syntax
        postal_code_condition =     (postal_code_condition.size() == 0)     ? default_false_value : postal_code_condition;
        city_condition =            (city_condition.size() == 0)            ? default_false_value : city_condition;
        district_condition_city =   (district_condition_city.size() == 0)   ? default_false_value : district_condition_city;
        district_condition_concat = (district_condition_concat.size() == 0) ? default_false_value : district_condition_concat;
        street_condition =          (street_condition.size() == 0)          ? default_false_value : street_condition;
        house_number_condition =    (house_number_condition.size() == 0)    ? default_false_value : house_number_condition;

        if(_to_ommit.isset()) {
            switch(_to_ommit.get_value()) {
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
        result.push_back(string() +
            "count(/adresy/oblast"
            "/obec["+ city_condition +"]"
            "/cast["+ postal_code_condition +"]"
            "/ulice["+ street_condition +"]"
            "/a["+ house_number_condition +"])"
            "> 0");

        // city ~ city district
        result.push_back(string() +
            "count(/adresy/oblast"
            "/obec"
            "/cast["
            "  ("+ district_condition_city +") "
            "  and ("+ postal_code_condition +") "
            "]"
            "/ulice["+ street_condition +"]"
            "/a["+ house_number_condition +"])"
            "> 0");

        // city cca ~ city + city district
        result.push_back(string() +
            "count(/adresy/oblast"
            "/obec"
            "/cast["
            "  ("+ district_condition_concat +") "
            "  and ("+ postal_code_condition +") "
            "]"
            "/ulice["+ street_condition +"]"
            "/a["+ house_number_condition +"])"
            "> 0");

        return result;
    }

    std::string xpath_normalize_chars(const std::string& _input) {
        static std::string cz_chars_upper
            ("AÁáBCČčDĎďEÉĚéěFGHIÍíJKLMNŇňOÓóPQRŘřSŠšTŤťUÚŮúůVWXYÝýZŽž");
        static std::string cz_chars_normalized
            ("aaabcccdddeeeeefghiiijklmnnnooopqrrrssstttuuuuuvwxyyyzzz");

        return "translate(" + _input + ",'" + cz_chars_upper + "','" + cz_chars_normalized + "')";
    }

    void erase_chars(std::string& _input, const std::vector<char>& _chars_to_erase) {
        //_input.erase(std::remove_if(_input.begin(), _input.end(), boost::is_any_of(_chars_to_erase)) );

        for (std::vector<char>::const_iterator it = _chars_to_erase.begin();
            it != _chars_to_erase.end();
            ++it
        ) {
            _input.erase (std::remove(_input.begin(), _input.end(), *it), _input.end());
        }
    }

    /* splitting input _registry_data into parts containing words with the last word being shortened
     */
    TestCzAddress::T_words_shortened parse_multiword(const std::string& _registry_data, const std::string& _delimiters, const std::string& _shortening_delimiters) {
        using std::string;
        using std::vector;
        TestCzAddress::T_words_shortened result;

        bool word_has_content = false;
        string::const_iterator word_begin = _registry_data.begin();

        for(string::const_iterator word_end = _registry_data.begin();
            word_end != _registry_data.end();
            ++word_end
        ) {
            if(boost::is_any_of(_shortening_delimiters)(*word_end)) {
                if(word_has_content) {
                    result.push_back(std::make_pair(string(word_begin, word_end), true));
                }
                word_begin = word_end;
                word_has_content = false;
            } else if(boost::is_any_of(_delimiters)(*word_end)) {
                if(word_has_content) {
                    result.push_back(std::make_pair(string(word_begin, word_end), false));
                }
                word_begin = word_end;
                word_has_content = false;
            } else if( !word_has_content ) {
                word_begin = word_end;
                word_has_content = true;
            }
        }
        // ...if the last character is no delimiter there might be last word left to be pushed to result
        if(word_has_content) {
            result.push_back(std::make_pair(string(word_begin, _registry_data.end()), false));
        }

        return result;
    }

    TestCzAddress::T_street_data parse_street1(const std::string& _street1, const std::string& _delimiters, const std::string& _shortening_delimiters) {
        using std::vector;
        using std::string;

        std::pair<TestCzAddress::T_words_shortened, std::vector<std::string> > result;

        result.first = parse_multiword(_street1, _delimiters, _shortening_delimiters);

        static boost::regex regex_house_number("(\\d+[[:alpha:]]?)(?:[\\s,\\.]|$)");

        std::vector<std::string> house_numbers;

        boost::sregex_token_iterator iter(_street1.begin(), _street1.end(), regex_house_number, 0);
        boost::sregex_token_iterator end;
        for( ; iter != end; ++iter ) {
           house_numbers.push_back(*iter);
        }

        if(house_numbers.size() < 1) {
           throw Fred::InternalError("no house number");
        }
        // using only last two "numbers"
        if(house_numbers.size() > 2) {
            vector<string> temp;
            vector<string>::const_reverse_iterator it = house_numbers.rbegin();
            temp.push_back(*it);
            ++it;
            if(it != house_numbers.rend()) {
                temp.push_back(*it);
                std::reverse(temp.begin(), temp.end());
            }

            result.second = temp;
        } else {
            result.second = house_numbers;
        }

        return result;
    }

    std::string parse_postal_code(const std::string& _postalcode) {
        std::string result;

        boost::regex postal_code_pattern("^(\\d{3,3})[\\s-]?(\\d{2,2})$");
        boost::match_results<std::string::const_iterator> captures;

        if(boost::regex_match(_postalcode, captures, postal_code_pattern)) {
            result.append(captures[1].first, captures[1].second);
            result.append(captures[2].first, captures[2].second);
        } else {
            throw Fred::InternalError("invalid postal code");
        }

        return result;
    }

    TestCzAddress::T_words_shortened rtrim_numbers(const TestCzAddress::T_words_shortened& _input, unsigned _nondigit_chars_tolerance_per_word) {

        TestCzAddress::T_words_shortened result;
        unsigned nondigit_chars = 0;
        bool trimming_finished = false;
        bool has_digit = false;

        // walk throught words
        for(TestCzAddress::T_words_shortened::const_reverse_iterator it = _input.rbegin();
            it != _input.rend();
            ++it
        ) {
            nondigit_chars = 0;

            // count non-digits in word
            for(std::string::const_iterator str_it = (*it).first.begin();
                str_it != (*it).first.end();
                ++str_it
            ) {
                if( !isdigit(*str_it) ) {
                    ++nondigit_chars;
                } else {
                    has_digit = true;
                }

                if(nondigit_chars > _nondigit_chars_tolerance_per_word) {
                    break;
                }
            }

            if( // trimming from right ends at first nondigit value
                ! has_digit
                // if there are more nondigits than limit, we don't trim the word
                || nondigit_chars > _nondigit_chars_tolerance_per_word
                // trimming is continuous, no trimming after first non-trimmed word
                || trimming_finished
            ) {
                result.push_back(*it);
                trimming_finished = true;
            }
        }
        std::reverse(result.begin(), result.end());

        return result;
    }

    std::string xpath_multiword_match(TestCzAddress::T_words_shortened _shortened_words, const std::string& _delimiters, const std::string& _xpath_lhs) {
        using std::string;
        std::vector<std::string> result;

        std::string get_nth_word_pre;
        std::string get_nth_word_suff;
        std::string nth_word;
        std::string translate_delimiters_pre = "translate(";
        std::string translate_delimiters_suff = ", '"+ _delimiters + "', '";
        for(unsigned i=0; i<_delimiters.size(); ++i) {
            // delimiter -> space
            translate_delimiters_suff += " ";
        }
        translate_delimiters_suff += "')";

        std::string lhs_processed =
            "normalize-space("
                + translate_delimiters_pre
                    + xpath_normalize_chars(_xpath_lhs)
                + translate_delimiters_suff
            +")";

        for(TestCzAddress::T_words_shortened::iterator it = _shortened_words.begin();
            it != _shortened_words.end();
            ++it
        ) {
            nth_word = get_nth_word_pre + lhs_processed + get_nth_word_suff;
            TestCzAddress::T_words_shortened::iterator it_test = it;
            ++it_test;

            // first word in string
            if(it == _shortened_words.begin()) {
                // ... and at the same time last word in string
                if(it_test == _shortened_words.end()) {
                    if(it->second) {
                        // TODO tohle asi neni moc vypovidajici - jednoslovnych nazvu se stejnym pismenem na zacatku jsou mraky
                        result.push_back(
                            " starts-with( "+lhs_processed+", " + xpath_normalize_chars( string("'") + it->first + "'" ) + " ) ");
                    } else {
                        result.push_back(
                            " "+lhs_processed+" = " + xpath_normalize_chars( string("'") + it->first + "'" ) + " ");
                    }
                } else {
                    if(it->second) {
                        result.push_back(
                            " starts-with( substring-before( "+lhs_processed+", ' ' ), " + xpath_normalize_chars( string("'") + it->first + "'" ) + " ) ");
                    } else {
                        result.push_back(
                            " substring-before( "+lhs_processed+", ' ' ) = " + xpath_normalize_chars( string("'") + it->first + "'" ) + " ");
                    }
                }

            // in between words
            } else if(it_test != _shortened_words.end()) {
                if(it->second) {
                    result.push_back(" starts-with( substring-before(" + nth_word + ", ' '), " + xpath_normalize_chars( string("'") + it->first + "'" ) + ") ");
                } else {
                    result.push_back(" substring-before("+ nth_word + ", ' ') = " + xpath_normalize_chars( string("'") + it->first + "'" ) + " ");
                }

            // last word in string
            } else {
                if(it->second) {
                    result.push_back(" starts-with( " + nth_word + ", " + xpath_normalize_chars( string("'") + it->first + "'" ) + ") ");
                } else {
                    result.push_back(" " + nth_word + " = " + xpath_normalize_chars( string("'") + it->first + "'" ) + " ");
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
        const std::string& _xpath_lhs
    ) {
        return
            xpath_multiword_match(
                _shortened_words,
                _delimiters,
                "translate("+_xpath_lhs+",'0123456789', '          ')"
            );
    }

    std::string xpath_district_match(TestCzAddress::T_words_shortened _shortened_words, const std::string& _delimiters, const std::string& _xpath_lhs) {
        std::vector<std::string> result;

        do {
            result.push_back(
                xpath_multiword_match(
                    _shortened_words,
                    _delimiters,
                    "translate("+_xpath_lhs+",'0123456789', '          ')"
                )
            );
        } while(std::next_permutation(_shortened_words.begin(), _shortened_words.end()));

        return boost::join(result, " or ");
    }

    std::string xpath_postal_code_match(const std::string& _postal_code, const std::string& _xpath_min, const std::string& _xpath_max) {
        return _postal_code+">="+_xpath_min+" and "+_postal_code+"<="+_xpath_max;
    }

    std::string xpath_house_number_match(std::vector<std::string> _numbers, std::vector<std::string> _xpath_lhs) {
        std::vector<std::string> result;

        BOOST_FOREACH(const std::string& lhs, _xpath_lhs) {
            BOOST_FOREACH(const std::string& value, _numbers) {
                // there should be no non-ascii letters in house number (e. g. 22a, 13c ...) so using boost::to_lower is ok here
                result.push_back(xpath_normalize_chars(lhs) + "=" "'" + boost::algorithm::to_lower_copy(value) + "'");
            }
        }
        return boost::join(result, " or ");
    }
}
}
