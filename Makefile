SRC_DIR=src
INC_DIRS=inc
CC = gcc
CC-RV = riscv64-unknown-elf-gcc
CFLAGS-M32 = -Wall -m32 -O3

m32:
	${CC} ${CFLAGS-M32} -I ${INC_DIRS}/ ${SRC_DIR}/pgtbl.c ${SRC_DIR}/main.c ${SRC_DIR}/bit_manip.c -o pgtbl

risc-v:
	${CC-RV} -Wall -O3 -I ${INC_DIRS}/ ${SRC_DIR}/pgtbl.c ${SRC_DIR}/main.c ${SRC_DIR}/bit_manip.c -o pgtbl-rv

clean:
	rm -rf pgtbl
	rm -rf pgtbl-rv
	rm -rf docs/

docs:
	doxygen doxygen.conf
