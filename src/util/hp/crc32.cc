/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include <cstdio>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <ios>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <boost/crc.hpp>

const unsigned buffer_size = 10240;

int main(int argc, char** argv)
{
    try
    {
        boost::crc_32_type  crc32;
        for (int i = 1; i < argc; ++i)
        {
            std::ifstream  ifs(argv[i], std::ios_base::binary);
            if(ifs)
                do
                {
                    char  buffer[buffer_size];
                    ifs.read(buffer, buffer_size);
                    crc32.process_bytes(buffer, ifs.gcount());
                } while (ifs);
            else
                std::cerr << "Unable to open file: " << argv[i] << std::endl;
        }//for i

        std::ios_base::fmtflags state = std::cout.flags();//save state
        std::cout << std::hex << std::uppercase << crc32.checksum() << std::endl;
        std::cout.flags(state);//restore state
        return EXIT_SUCCESS;
    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return EXIT_FAILURE;
    }
}//main
