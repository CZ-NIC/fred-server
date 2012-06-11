#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([fred-server], [2.11.2])
AC_CONFIG_SRCDIR(src/corba/fred-rifd.cc)
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

AM_INIT_AUTOMAKE([subdir-objects])
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
AC_CHECK_LIB([pthread], [pthread_exit])

# Check for omniORB (note that 4.1.1 is bugged for us)
PKG_CHECK_MODULES(OMNIORB4, [omniORB4 > 4.1.1, omniDynamic4, omnithread3])
# check for libxml2
PKG_CHECK_MODULES(XML2, [libxml-2.0 >= 2.5.0])

AX_BOOST_BASE
AX_BOOST_DATE_TIME
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_REGEX

#libcurl.a
#checking for some function with C linkage (viz readelf -a /usr/lib/libcurl | less)
#in ubuntu karmic libcurl4-openssl-dev file /usr/lib/libcurl.a
AC_CHECK_LIB([curl], [curl_msnprintf],,[AC_MSG_ERROR([Library curl not found])])
#lib curl headers check
AC_CHECK_HEADER([curl/curl.h],,[AC_MSG_ERROR([Header curl/curl.h not found])])

# For logger class when log type is console or custom file
AX_BOOST_THREAD 
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
AC_FUNC_MALLOC
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

#expand paths for bindir, sbindir and datarootdir variable
AS_AC_EXPAND([BINDIR], [$bindir])
AS_AC_EXPAND([DATAROOTDIR], [$datarootdir])
AS_AC_EXPAND([SBINDIR], [$sbindir])
AS_AC_EXPAND([SYSCONFDIR], [$sysconfdir])
AS_AC_EXPAND([LOCALSTATEDIR], [$localstatedir])
AS_AC_EXPAND([PREFIX], [$prefix])

#put context name into config.h file
AC_DEFINE_UNQUOTED([CONFIG_FILE], [["]$SYSCONFDIR/fred/server.conf["]], [location of fred config file])

# config file for hpmail (postservis)
AC_DEFINE_UNQUOTED([HPMAIL_CONFIG], [["]$SYSCONFDIR/fred/hpmail.conf["]], [location of hpmail config file])

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

# libmpdec config v2.2
AM_PROG_CC_C_O
AC_ARG_VAR(MACHINE, [force configuration: x64, uint128, ansi64, ppro, ansi32, ansi-legacy])
# System and machine type:
AC_MSG_CHECKING(system as reported by uname -s)
ac_sys_system=`uname -s`
AC_MSG_RESULT($ac_sys_system)

AC_MSG_CHECKING(machine type as reported by uname -m)
ac_sys_machine=`uname -m`
AC_MSG_RESULT($ac_sys_machine)

# Checks for header files:
AC_HEADER_STDC
AC_CHECK_HEADERS(inttypes.h stdint.h)

# Type availability checks:
AC_TYPE_SIZE_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_TYPE(__uint128_t, AC_DEFINE(HAVE_UINT128_T, 1,
              [Define if your compiler provides __uint128_t.]),,)

# Sizes of various types:
AC_CHECK_SIZEOF(size_t, 4)
AC_CHECK_SIZEOF(__uint128_t, 8)

# x64 with gcc asm:
AC_MSG_CHECKING(for x64 gcc inline assembler)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
  __asm__ __volatile__ ("movq %rcx, %rax");
]])],[have_gcc_asm_for_x64=yes],[have_gcc_asm_for_x64=no])
AC_MSG_RESULT($have_gcc_asm_for_x64)
if test "$have_gcc_asm_for_x64" = yes; then
    AC_DEFINE(HAVE_GCC_ASM_FOR_X64, 1,
    [Define if we can use x64 gcc inline assembler.])
fi

# x87 with gcc asm:
AC_MSG_CHECKING(for x87 gcc inline assembler)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[
  unsigned short cw;
  __asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
  __asm__ __volatile__ ("fldcw %0" : : "m" (cw));
]])],[have_gcc_asm_for_x87=yes],[have_gcc_asm_for_x87=no])
AC_MSG_RESULT($have_gcc_asm_for_x87)
if test "$have_gcc_asm_for_x87" = yes; then
    AC_DEFINE(HAVE_GCC_ASM_FOR_X87, 1,
    [Define if we can use x87 gcc inline assembler.])
fi

