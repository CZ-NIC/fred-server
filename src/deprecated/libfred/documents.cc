/*
 * Copyright (C) 2007-2021  CZ.NIC, z. s. p. o.
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
#include "util/log/logger.hh"
#include "src/deprecated/libfred/documents.hh"
#include "src/util/subprocess.hh"
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace LibFred
{
  namespace Document
  {
    /// temporary file with unique filename generation and final remove
    class TmpFile : public std::fstream
    {
      const std::string name_str; ///< template of unique name in tmp directory
      std::vector<char> name; ///< buffer of unique name in tmp directory
     public:

    struct OPEN_ERROR : public std::runtime_error
    {
       OPEN_ERROR()
       : std::runtime_error("TmpFile: error in file opening")
       {}
    };
    struct NAME_ERROR : public std::runtime_error
    {
      NAME_ERROR()
       : std::runtime_error("TmpFile: error in name generation")
       {}
    };

      /// initialize unique name 
      TmpFile()
          : name_str("/tmp/fred-gendoc-XXXXXX")
          , name(name_str.begin(), name_str.end())
      {
          name.push_back('\0');//C string end

        mode_t _umask;
        int fd;
        _umask = umask(0077);
        fd = mkstemp(&name[0]);
        umask(_umask);
        if (fd < 0) {
            throw NAME_ERROR();
        }
        ::close(fd);
      }
      /// try to delete file (if exist) and free memory for unique name  
      ~TmpFile()
      {
        remove(&name[0]);
      }
      /// open file with unique name in given mode
      void open(std::ios::openmode mode)
      {
        std::fstream::open(&name[0],mode);
        if (!is_open()) throw OPEN_ERROR();
      }
      /// return unique name
      const char *getName() const
      {
        return &name[0];
      }
    };
    /// description of process of generation
    struct GenProcType 
    {
      GenProcType() 
        : rmlProcessor(true)
      {}
      GenProcType(
        const std::string& _xslTemplateName, bool _rmlProcessor,
        const std::string& _mime
      ) :
        xslTemplateName(_xslTemplateName), rmlProcessor(_rmlProcessor),
        mime(_mime)
      {}
      std::string xslTemplateName; ///< name of file with xsl template
      bool rmlProcessor; ///< include rml processing after xsl tranfsormation
      std::string mime; ///< mime type of generated file 
    };

    /// implementation of Generator interface
    class GeneratorImpl : public Generator
    {
      const std::string& path; ///< path to pdf generator
      const std::string& pathTemplates; ///< path to pdf generator templates
      const std::string& pathFM; ///< path to filemanager client
      const std::string& corbaNS; ///< host with corba nameservice
      const GenProcType& genProc; ///< processing description
      std::ostream* out; ///< output stream if filename is empty
      std::string filename; ///< filename for storage
      unsigned filetype; //< type of generated document from filemanager
      TmpFile bufferFile; ///< simulate streaming (remove?)
      std::stringstream outBuffer; ///< for id generation
      std::string lang; ///< code of language to use;
     public:
      /// initialize generator with streams, path, template and filename
      GeneratorImpl(
        const std::string& _path, const std::string& _pathTemplates,
        const std::string& _pathFM, const std::string& _corbaNS, 
        const GenProcType& _genProc,
        std::ostream* _out, const std::string& _filename, unsigned _filetype,
        const std::string& _lang
      )
        : path(_path), pathTemplates(_pathTemplates), 
          pathFM(_pathFM), corbaNS(_corbaNS),
          genProc(_genProc), out(NULL),
          filename(_filename), filetype(_filetype), lang(_lang)
      {
        bufferFile.open(std::ios::out);
        out = _out ? _out : &outBuffer;
      }
      /// generate document
      TID generate()
      {
        TmpFile outputFile;
        std::stringstream cmd;
        // if template name is empty instead of xslt transformation
        // just echo input to output
        if (genProc.xslTemplateName.empty()) cmd << "cat ";
        else {
          cmd << "xsltproc "
              << "--stringparam srcpath " << pathTemplates << "/templates/" 
              << " ";
          if (!lang.empty())
            cmd << "--stringparam lang " << lang << " ";
          cmd << pathTemplates << "templates/" << genProc.xslTemplateName << " ";
        }
        cmd << bufferFile.getName();
        // if input is rml and processing is needed filter through
        // external application
        if (genProc.rmlProcessor)
          cmd << " | " << path << " ";
        // in case of generation into corba filesystem send result to
        // corba filesystem client
        if (!filename.empty()) {
          cmd << "| " << pathFM 
              << " -m " << genProc.mime 
              << " -s "
              << " -l " << filename
              << " -t " << filetype;
          if (!corbaNS.empty())
            cmd << " -n " << corbaNS;
        }     
        cmd << " > "
            << outputFile.getName();
        LOGGER.debug(boost::format("running shell command: %1%") % cmd.str());

        //if (system(cmd.str().c_str())) throw Generator::ERROR();
        SubProcessOutput output = ShellCmd(cmd.str(), 3600).execute();
        if (!output.stderr.empty())
        {
            LOGGER.error(output.stderr);
            throw Generator::ERROR();
        }

        outputFile.open(std::ios::in);
        // TODO: filemanager_client has to annouce id better
        *out << outputFile.rdbuf();
        if (!filename.empty()) return STR_TO_ID(outBuffer.str().c_str());
        else return 0;
      } 
      virtual std::ostream& getInput()
      {
        return bufferFile;
      }
      virtual TID closeInput()
      {
        bufferFile.close();
        return generate();
      }
    };
    /// implementation of Manager interface
    class ManagerImpl : public Manager
    {
      std::string path; /// path to rml processor
      std::string pathTemplates; /// path to rml processor templates
      std::string pathFM; ///< path to filemanager client
      std::string corbaNS; ///< ns with corba filemanager
      typedef std::map<GenerationType, GenProcType> GenerationMapType;
      GenerationMapType templateMap; ///< list of prepared templates
     public:
      ManagerImpl(
        const std::string& _path, const std::string& _pathTemplates,
        const std::string& _pathFM,
        const std::string& _corbaNS 
      ) : path(_path), pathTemplates(_pathTemplates), 
          pathFM(_pathFM), corbaNS(_corbaNS)
      {
        templateMap[GT_INVOICE_PDF] = GenProcType(
          "invoice.xsl", true, "application/pdf"
        );
        templateMap[GT_ADVANCE_INVOICE_PDF] = GenProcType(
          "advance_invoice.xsl", true, "application/pdf"
        );
        templateMap[GT_AUTHINFO_REQUEST_PDF] = GenProcType(
          "auth_info.xsl", true, "application/pdf"
        );
        templateMap[GT_PUBLIC_REQUEST_PDF] = GenProcType(
          "public_request.xsl", true, "application/pdf"
        );
        templateMap[GT_INVOICE_OUT_XML] = GenProcType(
          "", false, "text/xml"
        );
        templateMap[GT_ACCOUNTING_XML] = GenProcType(
          "", false, "text/xml"
        );
        templateMap[GT_WARNING_LETTER] = GenProcType(
          "warning_letter.xsl", true, "application/pdf"
        );
        templateMap[GT_CONTACT_IDENTIFICATION_LETTER_PIN2] = GenProcType(
          "mojeid_auth_user.xsl", true, "application/pdf"
        );
        templateMap[GT_CONTACT_IDENTIFICATION_LETTER_PIN3] = GenProcType(
          "mojeid_auth_owner.xsl", true, "application/pdf"
        );
        templateMap[GT_CONTACT_REIDENTIFICATION_LETTER_PIN3] = GenProcType(
          "mojeid_auth_owner_change_optys.xsl", true, "application/pdf"
        );
        templateMap[GT_CONTACT_VALIDATION_REQUEST_PIN3] = GenProcType(
          "mojeid_validate.xsl", true, "application/pdf"
        );
        templateMap[GT_CONTACT_VERIFICATION_LETTER_PIN3] = GenProcType(
          "contact_verification_auth_owner.xsl", true, "application/pdf"
        );
        templateMap[GT_ADMIN_CONTACT_VERIFICATION_CONTACT_CHECK_NOTICE] = GenProcType(
          "notice_to_correct_data.xsl", true, "application/pdf"
        );
        templateMap[GT_ADMIN_CONTACT_VERIFICATION_CONTACT_CHECK_THANK_YOU] = GenProcType(
          "confirm_correction.xsl", true, "application/pdf"
        );
        templateMap[GT_CONTACT_IDENTIFICATION_LETTER_PIN3_OPTYS] = GenProcType(
          "mojeid_auth_owner_optys.xsl", true, "application/pdf"
        );
        templateMap[GT_MOJEID_CARD] = GenProcType(
          "mojeid_card_optys.xsl", true, "application/pdf"
        );
        templateMap[GT_RECORD_STATEMENT_DOMAIN] = GenProcType(
          "record_statement_domain.xsl", true, "application/pdf"
        );
        templateMap[GT_RECORD_STATEMENT_NSSET] = GenProcType(
          "record_statement_nsset.xsl", true, "application/pdf"
        );
        templateMap[GT_RECORD_STATEMENT_KEYSET] = GenProcType(
          "record_statement_keyset.xsl", true, "application/pdf"
        );
        templateMap[GT_RECORD_STATEMENT_CONTACT] = GenProcType(
          "record_statement_contact.xsl", true, "application/pdf"
        );

      }
      std::unique_ptr<LibFred::Document::Generator> createOutputGenerator(
        GenerationType type, std::ostream& output,
        const std::string& lang
      ) const
      {
        GenerationMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return std::unique_ptr<LibFred::Document::Generator>(new GeneratorImpl(
          path,pathTemplates,pathFM,corbaNS,i->second,&output,"",0,lang
        ));
      }
      std::unique_ptr<LibFred::Document::Generator> createSavingGenerator(
        GenerationType type, 
        const std::string& filename, unsigned filetype,
        const std::string& lang         
      ) const
      {
        GenerationMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return std::unique_ptr<LibFred::Document::Generator>(new
                GeneratorImpl(path,pathTemplates,pathFM,corbaNS,i->second,NULL
                        ,filename,filetype,lang
        ));
      }
      virtual void generateDocument(
        GenerationType type, 
        std::istream& input, std::ostream& output,
        const std::string& lang
      ) const
      {
        std::unique_ptr<LibFred::Document::Generator> g
          = createOutputGenerator(type,output,lang);
        g->getInput() << input.rdbuf();
        g->closeInput();
      }
      virtual TID generateDocumentAndSave(
        GenerationType type,
        std::istream& input, const std::string& name, unsigned filetype,
        const std::string& lang
      ) const
      {
          std::unique_ptr<LibFred::Document::Generator> g
              = createSavingGenerator(type,name,filetype,lang);
        g->getInput() << input.rdbuf();
        return g->closeInput();
      }
    };
    std::unique_ptr<LibFred::Document::Manager> Manager::create(
      const std::string& path, const std::string& pathTemplates,
      const std::string& pathFM, 
      const std::string& corbaNS
    )
    {
      return std::unique_ptr<LibFred::Document::Manager>(new
              ManagerImpl(path,pathTemplates,pathFM,corbaNS));
    }
  }
} // Fred
