TESTS=$(sort $(wildcard *.fun))
PROGS=$(subst .fun,,$(TESTS))
OUTS=$(patsubst %.fun,%.out,$(TESTS))
DIFFS=$(patsubst %.fun,%.diff,$(TESTS))
RESULTS=$(patsubst %.fun,%.result,$(TESTS))

#CFILES=$(sort $(wildcard *.c))

#PPC=/u/gheith/public/ppc64-linux/bin
#AS=${PPC}/as
#LD=${PPC}/ld

CC = arm-linux-gnueabi-as

UTILS = compiler emulator

.SECONDARY:

.PROCIOUS : %.o %.S %.out

CFLAGS=-MD -g -std=gnu99 -O0 -Werror -Wall
OFILES=$(subst .c,.o,$(CFILES))

all : ${UTILS}

compiler_CFILES=$(wildcard src/compiler/*.c)
emulator_CFILES=$(wildcard src/emulator/*.c)

compiler : Makefile ${compiler_CFILES} 
	gcc $(CFLAGS) -o $@ ${compiler_CFILES}
	
emulator : Makefile ${emulator_CFILES}
	gcc $(CFLAGS) -o $@ ${emulator_CFILES}
	
%.o : %.S all Makefile
	($(CC) -o $*.o $*.S) || touch $@

%.S : compiler %.fun
	@echo "========== $* =========="
	(./compiler < $*.fun > $*.S) || touch $@

progs : $(PROGS)

#ppc.o : ppc.asm
#	($(AS) -o $*.o $*.asm) || touch $@

$(PROGS) : % : %.o
	(arm-linux-gnueabi-ld -e entry -o $@ $*.o) || touch $@

outs : $(OUTS)

$(OUTS) : %.out : emulator Makefile %
	(./emulator $* > $*.out) || touch $@

diffs : $(DIFFS)

%.ok:
	touch $@

$(DIFFS) : %.diff : all Makefile %.out %.ok
	@(((diff -b $*.ok $*.out > /dev/null 2>&1) && (echo "===> $* ... pass")) || (echo "===> $* ... fail" ; echo "----- test -----"; cat $*.fun; echo "----- expected ------"; cat $*.ok ; echo "----- found -----"; cat $*.out)) > $*.diff 2>&1

$(RESULTS) : %.result : all Makefile %.diff
	@cat $*.diff

test : all Makefile $(DIFFS)
	@cat $(DIFFS)

clean :
	rm -f $(PROGS)
	rm -f $(UTILS)
	rm -f *.S
	rm -f *.out
	rm -f *.d src/emulator/*.d src/compiler/*.d
	rm -f *.o src/emulator/*.o src/compiler/*.o
	rm -f emulator compiler
	rm -f *.diff

-include *.d
