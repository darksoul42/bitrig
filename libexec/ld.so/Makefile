#	$OpenBSD: Makefile,v 1.56 2015/05/26 20:15:54 miod Exp $

SUBDIR=ldconfig ldd
MAN=	ld.so.1

.include <bsd.own.mk>

.if defined(NOPIC)
NOPROG=
.else
PROG=	ld.so

VPATH=${.CURDIR}/../../lib/libc/string

SRCS=	ldasm.S boot.c loader.c resolve.c dlfcn.c dl_printf.c rtld_machine.c
SRCS+=	path.c util.c sod.c strsep.c strtol.c dir.c library_subr.c dl_prebind.c
SRCS+=	dl_realpath.c dl_uname.c dl_dirname.c strlcat.c strlen.c trace.c
SRCS+=	malloc.c reallocarray.c library.c tls.c

.include "${.CURDIR}/${MACHINE_CPU}/Makefile.inc"
.PATH:	${.CURDIR}/${MACHINE_CPU}

DEBUG?=	-g
CFLAGS += -Wall -fno-builtin -Werror
CFLAGS += -I${.CURDIR} -I${.CURDIR}/${MACHINE_CPU} \
	-D'DEF_WEAK(x)=asm("")' -D'DEF_STRONG(x)=asm("")' \
	-Dstrsep=_dl_strsep -Dstrlcat=_dl_strlcat -Dstrlen=_dl_strlen
CDIAGFLAGS=
INSTALL_STRIP=

ELF_LDFLAGS+=--shared -Bsymbolic --no-undefined

candidate=	$(PROG).test
CLEANFILES+=	${candidate}
.if ${HOSTCC} == ${CC}
test_prog=	test-$(PROG)
CLEANFILES+=	test-$(PROG)
test_cmd= 	'ulimit -c 0; [ "`${.OBJDIR}/$(test_prog) ok`" = ok ]'

$(test_prog):
	printf '#include <stdio.h>\nint main(int argc, char **argv){ printf("%%s\\n", argv[1]); }\n' | \
	$(CC) -x c - -Wl,-dynamic-linker,${.OBJDIR}/$(candidate) -o $@

.endif
$(PROG): $(test_prog)
	$(LD) -x -e _dl_start $(ELF_LDFLAGS) -o $(candidate) $(OBJS) $(LDADD)
	eval $(test_cmd)
	cp $(candidate) $@
.endif

.include <bsd.prog.mk>

boot.o: boot.c
	${COMPILE.c} -fno-stack-protector ${.IMPSRC}
