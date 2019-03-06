/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
 *  optys mail upload client header
 */

#ifndef UPLOAD_CLIENT_HH_D029760C64D54E88B81930A7B51BC1D3
#define UPLOAD_CLIENT_HH_D029760C64D54E88B81930A7B51BC1D3

#include <vector>
#include <set>
#include <string>
#include <utility>
#include <stdexcept>


#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "src/deprecated/libfred/file.hh"


class ScpUploadException : public std::runtime_error
{
    std::set<std::string> sent_zip_file_relative_names_;
public:

    ScpUploadException(const std::string& what,
        const std::set<std::string>& sent_zip_file_relative_names)
    : std::runtime_error(what)
    , sent_zip_file_relative_names_(sent_zip_file_relative_names)
    {}

    std::set<std::string> get_sent_zip_file_relative_names() const
    {
        return sent_zip_file_relative_names_;
    }
    virtual ~ScpUploadException() {};
};

/**
 * Send letters via Optys.
 * Download designated letters from FRED file manager, archive them in pkzip file in temp dir with specific file names and upload them via ssh-scp into Optys server.
 * Return "unable to process" letters.
 */
class OptysUploadClient
{
    const std::string host_;
    const int port_;
    const std::string user_;
    const std::string password_;
    const std::string zip_tmp_dir_;
    const bool cleanup_zip_tmp_dir_;
    std::shared_ptr<LibFred::File::Manager> file_manager_;
    std::vector<std::string > zip_file_names_;
    std::map<std::string, LibFred::Messages::LetterProcInfo> failed_letters_by_batch_id_;//failed letters by batch_id
    std::vector<std::pair<LibFred::Messages::letter_proc, std::string> > failed_letter_batch_id_pairs_;//failed letters with batch_id
public:
    OptysUploadClient(const std::string& host, int port, const std::string& user, const std::string& password, const std::string& zip_tmp_dir
            , bool cleanup_zip_tmp_dir, std::shared_ptr<LibFred::File::Manager> file_manager);
    OptysUploadClient& zip_letters(const std::map<std::string, LibFred::Messages::LetterProcInfo>& message_type_letters_map
        , const std::string& zip_filename_before_message_type
        , const std::string& zip_filename_after_message_type);
    std::map<std::string, LibFred::Messages::LetterProcInfo> scp_upload(); //return failed_letters_by_batch_id or throw ScpUploadException
};



#endif
