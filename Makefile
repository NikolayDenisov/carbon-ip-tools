CC=gcc
CC_FLAGS=-O3 -fPIC -g -c -Wall -W -std=c11
CC_FLAGS+= -pedantic
LIBS += -lm

STD_DEFINES=-D INLINE

run : ip_math.o
	${CC} ${LINK_FLAGS} -o ip_math ip_math.o $(LIBS)

ip_math.o :	ip_math.c
	${CC} ${STD_DEFINES} ${CC_FLAGS} -c ip_math.c $(LIBS)

clean :
	rm -f *.o core
	rm -f ip_math
check:
	cppcheck --template=gcc --enable=warning --std=c11 --force ip_math.c

tests:
	echo "TODO"
