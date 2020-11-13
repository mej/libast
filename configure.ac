# Configure template for LibAST
#
# Copyright (C) 1997-2013, Michael Jennings <mej@eterm.org>
# 
# Copyright (C) 2013, The Regents of the University of California,
# through Lawrence Berkeley National Laboratory (subject to receipt of
# any required approvals from the U.S. Dept. of Energy).
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies of the Software, its documentation and
# marketing & publicity materials, and acknowledgment shall be given in
# the documentation, materials and software packages that this Software
# was used.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

AC_INIT([LibAST], [0.8.1])
AC_CONFIG_SRCDIR([configure.ac])
AM_INIT_AUTOMAKE

AC_DEFINE_UNQUOTED(LIBAST_VERSION, "$VERSION", [Version])
AC_CONFIG_MACRO_DIR([.])

AC_CONFIG_HEADERS([config.h include/libast/sysdefs.h])
AC_CONFIG_FILES([include/libast/types.h Makefile include/Makefile
                 src/Makefile test/Makefile libast-config])

dnl# Set some basic variables
AUTHORS="Michael Jennings (mej@eterm.org and mej@lanl.gov)"
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
               AC_MSG_RESULT([no - assumed because cross-compiling])
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
AH_TOP([
#pragma once
])
AH_BOTTOM([
])
AC_OUTPUT

test -f libast-config && chmod 755 libast-config

AST_STATUS()
