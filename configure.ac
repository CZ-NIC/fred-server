#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([fred-server], [2.37.1])
AC_CONFIG_SRCDIR(src/bin/corba/fred-rifd.cc)
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# doxygen
DX_HTML_FEATURE(OFF)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(fred-server, doxy.conf, [doc])

AM_INIT_AUTOMAKE([subdir-objects tar-ustar filename-length-max=220])
m4_pattern_allow([^AC_MSG_])

AC_DEFUN([AS_AC_TEST_CORBA_PORTS],
[
 AC_MSG_CHECKING([whether CORBA ports are unique])
 if test $PIFDPORT -eq $RIFDPORT -o $PIFDPORT -eq $ADIFDPORT -o $RIFDPORT -eq $ADIFDPORT -o $LOGDPORT -eq $PIFDPORT -o $LOGDPORT -eq $RIFDPORT -o $LOGDPORT -eq $ADIFDPORT; then
	 AC_MSG_RESULT([no])
	 AC_MSG_ERROR([each of CORBA port must be unique])
 else
	 AC_MSG_RESULT([yes])
 fi
])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB


PKG_PROG_PKG_CONFIG


# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_exit], [],
[
    AC_MSG_ERROR([library pthread not found])
])

AC_CHECK_LIB([mpdec], [mpd_free], [],
[
    AC_MSG_ERROR([library mpdec not found])
])

AC_CHECK_LIB([ssh], [ssh_buffer_free], [],
[
    AC_MSG_ERROR([library ssh not found])
])

AC_CHECK_HEADER([libssh/libssh.h],,[AC_MSG_ERROR([Header libssh/libssh.h not found])])

AC_CHECK_LIB([z], [get_crc_table], [],
[
    AC_MSG_ERROR([library zlib not found])
])

AC_CHECK_HEADER([zlib.h],,[AC_MSG_ERROR([Header zlib.h not found])])

AC_CHECK_LIB([minizip], [rand], [],
[
    AC_MSG_ERROR([library minizip not found])
])

AC_CHECK_HEADER([minizip/zip.h],,[AC_MSG_ERROR([Header minizip/zip.h not found])])

AX_CHECK_COMPILE_FLAG([-std=c++14], [
                      CXXFLAGS="$CXXFLAGS -std=c++14"])

# check for libcrypto
PKG_CHECK_MODULES(LIBCRYPTO, [libcrypto])

AC_CHECK_HEADER([openssl/evp.h],,[AC_MSG_ERROR([Header openssl/evp.h not found])])

# Check for omniORB (note that 4.1.1 is bugged for us)
PKG_CHECK_MODULES(OMNIORB4, [omniORB4 > 4.1.1, omniDynamic4, omnithread3])
# check for libxml2
PKG_CHECK_MODULES(XML2, [libxml-2.0 >= 2.5.0])

AX_BOOST_BASE
AX_BOOST_DATE_TIME
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_REGEX
AX_BOOST_FILESYSTEM
AX_BOOST_RANDOM

#libcurl.a
#checking for some function with C linkage (viz readelf -a /usr/lib/libcurl | less)
#in ubuntu karmic libcurl4-openssl-dev file /usr/lib/libcurl.a
AC_CHECK_LIB([curl], [curl_msnprintf],,[AC_MSG_ERROR([Library curl not found])])
#lib curl headers check
AC_CHECK_HEADER([curl/curl.h],,[AC_MSG_ERROR([Header curl/curl.h not found])])

# For logger class when log type is console or custom file
AX_BOOST_THREAD 
AX_BOOST_SYSTEM
# For filter serialization to xml
AX_BOOST_SERIALIZATION

# For unittests
AX_BOOST_UNIT_TEST_FRAMEWORK

# Checks for PostgreSQL headers and libs
AX_LIB_POSTGRESQL(8.0)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(
	[arpa/inet.h stdlib.h string.h sys/socket.h \
	sys/time.h syslog.h unistd.h]
)

