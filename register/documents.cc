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
      const std::string& xslTemplate; ///< xsl template for xslt transform
      std::ostream* out; ///< output stream if filename is empty
      std::string filename; ///< filename for storage
      TmpFile bufferFile; ///< simulate streaming (remove?)
      std::stringstream outBuffer; ///< for id generation 
     public:
      /// initialize generator with streams, path, template and filename
      GeneratorImpl(
        const std::string& _path, const std::string& _xslTemplate,
        std::ostream* _out, const std::string& _filename
      ) throw (TmpFile::NAME_ERROR,TmpFile::OPEN_ERROR)
        : path(_path), xslTemplate(_xslTemplate), out(NULL),
          filename(_filename)
      {
        bufferFile.open(std::ios::out);
        out = _out ? _out : &outBuffer; 
      }
      /// generate document
      TID generate()
      {
        TmpFile outputFile;
        std::stringstream cmd;
        cmd << "xsltproc "
            << path << "/templates/" << xslTemplate << " "
            << bufferFile.getName()
            << " | " << path << "/doc2pdf.py > "
            << outputFile.getName();
        system(cmd.str().c_str());
        outputFile.open(std::ios::in);
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
      typedef std::map<GenerationType, std::string> TemplateMapType;
      TemplateMapType templateMap;
     public:
      ManagerImpl(const std::string& _path) : path(_path)
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
        return new GeneratorImpl(path,i->second,&output,"");
      }
      virtual Generator *createSavingGenerator(
        GenerationType type, const std::string& filename
      ) const throw (Generator::ERROR)
      {
        TemplateMapType::const_iterator i = templateMap.find(type);
        if (i == templateMap.end()) throw Generator::ERROR();
        return new GeneratorImpl(path,i->second,NULL,filename);
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
        std::istream& input, const std::string& name
      ) const throw (Generator::ERROR)
      {
        Generator *g = createSavingGenerator(type,name);
        g->getInput() << input.rdbuf();
        return g->closeInput();
      }
    };
    Manager *Manager::create(const std::string& path)
    {
      return new ManagerImpl(path);
    }
  }
} // Register
