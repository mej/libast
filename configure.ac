# $Id$

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE([libast], [0.8])
AC_DEFINE_UNQUOTED(LIBAST_VERSION, "$VERSION", [Version])
AC_CONFIG_HEADERS([config.h include/libast/sysdefs.h])
AC_CONFIG_MACRO_DIR([.])

dnl# Set some basic variables
AUTHORS="Michael Jennings (mej@eterm.org)"
AC_SUBST(AUTHORS)
AC_DEFINE_UNQUOTED(AUTHORS, "$AUTHORS", [Authors])

AC_PROG_CC
AC_PROG_CPP

AST_STD_CHECKS()
AST_PROG_CHECKS()
AST_VAR_CHECKS()
AST_HEADER_CHECKS()
AST_TYPE_CHECKS()
AST_FUNC_CHECKS()
AST_COMPILER_CHECKS()

AC_PROG_LIBTOOL()

AST_ARG_DEBUG(DEBUG)
if test $DEBUG -eq 0 ; then
    AC_WARN(*** Debugging support disabled.  Client programs will ***)
    AC_WARN(*** not have access to any libast debugging routines! ***)
fi

AC_ARG_WITH(pedantry,
    [  --with-pedantry         add -Werror -std=c99 -pedantic to CFLAGS],
    [
        if test "$withval" != "no"; then
            CFLAGS="$CFLAGS -Wall -Werror -std=c99"
            AC_DEFINE(STRICT_ISO_C99, 1, [Defined if strict ISO C99 (9899:1999) is requested or required.])
        fi
    ]
)

AC_MSG_CHECKING(if STRICT_ISO_C99 required)
AC_TRY_RUN([
               int main () {
                   int a = 0, b = 0;
                   (long) a = (long) b;
                   return 0;
               }
           ], AC_MSG_RESULT(no), [
               AC_MSG_RESULT(yes)
               AC_DEFINE(STRICT_ISO_C99, 1, [Defined if strict ISO C99 (9899:1999) is requested or required.])
           ], [
               AC_MSG_RESULT(no - assumed because cross-compiling)
])

AST_X11_SUPPORT()
AST_IMLIB2_SUPPORT()
AST_MMX_SUPPORT()
AST_ARG_REGEXP(REGEXP)
AST_ARG_BACKQUOTE_EXEC(ALLOW_BACKQUOTE_EXEC)
AST_PTHREADS()

LIBCURL_CHECK_CONFIG([yes], [7.9.0], [], [])

AST_FLAGS()
AC_SUBST(CC)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)

basedir=.
AC_OUTPUT(include/libast/types.h Makefile include/Makefile \
          include/libast/Makefile src/Makefile test/Makefile \
          libast-config)
test -f libast-config && chmod 755 libast-config

AST_STATUS()
