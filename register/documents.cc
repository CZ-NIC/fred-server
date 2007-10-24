#include "documents.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace Register
{
  namespace Document
  {
    /// temporary file with unique filename generation and final remove
    class TmpFile : public std::fstream
    {
      #define NAME_TEMPLATE "/tmp/fred-gendoc-XXXXXX"
      char *name; ///< unique name in tmp directory
     public:
      class OPEN_ERROR {}; ///< error in file opening
      class NAME_ERROR {}; ///< error in name generation
      /// initialize unique name 
      TmpFile() throw (OPEN_ERROR)
      {
        mode_t _umask;
        int fd;
        name = (char *)malloc(strlen(NAME_TEMPLATE)+1);
        strcpy(name,NAME_TEMPLATE);
        _umask = umask(0077);
        fd = mkstemp(name);
        umask(_umask);
        if (fd < 0) {
            free(name);
            throw NAME_ERROR();
        }
        ::close(fd);
      }
      /// try to delete file (if exist) and free memory for unique name  
      ~TmpFile()
      {
        remove(name);
        free(name);
      }
      /// open file with unique name in given mode
      void open(std::ios::openmode mode) throw (OPEN_ERROR)
      {
        std::fstream::open(name,mode);
        if (!is_open()) throw OPEN_ERROR();
      }
      /// return unique name
      const char *getName() const
      {
        return name;
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
      ) throw (TmpFile::NAME_ERROR,TmpFile::OPEN_ERROR)
        : path(_path), pathTemplates(_pathTemplates), 
          pathFM(_pathFM), corbaNS(_corbaNS),
          genProc(_genProc), out(NULL),
          filename(_filename), filetype(_filetype), lang(_lang)
      {
        bufferFile.open(std::ios::out);
        out = _out ? _out : &outBuffer;
      }
      /// generate document
      TID generate() throw (Generator::ERROR)
      {
        TmpFile outputFile;
        std::stringstream cmd;
        // if template name is empty instead of xslt transformation
        // just echo input to output
        if (genProc.xslTemplateName.empty()) cmd << "cat ";
        else {
          cmd << "xsltproc "
              << "--stringparam srcpath " << pathTemplates << "templates/" 
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
          cmd << "| python " << pathFM 
              << " -m " << genProc.mime 
              << " -s "
              << " -l " << filename
              << " -t " << filetype;
          if (!corbaNS.empty())
            cmd << " -n " << corbaNS;
        }     
        cmd << " > "
            << outputFile.getName();
        if (system(cmd.str().c_str())) throw Generator::ERROR();
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
      virtual TID closeInput() throw (Generator::ERROR)
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
        templateMap[GT_INVOICE_OUT_XML] = GenProcType(
          "", false, "text/xml"
        );
        templateMap[GT_ACCOUNTING_XML] = GenProcType(
          "", false, "text/xml"
        );
        templateMap[GT_WARNING_LETTER] = GenProcType(
          "warning_letter.xsl", true, "application/pdf"
        );
      }      
      virtual Generator *createOutputGenerator(
        GenerationType type, std::ostream& output,
        const std::string& lang
      ) const throw (Generator::ERROR)
      {
        GenerationMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return new GeneratorImpl(
          path,pathTemplates,pathFM,corbaNS,i->second,&output,"",0,lang
        );
      }
      virtual Generator *createSavingGenerator(
        GenerationType type, 
        const std::string& filename, unsigned filetype,
        const std::string& lang         
      ) const throw (Generator::ERROR)
      {
        GenerationMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return new GeneratorImpl(
          path,pathTemplates,pathFM,corbaNS,i->second,NULL,
          filename,filetype,lang
        );
      }
      virtual void generateDocument(
        GenerationType type, 
        std::istream& input, std::ostream& output,
        const std::string& lang
      ) const throw (Generator::ERROR)
      {
        Generator *g = createOutputGenerator(type,output,lang);
        g->getInput() << input.rdbuf();
        g->closeInput();
      }
      virtual TID generateDocumentAndSave(
        GenerationType type,
        std::istream& input, const std::string& name, unsigned filetype,
        const std::string& lang
      ) const throw (Generator::ERROR)
      {
        Generator *g = createSavingGenerator(type,name,filetype,lang);
        g->getInput() << input.rdbuf();
        return g->closeInput();
      }
    };
    Manager *Manager::create(
      const std::string& path, const std::string& pathTemplates,
      const std::string& pathFM, 
      const std::string& corbaNS
    )
    {
      return new ManagerImpl(path,pathTemplates,pathFM,corbaNS);
    }
  }
} // Register
