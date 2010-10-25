
AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) \
              $(BOOST_CPPFLAGS) \
              $(LIBIDN_CFLAGS) \
              $(XML2_CFLAGS) \
              -I$(top_srcdir) \
              -I$(top_srcdir)/src \
              -I$(top_srcdir)/src/register \
              -I$(top_srcdir)/src/corba \
              -I$(top_srcdir)/src/corba/admin \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/util/db \
              -I$(top_srcdir)/util/db/model \
              -I$(top_builddir)/src \
              -I$(top_builddir)/src/corba

sbin_PROGRAMS = fred-msgd

CLEANFILES = MessagesSK.cc MessagesDynSK.cc MessagesSK.h Messages.hh \
             FileManagerSK.cc FileManagerDynSK.cc FileManagerSK.h FileManager.hh
                   
BUILT_SOURCES = MessagesSK.cc FileManagerSK.cc

fred_msgd_SOURCES = fred-msgd.cc \
                    MessagesSK.cc \
                    MessagesDynSK.cc \
                    FileManagerSK.cc \
                    FileManagerDynSK.cc \
                     messages_filemanager.cc messages_filemanager.h \
                    ../nameservice.cc messages.cc messages.h \
                    ../file_manager_client.cc 

fred_msgd_LDFLAGS =   $(POSTGRESQL_LDFLAGS) \
                     $(BOOST_LDFLAGS) \
                     $(AM_LDFLAGS) \
                     $(BOOST_PROGRAM_OPTIONS_LIB) \
                     $(OMNIORB4_LIBS) \
                     $(BOOST_DATE_TIME_LIB) \
                     $(BOOST_REGEX_LIB) \
                     $(BOOST_THREAD_LIB)


fred_msgd_LDADD =    $(top_builddir)/src/register/messages/libmessages.a \
                     $(top_builddir)/src/model/libmodel.a \
                     $(top_builddir)/util/db/libdb.a \
                     $(top_builddir)/util/log/liblog.a \
                     $(top_builddir)/util/types/libtypes.a \
                     $(top_builddir)/src/old_utils/liboldutils.a
                      
MessagesSK.cc: $(IDLDIR)/Messages.idl
	omniidl -bcxx -Wba -Wbinline $(IDLDIR)/Messages.idl

FileManagerSK.cc: $(IDLDIR)/FileManager.idl $(IDLDIR)/_FileManager.idl $(IDLDIR)/_dataTypes.idl
	omniidl -bcxx -Wba -Wbinline $(IDLDIR)/FileManager.idl
                     
                     
                     


