# Process this file with autoconf to produce a configure script.

AC_INIT(gbzadmin3, 2.4.2)
AM_INIT_AUTOMAKE()
AC_CONFIG_HEADERS(config.h)
AM_MAINTAINER_MODE

# prevent setting xFLAGS to default of -g -O2
if test x"$CFLAGS" = x""; then
  AC_SUBST(CFLAGS, [ ])
fi
if test x"$CXXFLAGS" = x""; then
  AC_SUBST(CXXFLAGS, [ ])
fi

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_HEADER_STDC
AC_LANG(C++)

CONF_DATE=`date -u +%Y%m%d`
GBZADMIN_DATE=`date -u +%Y-%m-%d`
GBZADMIN_VERSION="2.99"
host=`uname`

pkg_modules="gtkmm-2.4 >= 2.8.0" # gconfmm-2.6"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CPPFLAGS)
AC_SUBST(PACKAGE_LIBS)

PKG_CHECK_MODULES(GLIBMMDEPS, glibmm-2.4)
AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

GETTEXT_PACKAGE=gbzadmin3
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

AC_ARG_ENABLE([debug],
              AS_HELP_STRING([--enable-debug], [compile with debug symbols @<:@default=no@:>@]),
              [want_debug="$enableval"], [want_debug=no])

if test "x$want_debug" = "xyes" -a $ac_cv_c_compiler_gnu != no; then
  CFLAGS="$CFLAGS -O0 -ggdb"
  CXXFLAGS="$CXXFLAGS -O0 -ggdb"
  AC_DEFINE([DEBUG], 1, [Define for debugging])
else
  CFLAGS="$CFLAGS -O2"
  CXXFLAGS="$CXXFLAGS -O2"
fi

# Add the languages which your application supports here.
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

AC_LANG(C)
AC_CHECK_HEADERS(curl/curl.h)
AC_CHECK_LIB(curl,curl_global_init)
LIBS=`curl-config --libs`

CFLAGS="$CFLAGS -Wall"
CXXFLAGS="$CXXFLAGS -Wall -std=gnu++11"

AC_MSG_CHECKING([for asynaptic programmer])
AC_MSG_RESULT([found])

AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])

AC_MSG_RESULT([Done.])
AC_MSG_RESULT([])

echo "
Configuration for $PACKAGE $VERSION :
--------------------------------

 Host System Type .....: $host
 Install path .........: $prefix
 Data Directory........: $datarootdir
 GTKmm version ........: `pkg-config gtkmm-2.4 --modversion`
 Preprocessor Flags ...: $CXXFLAGS
 Compiler Flags .......: $CFLAGS $PACKAGE_CFLAGS
 Linker Flags .........: $LDFLAGS $LIBS $PACKAGE_LIBS

 Now type 'make' to build $PACKAGE $VERSION,
 and then 'make install' for installation.
"

