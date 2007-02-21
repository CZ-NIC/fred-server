#include "documents.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <map>

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
        name = (char *)malloc(strlen(NAME_TEMPLATE)+1);
        strcpy(name,NAME_TEMPLATE);
        if (mkstemp(name) < 0) {
          free(name);
          throw NAME_ERROR();
        }
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
    /// implementation of Generator interface
    class GeneratorImpl : public Generator
    {
      const std::string& path; ///< path to pdf generator
      const std::string& pathFM; ///< path to filemanager client
      const std::string& corbaNS; ///< host with corba nameservice
      const std::string& xslTemplate; ///< xsl template for xslt transform
      std::ostream* out; ///< output stream if filename is empty
      std::string filename; ///< filename for storage
      unsigned filetype; //< type of generated document from filemanager
      TmpFile bufferFile; ///< simulate streaming (remove?)
      std::stringstream outBuffer; ///< for id generation
      std::string lang; ///< code of language to use;
     public:
      /// initialize generator with streams, path, template and filename
      GeneratorImpl(
        const std::string& _path, 
        const std::string& _pathFM, const std::string& _corbaNS, 
        const std::string& _xslTemplate,
        std::ostream* _out, const std::string& _filename, unsigned _filetype,
        const std::string& _lang
      ) throw (TmpFile::NAME_ERROR,TmpFile::OPEN_ERROR)
        : path(_path), pathFM(_pathFM), corbaNS(_corbaNS),
          xslTemplate(_xslTemplate), out(NULL),
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
        cmd << "xsltproc  "
            << "--stringparam srcpath " << path << "/templates/" << " ";
        if (!lang.empty())
          cmd << "--stringparam lang " << lang << " ";
        cmd << path << "/templates/" << xslTemplate << " "
            << bufferFile.getName()
            << " | " << path << "/doc2pdf.py ";
        // in case of generation into corba filesystem send result to
        // corba filesystem client
        if (!filename.empty()) {
          cmd << "| python " <<  pathFM 
              << "/filemanager_client.py -m application/pdf -s "
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
      std::string path;
      std::string pathFM;
      typedef std::map<GenerationType, std::string> TemplateMapType;
      TemplateMapType templateMap;
      std::string corbaNS;
     public:
      ManagerImpl(
        const std::string& _path, const std::string& _pathFM,
        const std::string& _corbaNS 
      ) : path(_path), pathFM(_pathFM), corbaNS(_corbaNS)
      {
        templateMap[GT_INVOICE_PDF] = "invoice.xsl";
        templateMap[GT_ADVANCE_INVOICE_PDF] = "advance_invoice.xsl";
        templateMap[GT_AUTHINFO_REQUEST_PDF] = "auth_info.xsl";
      }
      
      virtual Generator *createOutputGenerator(
        GenerationType type, std::ostream& output,
        const std::string& lang
      ) const throw (Generator::ERROR)
      {
        TemplateMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return new GeneratorImpl(
          path,pathFM,corbaNS,i->second,&output,"",0,lang
        );
      }
      virtual Generator *createSavingGenerator(
        GenerationType type, 
        const std::string& filename, unsigned filetype,
        const std::string& lang         
      ) const throw (Generator::ERROR)
      {
        TemplateMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return new GeneratorImpl(
          path,pathFM,corbaNS,i->second,NULL,filename,filetype,lang
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
      const std::string& path, const std::string& pathFM, 
      const std::string& corbaNS
    )
    {
      return new ManagerImpl(path,pathFM,corbaNS);
    }
  }
} // Register
