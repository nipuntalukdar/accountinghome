#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([accountinghome], [1.0], [nipun.talukdar@gmail.com])
AC_CONFIG_SRCDIR([command_parse.hpp])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([pcre], [pcre_compile],,[AC_MSG_ERROR(Library libpcre is not found) ])
AC_CHECK_LIB([db_cxx], [main],,[AC_MSG_ERROR(Library libdb_cxx is not found) ])


# Checks for header files.
AC_CHECK_HEADERS(iostream, , [AC_MSG_ERROR(iostream header missing)])
AC_CHECK_HEADERS(pcre.h, , [AC_MSG_ERROR(pcre.h header missing)])
AC_CHECK_HEADERS(db.h, , [AC_MSG_ERROR(db.h header missing)])
AC_CHECK_HEADERS(db_cxx.h, , [AC_MSG_ERROR(db_cxx.h header missing)])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
