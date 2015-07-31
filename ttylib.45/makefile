# =========================================================================== #
#   Lucent Technologies Fixed-Point DSP Math Library C Simulation.            #
#                                                                             #
#   Copyrite (c) 1997 Lucent Technologies.  All rights reserved.              #
#   Lucent Technologies proprietary and confidential.                         #
# --------------------------------------------------------------------------- #
CCTARGET=./libtty.a
FXPTLIBS=../fxptlibs
CC=gcc
CCFLAGS=-Wall
CODER_INC_DIR=../code
CCINCDIR=-I$(CODER_INC_DIR) -I$(FXPTLIBS)/ops
CCOBJS=\
	bdo2dit.o\
	dit2a.o\
	tone_gen.o\
	tty_dbg.o\
	tty_dec.o\
	tty_enc.o\
	tty_gen.o\
	tty_glob.o\
	tty_hdr.o

CCINCS=

.c.o:
	${CC} ${CCFLAGS} ${CCINCDIR} -c $*.c

${CCTARGET}: ${CCOBJS}
	ar rv ${CCTARGET} ${CCOBJS}
	ranlib ${CCTARGET}
#	rm -f *.o

${CCOBJS}: ${CCINCS}

clean:
	rm -f *.o
clobber:
	rm -f *.o ${CCTARGET}

$(CCOBJS): tty.h $(CODER_INC_DIR)/tty_hdr.h
