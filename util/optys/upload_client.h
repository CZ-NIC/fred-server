/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  optys mail upload client header
 */

#ifndef UPLOAD_CLIENT_H_1f78f9abca47483e845e3f3625299b50
#define UPLOAD_CLIENT_H_1f78f9abca47483e845e3f3625299b50

#include <vector>
#include <string>
#include <utility>

#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/file.h"

class OptysUploadClient
{
    const std::string host_;
    const int port_;
    const std::string user_;
    const std::string password_;
    const std::string zip_tmp_dir_;
    const bool cleanup_zip_tmp_dir_;
    boost::shared_ptr<Fred::File::Manager> file_manager_;
    std::vector<std::string > zip_file_names_;
    std::map<std::string, Fred::Messages::LetterProcInfo> fm_failed_letters_by_batch_id_;//file manager failed letters by batch_id
    std::vector<std::pair<Fred::Messages::letter_proc, std::string> > fm_failed_letter_batch_id_pairs_;//file manager failed letters
public:
    OptysUploadClient(const std::string& host, int port, const std::string& user, const std::string& password, const std::string& zip_tmp_dir
            , bool cleanup_zip_tmp_dir, boost::shared_ptr<Fred::File::Manager> file_manager);
    OptysUploadClient& zip_letters(const std::map<std::string,Fred::Messages::LetterProcInfo>& message_type_letters_map
        , const std::string& zip_filename_before_message_type
        , const std::string& zip_filename_after_message_type);
    std::map<std::string, Fred::Messages::LetterProcInfo> scp_upload(); //return fm_failed_letters_by_batch_id
};



#endif
