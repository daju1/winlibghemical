####################################################################################################
####################################################################################################
####################################################################################################

### we need this section only if we wish to compile/link to the MPQC program. this is disabled
### by default!!! compile and install both the MPQC program and the SC toolkit before using this.

### also remember to check the file config.h.OLD, and uncomment the MPQC-related macro there!!!

#MPQCFLAGS =	$(shell sc-config --cppflags)
#MPQCLIBS =	$(shell sc-config --libs)

####################################################################################################
####################################################################################################
####################################################################################################

CC =		g++

BASEFLAGS =	-g -c -O1 -Wall
CFLAGS =	-I../

EX1 =		example1
EX2 =		example2
EX3 =		example3

MOPAC7LIBS =	$(shell pkg-config --libs libmopac7)
LIBS =		$(MOPAC7LIBS) -lg2c -lm

OFILES =	../*.o

EX1SRC =	fileio.cpp example1.cpp
EX2SRC =	fileio.cpp example2.cpp
EX3SRC =	streamio.cpp example3.cpp

OBJ =		$(SRC:.cpp=.o)

####################################################################################################

all:		$(EX1) $(EX2) $(EX3)

$(EX1):		fileio.o example1.o
		$(CC) -o $(EX1) fileio.o example1.o $(OFILES) $(MPQCLIBS) $(LIBS)

$(EX2):		fileio.o example2.o
		$(CC) -o $(EX2) fileio.o example2.o $(OFILES) $(MPQCLIBS) $(LIBS)

$(EX3):		streamio.o example3.o
		$(CC) -o $(EX3) streamio.o example3.o $(OFILES) $(MPQCLIBS) $(LIBS)

%.o: %.cpp
		$(CC) $(BASEFLAGS) $(CFLAGS) $(MPQCFLAGS) $(@:.o=.cpp)

depend:
		$(CC) -c -MM $(CFLAGS) *.cpp > depend

clean:
		rm -f *.o *~ depend $(EX1) $(EX2) $(EX3)

####################################################################################################
