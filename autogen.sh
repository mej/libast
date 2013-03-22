#!/bin/sh
# Run this to generate all the initial makefiles, etc.

broken() {
    echo
    echo "You need libtool, autoconf, and automake.  Install them"
    echo "and try again.  Get source at ftp://ftp.gnu.org/pub/gnu/"
    echo "ERROR:  $1 not found."
    exit -1
}
abort() {
	echo
	echo "Running '$1' failed :("
	echo "Try updating the package on your system and try again."
	exit -2
}

DIE=0

echo "Generating configuration files for libast, please wait...."

if autoreconf -V >/dev/null 2>&1 ; then
    set -x
    autoreconf -f -i
else
    LIBTOOLIZE_CHOICES="$LIBTOOLIZE libtoolize glibtoolize"
    ACLOCAL_CHOICES="$ACLOCAL aclocal"
    AUTOCONF_CHOICES="$AUTOCONF autoconf"
    AUTOHEADER_CHOICES="$AUTOHEADER autoheader"
    AUTOMAKE_CHOICES="$AUTOMAKE automake"

    for i in $LIBTOOLIZE_CHOICES ; do
        $i --version </dev/null >/dev/null 2>&1 && LIBTOOLIZE=$i && break
    done
    [ "x$LIBTOOLIZE" = "x" ] && broken libtool

    for i in $ACLOCAL_CHOICES ; do
        $i --version </dev/null >/dev/null 2>&1 && ACLOCAL=$i && break
    done
    [ "x$ACLOCAL" = "x" ] && broken automake

    for i in $AUTOCONF_CHOICES ; do
        $i --version </dev/null >/dev/null 2>&1 && AUTOCONF=$i && break
    done
    [ "x$AUTOCONF" = "x" ] && broken autoconf

    for i in $AUTOHEADER_CHOICES ; do
        $i --version </dev/null >/dev/null 2>&1 && AUTOHEADER=$i && break
    done
    [ "x$AUTOHEADER" = "x" ] && broken autoconf

    for i in $AUTOMAKE_CHOICES ; do
        $i --version </dev/null >/dev/null 2>&1 && AUTOMAKE=$i && break
    done
    [ "x$AUTOMAKE" = "x" ] && broken automake

    # Export them so configure can AC_SUBST() them.
    export LIBTOOLIZE ACLOCAL AUTOCONF AUTOHEADER AUTOMAKE

    # Run the stuff.
    (set -x && $LIBTOOLIZE -c -f) || abort libtool
    (set -x && $ACLOCAL) || abort aclocal
    (set -x && $AUTOCONF) || abort autoconf
    (set -x && $AUTOHEADER) || abort autoheader
    (set -x && $AUTOMAKE -a -c) || abort automake
fi

# Run configure.
if test x"$NOCONFIGURE" = x; then
    (set -x && ./configure "$@")
fi