# _FORTIFY_SOURCE wrappers for memmove and bcopy are incorrect:
# http://sourceware.org/ml/libc-alpha/2010-12/msg00009.html
AC_MSG_CHECKING(for glibc _FORTIFY_SOURCE/memmove bug)
saved_cflags="$CFLAGS"
CFLAGS="-O2 -D_FORTIFY_SOURCE=2"
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void foo(void *p, void *q) { memmove(p, q, 19); }
int main() {
  char a[32] = "123456789000000000";
  foo(&a[9], a);
  if (strcmp(a, "123456789123456789000000000") != 0)
    return 1;
  foo(a, &a[9]);
  if (strcmp(a, "123456789000000000") != 0)
    return 1;
  return 0;
}
]])],
[have_glibc_memmove_bug=no],
[have_glibc_memmove_bug=yes],
[have_glibc_memmove_bug=undefined])
CFLAGS="$saved_cflags"
AC_MSG_RESULT($have_glibc_memmove_bug)
if test "$have_glibc_memmove_bug" = yes; then
    AC_DEFINE(HAVE_GLIBC_MEMMOVE_BUG, 1,
    [Define if glibc has incorrect _FORTIFY_SOURCE wrappers
     for memmove and bcopy.])
fi


# suncc is dectected as cc:
case $ac_sys_system in
sun*|Sun*)
    case $CC in
        cc)
            CC=suncc
            ;;
    esac
esac

# Compiler dependent settings:
AR=ar
MPD_WARN="-Wall"
MPD_OPT="-O2 -fpic"
MPD_PGEN=
MPD_PUSE=
MPD_CCOV=
MPD_LDCOV=
case $CC in
    *gcc*)
        MPD_WARN="-Wall -W -Wno-unknown-pragmas"
        MPD_OPT="-O2 -fpic -s"
        MPD_PGEN="-fprofile-generate -fprofile-values"
        MPD_PUSE="-fprofile-use -freorder-blocks"
        MPD_CCOV="-DTEST_COVERAGE -O0 -g -fno-inline -fprofile-arcs -ftest-coverage -fpic"
        MPD_LDCOV="-fprofile-arcs"
        ;;
    *icc*)
        AR=xiar
        MPD_WARN="-Wall -Wno-unknown-pragmas"
        MPD_OPT="-O2 -fpic -s"
        MPD_PGEN="-wd11505 -prof-gen"
        MPD_PUSE="-wd11505 -prof-use"
        ;;
    *clang*)
        MPD_WARN="-Wall -W -Wno-unknown-pragmas"
        MPD_OPT="-O2 -fpic"
        ;;
    *suncc*)
        MPD_WARN=
        MPD_OPT="-O2 -fpic -s"
        ;;
esac

# Values for the MPD_HEADER_CONFIG variable:
CONFIG_64="
#ifdef CONFIG_32
  #error \"cannot use CONFIG_32 with 64-bit header.\"
#endif

#ifndef CONFIG_64
  #define CONFIG_64
#endif"

CONFIG_32="
#ifdef CONFIG_64
  #error \"cannot use CONFIG_64 with 32-bit header.\"
#endif

#ifndef CONFIG_32
  #define CONFIG_32
#endif"

CONFIG_UNIVERSAL="
#if defined(CONFIG_64) || defined(CONFIG_32)
  #error \"cannot use CONFIG_64 or CONFIG_32 with universal header.\"
#endif

#if defined(__ppc__)
  #define CONFIG_32
  #define ANSI
#elif defined(__ppc64__)
  #define CONFIG_64
  #define ANSI
#elif defined(__i386__)
  #define CONFIG_32
  #define ANSI
#elif defined(__x86_64__)
  #define CONFIG_64
  #define ASM
#else
  #error \"unknown architecture for universal build.\"
#endif"

# Auto-detect machine dependent settings:
M64=
M32=
if test -n "$MACHINE"; then
    M64="-m64 "
    M32="-m32 "
    case "$MACHINE" in
        x64|uint128|ansi64|full_coverage|ppro|ansi32|ansi-legacy|universal)
            : ;;
        *)
            echo "configure: error: invalid MACHINE variable: $MACHINE"
            exit 1 ;;
    esac
elif test $ac_cv_sizeof_size_t -eq 8; then
    if test $have_gcc_asm_for_x64 = yes; then
        MACHINE="x64"
    elif test $ac_cv_sizeof__uint128_t -eq 8; then
        MACHINE="uint128"
    else
        MACHINE="ansi64"
    fi
else
    MACHINE="ansi32"
    if test $have_gcc_asm_for_x87 = yes; then
        case $CC in
            *gcc*|*clang*) # icc >= 11.0 works as well
                case $ac_sys_system in
                    darwin*|Darwin*)
                    ;;
                *)
                    MACHINE="ppro"
                    ;;
                esac
            ;;
        esac
    fi
