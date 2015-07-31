all:
	make -C fxptlibs/ops
	make -C dsp_fx
	make -C tty
	make -C code

clean:
	make -C fxptlibs/ops clean
	make -C dsp_fx clean
	make -C tty clean
	make -C code clean

clobber:
	make -C fxptlibs/ops clobber
	make -C dsp_fx clobber
	make -C tty clobber
	make -C code clobber
