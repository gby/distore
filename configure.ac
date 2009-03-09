#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(distore, 0.0.1, info@codefidence.com)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR([src/main.c])

AM_INIT_AUTOMAKE()

# Wiered `@<:@' and similar are just escape string for `[' and `]'
AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug],
			                                        [Build for debugging]))

CFLAGS="$CFLAGS -pipe -Wall"
LDFLAGS="$LDFLAGS -Wl,-z -Wl,defs -Wl,--as-needed"
# If debug requested - set compiler flags
# and enable debug for asn1c compiler internals
if [[ "$enable_debug" != "" ]]; then
	CFLAGS="$CFLAGS -O0 -g3"
	CPPFLAGS="$CPPFLAGS -DDEBUG"
else
	CFLAGS="$CFLAGS -O3"
	CPPFLAGS="$CPPFLAGS -DNDEBUG"
	LDFLAGS="$LDFLAGS -O3"
fi
AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([event], [event_init], , AC_MSG_ERROR([Can not find libevent library]))
AC_CHECK_HEADER([event.h], , AC_MSG_ERROR([Can not find libevent headers - event.h], ))
# Checking for OpenSSL libcrypto library
AC_CHECK_LIB([crypto], [OpenSSL_add_all_digests], , AC_MSG_ERROR([Can not find OpenSSL libcrypto library]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Initialize hash library
AC_CONFIG_SUBDIRS([lib/libghthash-0.6.2])

# Checks for library functions.
AC_CHECK_FUNCS([memset socket])

AC_CONFIG_FILES([Makefile
				 etc/Makefile
				 lib/Makefile
                 src/Makefile])

AC_OUTPUT