AC_CHECK_HEADER([boost/test/included/unit_test.hpp],,[AC_MSG_ERROR([Header boost/test/included/unit_test.hpp not found])])
AC_CHECK_HEADER([omniORB4/CORBA.h],,[AC_MSG_ERROR([Header omniORB4/CORBA.h not found])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
#AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday memset strcasecmp strncasecmp strspn])

# fred database host
AC_ARG_WITH([dbhost], [AS_HELP_STRING([--with-dbhost],
			[Database host name [localhost]])],
			[AC_SUBST([DBHOST], [$withval])],
			[AC_SUBST([DBHOST], [localhost])])

# fred database port
AC_ARG_WITH([dbport], [AS_HELP_STRING([--with-dbport],
			[Database port number [5432]])],
			[AC_SUBST([DBPORT], [$withval])],
			[AC_SUBST([DBPORT], [5432])])

# fred database name
AC_ARG_WITH([dbname], [AS_HELP_STRING([--with-dbname],
			[Database name [fred]])],
			[AC_SUBST([DBNAME], [$withval])],
			[AC_SUBST([DBNAME], [fred])])

# fred database user name
AC_ARG_WITH([dbuser], [AS_HELP_STRING([--with-dbuser],
			[Database user name [fred]])],
			[AC_SUBST([DBUSER], [$withval])],
			[AC_SUBST([DBUSER], [fred])])

# fred database password
AC_ARG_WITH([dbpass], [AS_HELP_STRING([--with-dbpass],
			[Database access password [password]])],
			[AC_SUBST([DBPASS], [$withval])],
			[AC_SUBST([DBPASS], [password])])

# corba nameservice host
AC_ARG_WITH([nshost], [AS_HELP_STRING([--with-nshost],
			[CORBA nameservice host name [localhost]])],
			[AC_SUBST([NSHOST], [$withval])],
			[AC_SUBST([NSHOST], [localhost])])
			
AC_ARG_WITH([monitoring_hosts_file], [AS_HELP_STRING([--with-monitoring-hosts-file],
			[File containing list of monitoring machines [$SYSCONFDIR/fred/monitoring_hosts.conf]])],
			[AC_SUBST([MONITORINGIPS], [$withval])],
			[AC_SUBST([MONITORINGIPS], [/usr/local/etc/fred/monitoring_hosts.conf])])

# corba nameservice port
AC_ARG_WITH([nsport], [AS_HELP_STRING([--with-nsport],
			[CORBA nameservice port [2809]])],
			[AC_SUBST([NSPORT], [$withval])],
			[AC_SUBST([NSPORT], [2809])])

# corba nameservice context
AC_ARG_WITH([nscontext], [AS_HELP_STRING([--with-nscontext],
			[CORBA nameservice context name [fred]])],
			[AC_SUBST([NSCONTEXT], [$withval])],
			[AC_SUBST([NSCONTEXT], [fred])])
#put context name into config.h file
AC_DEFINE_UNQUOTED([CONTEXT_NAME], [["]$NSCONTEXT["]], [name of CORBA Nameservice context])

# corba rifd port
AC_ARG_WITH([rifdport], [AS_HELP_STRING([--with-rifdport],
			[CORBA fred-rifd port [2222]])],
			[AC_SUBST([RIFDPORT], [$withval])],
			[AC_SUBST([RIFDPORT], [2222])])
#pifd port
AC_ARG_WITH([pifdport], [AS_HELP_STRING([--with-pifdport],
			[CORBA fred-pifd port [2223]])],
			[AC_SUBST([PIFDPORT], [$withval])],
			[AC_SUBST([PIFDPORT], [2223])])
#adifd port
AC_ARG_WITH([adifdport], [AS_HELP_STRING([--with-adifdport],
			[CORBA fred-adifd port [2224]])],
			[AC_SUBST([ADIFDPORT], [$withval])],
			[AC_SUBST([ADIFDPORT], [2224])])
#logd port
AC_ARG_WITH([logdport], [AS_HELP_STRING([--with-logdport],
			[CORBA fred-logd port [2226]])],
			[AC_SUBST([LOGDPORT], [$withval])],
			[AC_SUBST([LOGDPORT], [2226])])
#mifd port
AC_ARG_WITH([mifdport], [AS_HELP_STRING([--with-mifdport],
			[CORBA fred-mifd port [2227]])],
			[AC_SUBST([MIFDPORT], [$withval])],
			[AC_SUBST([MIFDPORT], [2227])])
#msgd port
AC_ARG_WITH([msgdport], [AS_HELP_STRING([--with-msgdport],
			[CORBA fred-msgd port [2228]])],
			[AC_SUBST([MSGDPORT], [$withval])],
			[AC_SUBST([MSGDPORT], [2228])])
#dbifd port
AC_ARG_WITH([dbifdport], [AS_HELP_STRING([--with-dbifdport],
			[CORBA fred-dbifd port [2229]])],
			[AC_SUBST([DBIFDPORT], [$withval])],
			[AC_SUBST([DBIFDPORT], [2229])])
#akmd port
AC_ARG_WITH([akmdport], [AS_HELP_STRING([--with-akmdport],
            [CORBA fred-akmd port [2230]])],
            [AC_SUBST([AKMDPORT], [$withval])],
            [AC_SUBST([AKMDPORT], [2230])])
#rsifd port
AC_ARG_WITH([rsifdport], [AS_HELP_STRING([--with-rsifdport],
            [CORBA fred-rsifd port [2231]])],
            [AC_SUBST([RSIFDPORT], [$withval])],
            [AC_SUBST([RSIFDPORT], [2231])])
#accifd port
AC_ARG_WITH([accifdport], [AS_HELP_STRING([--with-accifdport],
            [CORBA fred-accifd port [2232]])],
            [AC_SUBST([ACCIFDPORT], [$withval])],
            [AC_SUBST([ACCIFDPORT], [2232])])


#hp login name
AC_ARG_WITH([hp_login_name], [AS_HELP_STRING([--with-hp_login_name],
			[Postservice login name])],
			[AC_SUBST([HP_LOGIN_NAME], [$withval])],
			[AC_SUBST([HP_LOGIN_NAME], [dreplech])])

#hp login password
AC_ARG_WITH([hp_login_password], [AS_HELP_STRING([--with-hp_login_password],
			[Postservice password])],
			[AC_SUBST([HP_LOGIN_PASSWORD], [$withval])],
			[AC_SUBST([HP_LOGIN_PASSWORD], [dreplech])])

#hp login note
AC_ARG_WITH([hp_login_note], [AS_HELP_STRING([--with-hp_login_note],
			[Postservice batch note - optional])],
			[AC_SUBST([HP_LOGIN_NOTE], [$withval])],
			[AC_SUBST([HP_LOGIN_NOTE], [test_fred])])

AS_AC_TEST_CORBA_PORTS()

# charging registrars disabled configuration
AC_ARG_WITH([epp_operations_charging], [AS_HELP_STRING([--with-epp-operations-charging],
            [Turn on/off EPP operations charging [false]])],
            [AC_SUBST([EPP_OPERATIONS_CHARGING], [$withval])],
            [AC_SUBST([EPP_OPERATIONS_CHARGING], [false])])

#optys mail login name
AC_ARG_WITH([optys_login_name], [AS_HELP_STRING([--with-optys_login_name],
            [Optys mail login name])],
            [AC_SUBST([OPTYS_LOGIN_NAME], [$withval])],
            [AC_SUBST([OPTYS_LOGIN_NAME], [test])])

#optys mail login password
AC_ARG_WITH([optys_login_password], [AS_HELP_STRING([--with-optys_login_password],
            [Optys mail login password])],
            [AC_SUBST([OPTYS_LOGIN_PASSWORD], [$withval])],
            [AC_SUBST([OPTYS_LOGIN_PASSWORD], [test])])

#optys mail login port
AC_ARG_WITH([optys_login_port], [AS_HELP_STRING([--with-optys_login_port],
            [Optys mail login port])],
            [AC_SUBST([OPTYS_LOGIN_PORT], [$withval])],
            [AC_SUBST([OPTYS_LOGIN_PORT], [22])])


#expand paths for bindir, sbindir and datarootdir variable
AS_AC_EXPAND([BINDIR], [$bindir])
AS_AC_EXPAND([DATAROOTDIR], [$datarootdir])
AS_AC_EXPAND([SBINDIR], [$sbindir])
AS_AC_EXPAND([SYSCONFDIR], [$sysconfdir])
AS_AC_EXPAND([LOCALSTATEDIR], [$localstatedir])
AS_AC_EXPAND([PREFIX], [$prefix])

#doxygen preprocessor config
AS_AC_EXPAND([BOOST_INCLUDE_DOXYGEN_PATH],[`echo $BOOST_CPPFLAGS | sed 's/-I//g' | sed 's/-pthread//g'`])

#put context name into config.h file
AC_DEFINE_UNQUOTED([CONFIG_FILE], [["]$SYSCONFDIR/fred/server.conf["]], [location of fred config file])

# config file for hpmail (postservis)
AC_DEFINE_UNQUOTED([HPMAIL_CONFIG], [["]$SYSCONFDIR/fred/hpmail.conf["]], [location of hpmail config file])

# config file for optys mail service
AC_DEFINE_UNQUOTED([OPTYS_CONFIG], [["]$SYSCONFDIR/fred/optysmail.conf["]], [location of optys mail service config file])

# location of data for tests 
AC_DEFINE_UNQUOTED([TEST_DATA], [["]$DATAROOTDIR/["]], [location of data for tests])

AC_DEFINE_UNQUOTED([HAVE_LOGGER], [1], [compile libraries with logging messages])


#location of idl files, when user use this option during configuration
#process, exactly given path will be used, otherwise will be used
#`$datarootdir/idl/fred/', where dataroot dir is `$prefix/share'
#and prefix is by default `/usr/local'. So whole path is
#`/usr/local/share/idl/fred/'
AC_ARG_WITH(
	[idldir],
	[AS_HELP_STRING(
		[--with-idldir],
		[location of idl directory containing IDL interfaces [DATAROOTDIR/idl/fred/]]
	)],
	[AC_SUBST([IDLDIR], [$withval])],
	[AC_SUBST([IDLDIR], [$DATAROOTDIR/idl/fred/])]
)

AC_CHECK_FILE(
	[$IDLDIR/Registry.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Registry.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/Messages.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Messages.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/MojeID.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain MojeID.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/FileManager.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain FileManager.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/Mailer.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Mailer.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/Zone.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Zone.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/_dataTypes.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain _dataTypes.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/Filters.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Filters.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/Whois.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Whois.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/Admin.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Admin.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/Logger.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain Logger.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/TechCheck.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain TechCheck.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/EPP.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain EPP.idl
		(use --with-idldir)]
	)]
)

