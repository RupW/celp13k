
CCTARGET=./libdsp40.a
CC= gcc
CCFLAGS=-Wall -O3
CODER_INC_DIR=../include
CCINCDIR=-I.
CCOBJS=\
	basic_op.o\
	basic_op40.o\
	lib_wmp_fx.o\
	math_adv.o\
	math_adv40.o\
	math_ext32.o\
	math_ext40.o\

CCINCS=

.c.o:
	${CC} ${CCFLAGS} ${CCINCDIR} -c $*.c

${CCTARGET}: ${CCOBJS}
	ar rv ${CCTARGET} ${CCOBJS}
	ranlib ${CCTARGET}

${CCOBJS}: ${CCINCS}

clean:
	rm -f *.o
clobber:
	rm -f *.o ${CCTARGET}

basic_op.o: basic_op.c
basic_op40.o: basic_op40.c
lib_wmp_fx.o: lib_wmp_fx.c
math_adv.o: math_adv.c
math_adv40.o: math_adv40.c
math_ext32.o: math_ext32.c
math_ext40.o: math_ext40.c
