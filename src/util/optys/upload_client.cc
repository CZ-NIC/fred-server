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
 *  optys mail upload client impl
 */

#include <string>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <sys/stat.h>
#include <errno.h>
#include <memory>
#include <boost/format.hpp>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <libssh/libssh.h>

#include "src/util/subprocess.hh"
#include "util/printable.hh"
#include <minizip/zip.h>


#include "src/util/optys/upload_client.hh"

    /**
     * Deleter functor for scp session.
     */
    struct ScpSessionDeleter
    {
        void operator()(ssh_scp s)
        {
            if(s != NULL)
            {
                ssh_scp_close(s);
                ssh_scp_free(s);
            }
        }
    };

    /**
     * Upload files using scp.
     * Create remote files with 0600 rights.
     * Throws mostly std::runtime_error with description of the error.
     */
    class ScpWriteSession
    {
        std::shared_ptr<ssh_session_struct>  ssh_session_;
        std::shared_ptr<ssh_scp_struct>  scp_session_;

    public:
        ScpWriteSession(std::shared_ptr<ssh_session_struct>  ssh_session)
        : ssh_session_(ssh_session)
        , scp_session_(ssh_scp_new(ssh_session.get(), SSH_SCP_WRITE, "."), ScpSessionDeleter())
        {
            if(scp_session_.get() == NULL)
            {
                std::string errmsg("ssh_scp_new failed: ");
                errmsg += ssh_get_error(ssh_session_.get());
                throw std::runtime_error(errmsg);
            }

            if(ssh_scp_init(scp_session_.get()) != SSH_OK)
            {
                std::string errmsg("ssh_scp_init failed: ");
                errmsg += ssh_get_error(ssh_session_.get());
                throw std::runtime_error(errmsg);
            }
        }

        void upload_file(const std::string& file_name, const std::string& file_content)
        {
            if(ssh_scp_push_file(scp_session_.get(), file_name.c_str(), file_content.length(), S_IRUSR | S_IWUSR) != SSH_OK)
            {
                std::string errmsg("Can't open remote file: ");
                errmsg += file_name;
                errmsg += " error: ";
                errmsg += ssh_get_error(ssh_session_.get());
                throw std::runtime_error(errmsg);
            }

            if(ssh_scp_write(scp_session_.get(), file_content.c_str(), file_content.length()) != SSH_OK)
            {
                std::string errmsg("Can't write to remote file: ");
                errmsg += file_name;
                errmsg += " error: ";
                errmsg += ssh_get_error(ssh_session_.get());
                throw std::runtime_error(errmsg);
            }
        }

    };

    /**
     * Deleter functor for ssh session.
     */
    struct SshSessionDeleter
    {
        void operator()(ssh_session s)
        {
            if(s != NULL)
            {
                ssh_disconnect(s);
                ssh_free(s);
            }
        }
    };

    /**
     * Deleter functor for ssh public key hash.
     */
    struct SshPubKeyHashDeleter
    {
        void operator()(unsigned char* h)
        {
            ssh_clean_pubkey_hash(&h);
        }
    };

    /**
     * Underlying ssh session wraper for scp upload.
     * Checking server public key hash against .ssh/known_hosts.
     * Throws mostly std::runtime_error with description of the error.
     */
    class SshSession
    {
        std::shared_ptr<ssh_session_struct>  ssh_session_;

        void check_ssh_known_hosts(ssh_session session)
        {
            int state = ssh_is_server_known(session);

            std::shared_ptr<unsigned char> hash_ptr;
            size_t hlen = 0;
            {
                unsigned char *hash = NULL;

#if LIBSSH_VERSION_MAJOR == 0 && LIBSSH_VERSION_MINOR < 6
                int pubkey_hash_result = 0;
                pubkey_hash_result = ssh_get_pubkey_hash(session, &hash);
                if(pubkey_hash_result < 0) {
                    ssh_clean_pubkey_hash(&hash);
                    throw std::runtime_error("ssh_get_pubkey_hash failed, unable to get buffer with the hash of the public key");
                }
                hlen = boost::numeric_cast<size_t>(pubkey_hash_result);
#else
                ssh_key srv_pubkey = 0;
                if(ssh_get_publickey(session, &srv_pubkey) < 0){
                    ssh_key_free(srv_pubkey);
                    throw std::runtime_error("get the server public key from a session failed");
                }

                if(ssh_get_publickey_hash(
                    srv_pubkey,SSH_PUBLICKEY_HASH_SHA1,&hash,&hlen) < 0) {
                    ssh_key_free(srv_pubkey);
                    ssh_clean_pubkey_hash(&hash);
                    throw std::runtime_error("get the server public key hash failed");
                }

                ssh_key_free(srv_pubkey);
#endif
                if ((hash == NULL) || (hlen == 0))
                {
                    ssh_clean_pubkey_hash(&hash);
                    throw std::runtime_error("unable to get buffer with the hash of the public key");
                }

                hash_ptr = std::shared_ptr<unsigned char>(hash,SshPubKeyHashDeleter());
            }
            std::ostringstream current_public_key_hash;
            current_public_key_hash <<  std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
            std::copy(hash_ptr.get(), hash_ptr.get()+hlen, std::ostream_iterator<unsigned int>(current_public_key_hash, " "));

            switch (state)
            {
            case SSH_SERVER_KNOWN_OK:
                break; /* ok */
            case SSH_SERVER_KNOWN_CHANGED:
                {
                    std::string errmsg("Host key for server has changed, new public key hash is: ");
                    errmsg += current_public_key_hash.str();
                    throw std::runtime_error(errmsg);
                }
                break;
            case SSH_SERVER_FOUND_OTHER:
                {
                    throw std::runtime_error("The host key for this server was not found but an other type of key exists. "
                        "An attacker might change the default server key to confuse your client into thinking the key does not exist.");
                }
                break;
            case SSH_SERVER_FILE_NOT_FOUND:
                {
                    throw std::runtime_error("Could not find known host file.");
                }
                break;
            case SSH_SERVER_NOT_KNOWN:
                {
                    std::string errmsg("The server is unknown. Current public key hash is: ");
                    errmsg += current_public_key_hash.str();
                    throw std::runtime_error(errmsg);
                }
                break;
            case SSH_SERVER_ERROR:
                {
                    std::string errmsg("ssh_is_server_known failed: ");
                    errmsg += ssh_get_error(session);
                    throw std::runtime_error(errmsg);
                }
                break;
            default :
                {
                    throw std::runtime_error("ssh_is_server_known failed");
                }
                break;
            }
        }
    public:
        SshSession(const std::string& host, int port, const std::string& user, const std::string& password)
        : ssh_session_( ssh_new(), SshSessionDeleter())
        {
            if(ssh_session_.get() == 0) throw std::runtime_error("ssh_new failed");

            int log_verbosity = SSH_LOG_PROTOCOL;
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_LOG_VERBOSITY, &log_verbosity);
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_HOST, host.c_str());
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_PORT, &port);
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_USER, user.c_str());

            if (ssh_connect(ssh_session_.get()) != SSH_OK)
            {
                std::string errmsg("ssh_connect failed: ");
                errmsg += ssh_get_error(ssh_session_.get());
                throw std::runtime_error(errmsg);
            }

            check_ssh_known_hosts(ssh_session_.get());

           if (ssh_userauth_password(ssh_session_.get(), NULL, password.c_str()) != SSH_AUTH_SUCCESS)
           {
               std::string errmsg("ssh_userauth_password failed: ");
               errmsg += ssh_get_error(ssh_session_.get());
               throw std::runtime_error(errmsg);
           }
        }

        ScpWriteSession get_scp_write()
        {
            return ScpWriteSession(ssh_session_);
        }
    };

    /**
     * Minizip wrapper.
     * Crates zip archive on disk, add files into archive from in-memory buffer, compressed by deflate algorithm.
     * Throws mostly std::runtime_error with description of the error.
     */
    class CreateZipFile : boost::noncopyable
    {
        zipFile  zip_archive_;
    public:
        zipFile get()
        {
            return zip_archive_;
        }

        ~CreateZipFile()
        {
            if(zip_archive_ != NULL)
            {
                if(zipClose(zip_archive_, NULL) != ZIP_OK)
                {
                    std::cerr << "zipClose failed" << std::endl;
                }
            }

        }
        CreateZipFile(const std::string& zip_file_name)
        {
            std::cout << "CreateZipFile zip_file_name: " << zip_file_name << std::endl;
            zip_archive_ = zipOpen64(zip_file_name.c_str(), APPEND_STATUS_CREATE);
            if(zip_archive_ == NULL)
            {
                std::string errmsg("zipOpen64 with APPEND_STATUS_CREATE flag failed, filename: ");
                errmsg += zip_file_name;
                throw std::runtime_error(errmsg);
            }
            std::cout << "CreateZipFile zip_archive_: " << zip_archive_ << std::endl;
        }

        class OpenNewFileInZipWithClose : boost::noncopyable
        {
            zipFile  _zip_archive_;
        public:
            OpenNewFileInZipWithClose(zipFile  zip_archive, const std::string& file_name)
            : _zip_archive_(zip_archive)
            {
                zip_fileinfo zi = {};//zero all struct fields init

                if(zipOpenNewFileInZip(_zip_archive_, file_name.c_str(), &zi,
                        NULL, 0, NULL, 0, NULL /* comment*/,
                        Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK)
                {
                    std::string errmsg("zipOpenNewFileInZip failed filename: ");
                    errmsg += file_name;
                    throw std::runtime_error(errmsg);
                }
            }

            ~OpenNewFileInZipWithClose()
            {
                if(zipCloseFileInZip(_zip_archive_) != ZIP_OK)
                {
                    std::cerr << "zipCloseFileInZip failed." << std::endl;
                }
            }
        };

        CreateZipFile& add_file(const std::string& file_name, const std::vector<char>& file_content )
        {
            std::cout << "CreateZipFile add_file: " << file_name << std::endl;
            OpenNewFileInZipWithClose new_file_in_zip(zip_archive_, file_name);

            if(zipWriteInFileInZip(zip_archive_, &file_content[0], file_content.size()) != ZIP_OK)
            {
                std::string errmsg("zipWriteInFileInZip failed filename: ");
                errmsg += file_name;
                throw std::runtime_error(errmsg);
            }

            return *this;
        };
    };

    OptysUploadClient::OptysUploadClient(const std::string& host, int port,
        const std::string& user, const std::string& password, const std::string& zip_tmp_dir,
        bool cleanup_zip_tmp_dir, std::shared_ptr<LibFred::File::Manager> file_manager)
    : host_(host)
    , port_(port)
    , user_(user)
    , password_(password)
    , zip_tmp_dir_(zip_tmp_dir)
    , cleanup_zip_tmp_dir_(cleanup_zip_tmp_dir)
    , file_manager_(file_manager)
    {
        if(!boost::filesystem::exists(zip_tmp_dir))
        {
            throw std::runtime_error(std::string("zip_tmp_dir: " + zip_tmp_dir +" not found"));
        }

        if(cleanup_zip_tmp_dir_)
        {
            std::string cleanup_command = std::string("rm -f ")+zip_tmp_dir_+"/*.zip";
            SubProcessOutput output = ShellCmd(cleanup_command.c_str(), 3600).execute();
            if (!output.stderr.empty() || !output.is_exited() || (output.get_exit_status() != EXIT_SUCCESS))
            {
                throw std::runtime_error(std::string("cleanup_zip_tmp_dir command: " + cleanup_command +" failed: ")+output.stderr);
            }
        }
    }

    OptysUploadClient& OptysUploadClient::zip_letters(
        const std::map<std::string, LibFred::Messages::LetterProcInfo>& message_type_letters_map
        , const std::string& zip_filename_before_message_type
        , const std::string& zip_filename_after_message_type)
    {
        //zip filename
        for(std::map<std::string, LibFred::Messages::LetterProcInfo>::const_iterator
            ci = message_type_letters_map.begin();
            ci != message_type_letters_map.end(); ++ci)
        {
            std::string batch_id = zip_filename_before_message_type+ ci->first + zip_filename_after_message_type;//have to match batch_id in set status
            std::string zip_file_name = batch_id + ".zip";

            CreateZipFile zip_file(zip_tmp_dir_+"/" + zip_file_name);
            //process letter ids
            LibFred::Messages::LetterProcInfo letters = ci->second;
            for(std::size_t i = 0; i < letters.size(); ++i)
            {
                unsigned long long file_id = letters[i].file_id;
                unsigned long long letter_id = letters[i].letter_id;

                std::string letter_file_name = boost::lexical_cast<std::string>(letter_id) + ".pdf";

                std::vector<char> file_buffer;
                try
                {
                    file_manager_->download(file_id,file_buffer);
                }
                catch (std::exception &ex)
                {
                    LOGGER.error(boost::format("filemanager download: '%1%' error processing letter_id: %2% file_id: %3%") % ex.what()
                         % letters[i].letter_id % letters[i].file_id );
                    failed_letters_by_batch_id_[batch_id].push_back(letters[i]);//save failed letter
                    continue;
                }

                zip_file.add_file(letter_file_name, file_buffer);
            }//for

            //zip_close(zip_file.get());

            zip_file_names_.push_back(zip_file_name);//set zip file for upload
        }


        return *this;
    }

    std::map<std::string, LibFred::Messages::LetterProcInfo>  OptysUploadClient::scp_upload()
    {
        std::set<std::string> sent_files;
        try
        {
            ScpWriteSession scp_write_session = SshSession(host_, port_, user_, password_).get_scp_write();

            for(std::vector<std::string >::const_iterator ci = zip_file_names_.begin();
                ci != zip_file_names_.end(); ++ci)
            {
                //open zip file
                std::ifstream zip_file_stream;
                std::string zip_file_name = zip_tmp_dir_+"/" + (*ci);
                zip_file_stream.open (zip_file_name.c_str(), std::ios::in | std::ios::binary);
                if(!zip_file_stream.is_open()) throw std::runtime_error("zip_file_stream.open failed, "
                    "unable to open file: "+ zip_file_name);

                //find end of zip file
                std::string zip_file_content;//buffer
                zip_file_stream.seekg (0, std::ios::end);
                long long zip_file_length = zip_file_stream.tellg();
                zip_file_stream.seekg (0, std::ios::beg);//reset

                //allocate zip file buffer
                zip_file_content.resize(zip_file_length);

                //read zip file
                zip_file_stream.read(&zip_file_content[0], zip_file_content.size());

                //upload
                scp_write_session.upload_file((*ci), zip_file_content);

                //note uploaded files
                sent_files.insert(*ci);
            }
        }
        catch(const std::exception& ex)
        {
            throw ScpUploadException(ex.what(), sent_files);
        }
        catch(...)
        {
            throw ScpUploadException("unknown exception", sent_files);
        }
        std::cout << "Sent: " << Util::format_container(sent_files, ", ") << std::endl;
        return failed_letters_by_batch_id_;
    }