fi

# Set configuration variables:
MPD_PREC=9
MPD_DPREC=18
CONFIGURE_LDFLAGS=
case "$MACHINE" in
    x64)
        MPD_HEADER_CONFIG=$CONFIG_64
        MPD_CONFIG=$M64"-DCONFIG_64 -DASM"
        CONFIGURE_LDFLAGS=$M64
        MPD_PREC=19
        MPD_DPREC=38
        ;;
    uint128)
        MPD_HEADER_CONFIG=$CONFIG_64
        MPD_CONFIG=$M64"-DCONFIG_64 -DANSI -DHAVE_UINT128_T"
        CONFIGURE_LDFLAGS=$M64
        MPD_PREC=19
        MPD_DPREC=38
        ;;
    ansi64)
        MPD_HEADER_CONFIG=$CONFIG_64
        MPD_CONFIG=$M64"-DCONFIG_64 -DANSI"
        CONFIGURE_LDFLAGS=$M64
        MPD_PREC=19
        MPD_DPREC=38
        ;;
    full_coverage)
        # Formerly ansi64c32, for testing only!
        MPD_HEADER_CONFIG=$CONFIG_FULLCOV
        MPD_CONFIG=$M64"-DCONFIG_32 -DANSI"
        CONFIGURE_LDFLAGS=$M64
        ;;
    ppro)
        MPD_HEADER_CONFIG=$CONFIG_32
        MPD_CONFIG=$M32"-DCONFIG_32 -DPPRO -DASM"
        CONFIGURE_LDFLAGS=$M32
        # Some versions of gcc miscompile inline asm:
        # http://gcc.gnu.org/bugzilla/show_bug.cgi?id=46491
        # http://gcc.gnu.org/ml/gcc/2010-11/msg00366.html
        case $CC in
            *gcc*)
                AC_MSG_CHECKING(for gcc ipa-pure-const bug)
                saved_cflags="$CFLAGS"
                CFLAGS="-O2"
                AC_RUN_IFELSE([AC_LANG_SOURCE([[
                __attribute__((noinline)) int
                foo(int *p) {
                  int r;
                  asm ( "movl \$6, (%1)\n\t"
                        "xorl %0, %0\n\t"
                        : "=r" (r) : "r" (p) : "memory"
                  );
                  return r;
                }
                int main() {
                  int p = 8;
                  if ((foo(&p) ? : p) != 6)
                    return 1;
                  return 0;
                }
                ]])],
                [have_ipa_pure_const_bug=no],
                [have_ipa_pure_const_bug=yes],
                [have_ipa_pure_const_bug=undefined])
                CFLAGS="$saved_cflags"
                AC_MSG_RESULT($have_ipa_pure_const_bug)
                if test "$have_ipa_pure_const_bug" = yes; then
                    MPD_CONFIG="$MPD_CONFIG -fno-ipa-pure-const"
                    AC_DEFINE(HAVE_IPA_PURE_CONST_BUG, 1,
                [Define if gcc has the ipa-pure-const bug.])
                fi
            ;;
        esac
        ;;
    ansi32)
        MPD_HEADER_CONFIG=$CONFIG_32
        MPD_CONFIG=$M32"-DCONFIG_32 -DANSI"
        CONFIGURE_LDFLAGS=$M32
        ;;
    ansi-legacy)
        MPD_HEADER_CONFIG=$CONFIG_32
        MPD_CONFIG=$M32"-DCONFIG_32 -DANSI -DLEGACY_COMPILER"
        CONFIGURE_LDFLAGS=$M32
        ;;
    universal)
        MPD_HEADER_CONFIG=$CONFIG_UNIVERSAL
        MPD_CONFIG="-DUNIVERSAL"
esac


# Substitute variables and generate output:
AC_SUBST(AR)
AC_SUBST(MPD_HEADER)
AC_SUBST(MPD_WARN)
AC_SUBST(MPD_CONFIG)
AC_SUBST(MPD_HEADER_CONFIG)
AC_SUBST(MPD_OPT)

AC_SUBST(MPD_CCOV)
AC_SUBST(MPD_LDCOV)
AC_SUBST(MPD_PGEN)
AC_SUBST(MPD_PUSE)
AC_SUBST(MPD_PREC)
AC_SUBST(MPD_DPREC)

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
                 fred-server.spec
                 config/server.conf
                 config/hpmail.conf
                 init-scripts/fred-server
                 util/decimal/mpdecimal-2.2/mpdecimal.h
                 ])
AC_OUTPUT
