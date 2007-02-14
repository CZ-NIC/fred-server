#include "documents.h"
#include <stdio.h>
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
      char *name; ///< unique name in tmp directory
     public:
      class OPEN_ERROR {}; ///< error in file opening
      class NAME_ERROR {}; ///< error in name generation
      /// initialize unique name 
      TmpFile() throw (OPEN_ERROR)
        : name(tempnam(NULL,"fred-docgen-"))
      {
        if (!name) throw NAME_ERROR();
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
    class GeneratorImpl : public Generator
    {
      const std::string& path; ///< path to pdf generator
      const std::string& pathFM; ///< path to filemanager client
      const std::string& xslTemplate; ///< xsl template for xslt transform
      std::ostream* out; ///< output stream if filename is empty
      std::string filename; ///< filename for storage
      unsigned filetype; //< type of generated document from filemanager
      TmpFile bufferFile; ///< simulate streaming (remove?)
      std::stringstream outBuffer; ///< for id generation
     public:
      /// initialize generator with streams, path, template and filename
      GeneratorImpl(
        const std::string& _path, const std::string& _pathFM, 
        const std::string& _xslTemplate,
        std::ostream* _out, const std::string& _filename, unsigned _filetype
      ) throw (TmpFile::NAME_ERROR,TmpFile::OPEN_ERROR)
        : path(_path), pathFM(_pathFM), xslTemplate(_xslTemplate), out(NULL),
          filename(_filename), filetype(_filetype)
      {
        bufferFile.open(std::ios::out);
        out = _out ? _out : &outBuffer; 
      }
      /// generate document
      TID generate() throw (Generator::ERROR)
      {
        TmpFile outputFile;
        std::stringstream cmd;
        cmd << "xsltproc "
            << path << "/templates/" << xslTemplate << " "
            << bufferFile.getName()
            << " | " << path << "/doc2pdf.py ";
        // in case of generation into corba filesystem send result to
        // corba filesystem client
        if (!filename.empty()) 
          cmd << "| python " <<  pathFM 
              << "/filemanager_client.py -m application/pdf "
              << " -n curlew " // TODO: change     
              << " -l " << filename
              << " -t " << filetype;     
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
    class ManagerImpl : public Manager
    {
      std::string path;
      std::string pathFM;
      typedef std::map<GenerationType, std::string> TemplateMapType;
      TemplateMapType templateMap;
     public:
      ManagerImpl(const std::string& _path, const std::string& _pathFM) :
        path(_path), pathFM(_pathFM)
      {
        templateMap[GT_INVOICE_PDF] = "invoice.xsl";
        templateMap[GT_ADVANCE_INVOICE_PDF] = "advance_invoice.xsl";
      }
      
      virtual Generator *createOutputGenerator(
        GenerationType type, std::ostream& output
      ) const throw (Generator::ERROR)
      {
        TemplateMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return new GeneratorImpl(path,pathFM,i->second,&output,"",0);
      }
      virtual Generator *createSavingGenerator(
        GenerationType type, const std::string& filename, unsigned filetype 
      ) const throw (Generator::ERROR)
      {
        TemplateMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return new GeneratorImpl(path,pathFM,i->second,NULL,filename,filetype);
      }
      virtual void generateDocument(
        GenerationType type, 
        std::istream& input, std::ostream& output
      ) const throw (Generator::ERROR)
      {
        Generator *g = createOutputGenerator(type,output);
        g->getInput() << input.rdbuf();
        g->closeInput();
      }
      virtual TID generateDocumentAndSave(
        GenerationType type,
        std::istream& input, const std::string& name, unsigned filetype
      ) const throw (Generator::ERROR)
      {
        Generator *g = createSavingGenerator(type,name,filetype);
        g->getInput() << input.rdbuf();
        return g->closeInput();
      }
    };
    Manager *Manager::create(
      const std::string& path, const std::string& pathFM
    )
    {
      return new ManagerImpl(path,pathFM);
    }
  }
} // Register