AC_CHECK_FILE(
	[$IDLDIR/ZoneGenerator.idl],
	[],
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain ZoneGenerator.idl
		(use --with-idldir)]
	)]
)

AC_ARG_WITH(
  [libidn], 
  [AC_HELP_STRING(
    [--with-libidn=[DIR]],
    [Support IDN (needs GNU Libidn)]
  )],
  libidn=$withval, 
  libidn=yes
)
if test "$libidn" != "no" ; then
  if test "$libidn" == "yes"; then
    PKG_CHECK_MODULES(LIBIDN, libidn >= 0.5.18, [plibidn=yes], [plibidn=no])
    AC_SUBST(LIBIDN_CFLAGS)
    AC_SUBST(LIBIDN_LIBS)
  fi
  if test "$plibidn" != "yes" ; then
    if test "$libidn" != "yes"; then
      LDFLAGS="${LDFLAGS} -L$libidn/lib"
      CPPFLAGS="${CPPFLAGS} -I$libidn/include"
    fi
    AC_CHECK_HEADER(
      idna.h,
      [AC_CHECK_LIB(
        idn, stringprep_check_version,
        [libidn=yes LIBS="${LIBS} -lidn"], libidn=no
       )],
      libidn=no
    )
  fi
fi
if test "$libidn" == "no"; then
  AC_MSG_ERROR([Libidn not found])
fi

# libmagic
# AC_CHECK_LIB(magic, magic_open)
# AC_CHECK_HEADERS(magic.h,,[AC_MSG_ERROR([magic.h not found])])

AM_PROG_CC_C_O


if test -z "$CFLAGS"; then
    CONFIGURE_CFLAGS="$MPD_WARN $MPD_CONFIG $MPD_OPT"
else
    CONFIGURE_CFLAGS="$MPD_CONFIG -fpic $CFLAGS"
fi
if test "$have_glibc_memmove_bug" = yes; then
    CONFIGURE_CFLAGS="$CONFIGURE_CFLAGS -U_FORTIFY_SOURCE"
fi

if test -n "$LDFLAGS"; then
    CONFIGURE_LDFLAGS="$LDFLAGS"
fi

AC_SUBST(CONFIGURE_CFLAGS)
AC_SUBST(CONFIGURE_LDFLAGS)

AC_CONFIG_FILES([Makefile
                 contrib/fedora/fred-server.spec
                 config/server.conf
                 config/hpmail.conf
                 config/optysmail.conf
                 init-scripts/fred-server
                 doxy.conf
                 ])
AC_OUTPUT
