# $FreeBSD$

.PATH:	${.CURDIR} ${.CURDIR}/../../../sbin/casper

PROG=	pwd

SRCS=	pwd.c

LIBADD=	casper nv pjdlog

BINDIR=	/libexec/casper

CFLAGS+=-I${.CURDIR}
CFLAGS+=-I${.CURDIR}/../../../lib/libcapsicum
CFLAGS+=-I${.CURDIR}/../../../lib/libcasper
CFLAGS+=-I${.CURDIR}/../../../lib/libpjdlog
CFLAGS+=-I${.CURDIR}/../../../sbin/casper

MAN=

.include <bsd.prog.mk>
