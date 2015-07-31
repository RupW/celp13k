
CCTARGET=./libtty.a
CC= gcc
CCFLAGS=-Wall -O3
CODER_INC_DIR=../code
FXPTLIBS=../dsp_fx
CCINCDIR=-I. -I$(CODER_INC_DIR) -I$(FXPTLIBS) -I ../dtmf
CCOBJS=\
	bdo2dit.o\
	dit2a.o\
	ttystate.o\
	tone_gen.o\
	tty_bit.o\
	tty_char.o\
	tty_dbg.o\
	tty_dec.o\
	tty_enc.o\
	tty_gen.o\
	tty_glob.o\
	tty_hdr.o\
	tty_rate.o

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

$(CCOBJS): tty.h $(CODER_INC_DIR)/tty_hdr.h
