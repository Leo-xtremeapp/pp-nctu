SHELL=/bin/sh
BENCHMARK=cg
BENCHMARKU=CG
PROGRAMNAME=cg
DATASIZE=MEDIUMN

include make.common

OBJS = cg.o \
       ${COMMON}/${RAND}.o \
       ${COMMON}/c_timers.o \
       ${COMMON}/wtime.o

${PROGRAM}: config ${OBJS}
	mkdir -p ./bin
	${CLINK} ${CLINKFLAGS} -o ${PROGRAM} ${OBJS} ${C_LIB}

.c.o:
	${CCOMPILE} $< -D${DATASIZE}

cg.o:		cg.c  globals.h

clean:
	- rm -f *.o *~
	rm -f ${COMMON}/*.o
	rm -f bin/*

run:
	${MAKE}
	taskset -c 0 ./bin/cg | grep -E 'Execution\ time*'
	taskset -c 0-7 ./bin/cg | grep -E 'Execution\ time*'
