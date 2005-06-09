AC_INIT
AC_CONFIG_SRCDIR([src/gtkmm/main.cpp])
AM_INIT_AUTOMAKE(lostirc, 0.4.7pre)
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_LIBTOOL
PKG_CHECK_MODULES(GLIBMMDEPS, glibmm-2.4)
AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)
PKG_CHECK_MODULES(GTKMMDEPS, [gtkmm-2.4 gdkmm-2.4 atkmm-1.6 pangomm-1.4])
AC_SUBST(GTKMMDEPS_CFLAGS)
AC_SUBST(GTKMMDEPS_LIBS)

AM_GNU_GETTEXT_VERSION(0.14.1)
AM_GNU_GETTEXT

CXXFLAGS="$CXXFLAGS -Wall"

# Add ./configure --enable-logdebug
AC_ARG_ENABLE(logdebug,
        [  --enable-logdebug       compile with log-debugging to a file],
        logdebug=$enable_logdebug,
        logdebug=no
)
AC_MSG_CHECKING(whether to log debug info to a file)
if test "x$logdebug" = xyes; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(DEBUG, [], whether to log debug info to a file)
else
        AC_MSG_RESULT(no)
fi

# Add ./configure --enable-desktopfile
AC_ARG_ENABLE(desktopfile,
        [  --disable-desktopfile   don't install .desktop/menu entry],
        desktopfile=$enable_desktopfile,
        desktopfile=yes
)
AC_MSG_CHECKING(whether to install .desktop/menu entry)
if test "x$desktopfile" = xyes; then
        AC_MSG_RESULT(yes)
        AM_CONDITIONAL(DESKTOPFILE, true)
else
        AC_MSG_RESULT(no)
        AM_CONDITIONAL(DESKTOPFILE, false)
fi

dnl Used for enabling the "-no-undefined" flag while generating DLLs
dnl Copied from the official gtk+-2 configure.in
AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

AC_CONFIG_FILES([lostirc.spec Makefile src/Makefile src/libirc/Makefile src/gtkmm/Makefile po/Makefile.in intl/Makefile ])
AC_OUTPUT
