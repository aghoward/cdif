GCC = g++
CXX_FLAGS = --std=c++17 -I../ -I./mocks
LIBS = -lgtest_main -lgmock_main -lgtest -lgmock -lpthread

OBJDIR = ./obj
TESTOBJS = ${OBJDIR}/registration_tests.o ${OBJDIR}/registrar_tests.o

all: unittests

clean:
	rm -Rf unittests ${OBJDIR}

${OBJDIR}:
	if [[ ! -e ${OBJDIR} ]]; then mkdir ${OBJDIR}; fi;

${OBJDIR}/%.o: %.cc ${OBJDIR}
	$(GCC) -o $@ -c ${CXX_FLAGS} $<

unittests: ${TESTOBJS}
	$(GCC) -o $@ ${CXX_FLAGS} ${TESTOBJS} ${LIBS}
