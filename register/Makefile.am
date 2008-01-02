noinst_LIBRARIES = libccreg.a

libccreg_a_SOURCES = register.cc zone.cc blacklist.cc domain.cc registrar.cc \
  object_impl.cc contact.cc nsset.cc auth_info.cc invoice.cc \
  documents.cc bank.cc info_buffer.cc poll.cc common_impl.cc \
  notify.cc \
  auth_info.h common_object.h epp_action.h mailer.h \
  object_impl.h sql.h bank.h contact.h exceptions.h \
  notify.h poll.h types.h blacklist.h documents.h \
  info_buffer.h nsset.h register.h zone.h common_impl.h \
  domain.h invoice.h object.h registrar.h

AM_CPPFLAGS = $(POSTGRESQL_CFLAGS) $(BOOST_CFLAGS) -I$(srcdir)/..
