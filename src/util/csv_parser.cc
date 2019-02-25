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
 *  csv parser
 */


#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <iterator>

#include "src/util/csv_parser.hh"

namespace Util
{
    std::vector<std::vector<std::string> > CsvParser::parse()
    {
        std::vector<std::vector<std::string> > csv_data;
        if(csv_data_.empty()) return csv_data;//empty input

        unsigned long long field_start_index = 0;//start of the field index
        unsigned long long field_end_index = 0;//end of the field index

        csv_data.push_back(std::vector<std::string>());//input not empty, add row

        do
        {
            if(csv_data_.at(field_start_index) == quotation_mark_)
            {//quoted field
                if((csv_data_.length() - field_start_index) < 2) throw std::runtime_error("invalid csv");//field length 2 suffices for quoted empty field case: ""
                unsigned long long quotation_mark_count = 0;//number of encountered quotation marks in current field
                for(unsigned long long i = field_start_index + 1; i < csv_data_.length(); ++i)//look for the end of the field
                {
                    if(csv_data_.at(i) == quotation_mark_)//look for next quotation mark
                    {
                        ++quotation_mark_count;

                        if(quotation_mark_count % 2 != 0)//odd quotation mark in the field
                        {
                            if(((csv_data_.length() - field_start_index)) == 2)
                            {
                                csv_data.back().push_back("");//add last empty field
                                return csv_data;
                            }
                            else
                            {
                                if((csv_data_.length() <= (i + 2)) || (csv_data_.at(i + 1) != quotation_mark_))//look for quoted quotation mark
                                {
                                    field_end_index = i;

                                    if((field_end_index - field_start_index) < 2)//empty
                                    {
                                        csv_data.back().push_back("");//add empty field
                                        break;//exit field "for loop"
                                    }
                                    else
                                    {
                                        csv_data.back().push_back(
                                            boost::algorithm::replace_all_copy(//unquote quoted quotes
                                            csv_data_.substr(field_start_index+1,field_end_index - 1 - field_start_index)
                                            , std::string(2,quotation_mark_), std::string(1,quotation_mark_)));//add non-empty field
                                        break;//exit field "for loop"
                                    }
                                }
                            }
                        }

                        if((quotation_mark_count % 2 == 0) && (csv_data_.at(i - 1) != quotation_mark_))//even quotation mark in the field check
                        {
                            throw std::runtime_error("missing quotation mark");
                        }
                    }
                    else
                    {
                        if((i+1) == csv_data_.length()) throw std::runtime_error("missing quotation mark at the end");
                    }
                }//look for the end of the field
            }
            else
            {//non-quoted field
                for(unsigned long long i = field_start_index; i < csv_data_.length(); ++i)//look for the end of the field
                {
                    if((csv_data_.at(i) == delimiter_) || (csv_data_.at(i) == '\n') || (csv_data_.at(i) == '\r'))//look for next delimiter or newline
                    {
                        field_end_index = i - 1;
                        csv_data.back().push_back(
                            boost::algorithm::replace_all_copy(//unquote quoted quotes
                            csv_data_.substr(field_start_index,field_end_index + 1 - field_start_index)
                            , std::string(2,quotation_mark_), std::string(1,quotation_mark_)));//add non-empty field
                        break;//exit field "for loop"
                    }

                    if((i + 1) == csv_data_.length())//look for end of input data
                    {
                        field_end_index = i;
                        csv_data.back().push_back(
                            boost::algorithm::replace_all_copy(//unquote quoted quotes
                            csv_data_.substr(field_start_index,field_end_index + 1 - field_start_index)
                            , std::string(2,quotation_mark_), std::string(1,quotation_mark_)));//add last non-empty field
                        return csv_data;
                    }
                }//look for the end of the field
            }

            if(csv_data_.length() >= (field_end_index + 2))
            {
                //find start of next field
                if(csv_data_.at(field_end_index + 1) == delimiter_)//next in row
                {
                    field_start_index = field_end_index + 2;
                }

                if((csv_data_.at(field_end_index + 1) == '\n') || (csv_data_.at(field_end_index + 1) == '\r'))//new row
                {
                    field_start_index = field_end_index + 2;

                    if((field_end_index + 3) <= csv_data_.length())//check next 2 chars behind end exists
                    {
                        //look for next newline character
                        if((csv_data_.at(field_end_index + 1) == '\r') && (csv_data_.at(field_end_index + 2) == '\n'))//new row for windows
                        {
                            field_start_index = field_end_index + 3;
                        }
                    }

                    csv_data.push_back(std::vector<std::string>());//add new row
                }
            }
        } while(((field_start_index + 1) >= (field_end_index + 1)) && ((field_start_index + 1) <= csv_data_.length()));

        if(csv_data.back().empty()) csv_data.pop_back();//remove last row if empty

        return csv_data;
    }

}


