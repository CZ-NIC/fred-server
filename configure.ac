#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([fred-server], 1.7)
AC_CONFIG_SRCDIR(ccReg_server.cc)
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

PKG_PROG_PKG_CONFIG

# Checks for libraries.

AC_CHECK_LIB([pthread], [pthread_exit])

PKG_CHECK_MODULES(OMNIORB4, [omniORB4, omniDynamic4, omnithread3])

AX_BOOST_BASE
AX_BOOST_DATE_TIME
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_REGEX

AX_LIB_POSTGRESQL(8.0)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(
	[arpa/inet.h stdlib.h string.h sys/socket.h \
	sys/time.h syslog.h unistd.h]
)

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

AC_ARG_WITH(
	[idldir], 
	[AS_HELP_STRING(
		[--with-idldir],
		[location of idl directory containing IDL interfaces [idl]]
	)],
	[AC_SUBST([IDLDIR], [$withval])],
	[AC_SUBST([IDLDIR], [/usr/share/idl/fred/])]
)

AC_CHECK_FILE(
	[$IDLDIR/ccReg.idl], 
	[], 
	[AC_MSG_ERROR(
		[directory "$IDLDIR" does not contain ccReg.idl 
		(use --with-idldir)]
	)]
)

AC_CONFIG_FILES([Makefile
                 register/Makefile
		 contrib/fedora/fred-server.spec])
AC_OUTPUT
