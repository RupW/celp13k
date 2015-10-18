# $Header: /home/phv/CVSROOT/qc13_fltpt_sim/makefile,v 1.1.1.1 1999/02/22 18:14:21 djy Exp $
#Makefile for celp13k
TARGET=celp13k
CC=gcc
TTY_DIR=../tty
FXPTLIBS=../fxptlibs
CCINCDIR=-I$(TTY_DIR) -I$(FXPTLIBS)/ops -I../dsp_fx
CCLIBDIR=-L$(TTY_DIR) -L$(FXPTLIBS)/ops -L../dsp_fx
CFLAGS=-Wall $(CCINCDIR) $(CCLIBDIR) -DDOS=1 -O3
LINTFILES= ${SRC:.c=.ln}
#LINTFLAGS =  -hacbiu
#LINTFLAGS =  -hacb
CPPFLAGS = -I.
LINT = /usr/bin/lint
LDFLAGS = -lm -lc

SRC=\
	cb.c\
	cb_tabs.c\
	celp13k.c\
	coderate.c \
	decode.c\
	encode.c \
	fer_sim.c\
	filter.c\
	frontfil.c\
	hammtab.c\
	io.c\
        io_qcp.c\
	init.c\
	log_10.c\
	lpc.c\
	lsp.c \
	modetabs.c\
	pack.c\
	pitch.c\
	postfilt.c\
	quantabs.c\
	quantize.c\
	ratedec.c\
	snr.c\
	target.c

OBJ= ${SRC:.c=.o}

OBJDOS= ${SRCDOS:.c=.o}

all: ${TARGET}


# For some reason it's necessary to manually link tty_glob.o on my mac, even though it is
# definitely included in libtty.a.
${TARGET}:$(OBJ) $(TTY_DIR)/libtty.a $(FXPTLIBS)/ops/libops.a
	$(CC) $(CFLAGS) $(OBJ) -lm -ltty $(TTY_DIR)/tty_glob.o -ldsp40 -lops -o $@


${TARGET}.lint: ${LINTFILES}
	${LINT.c} -lm -lc ${LINTFILES}

clean:
	rm -f *.o

clobber:
	rm -f *.o ${TARGET} ${TARGET}.exe

$(OBJ): celp.h coder.h
fer_sim.o: fer_sim.h

