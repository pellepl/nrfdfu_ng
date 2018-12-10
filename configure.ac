dnl
dnl
dnl configure.in:
dnl
dnl Copyright (c) 2015 James McKenzie <software@madingley.org>,
dnl All rights reserved.
dnl
dnl
AC_PREREQ(2.13)
AC_INIT(nrfdfu,1.00)
AC_CONFIG_SRCDIR(nrfdfu.c)
AM_INIT_AUTOMAKE

AC_PROG_CC
AM_PROG_CC_C_O
AC_SYS_LARGEFILE



AM_CONFIG_HEADER(config.h)

AC_CHECK_PROG(CPROTO,cproto,cproto)


AC_CHECK_HEADERS(fcntl.h stdlib.h stdint.h unistd.h)

AC_C_INLINE
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM
dnl AC_CHECK_FUNCS()

dnl CFLAGS=-g

PKG_CHECK_MODULES(LIBZIP,libzip)
PKG_CHECK_MODULES(JSON_C,json-c)
PKG_CHECK_MODULES(BLUEZ,bluez)

AC_OUTPUT([Makefile])

