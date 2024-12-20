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

CC = g++

LOCAL=local

BASEFLAGS = -g -c -O1 -Wall
CFLAGS    = -I../ -I../${LOCAL}/include -I../${LOCAL}/include/ghemical -I../libghemical-3.0.0/src/
LDFLAGS   = -L../${LOCAL}/lib/
PKG_CONFIG_PATH=$(dirname $(pwd))/${LOCAL}/lib/pkgconfig

EX1 =		example1
EX2 =		example2
EX3 =		example_moldyn
EX4 =		example_make_mol_plots
EX5 =		example_trimtraj

MOPAC7LIBS = $(shell pkg-config --libs libmopac7)
LIBS       = $(MOPAC7LIBS) -lghemical -loglappth -linchi -lopenbabel -lm -lGLU -lGL -lglib-2.0 

OBJ = $(SRC:.cpp=.o)
OBJ = fileio.o 

####################################################################################################

all:		$(EX1) $(EX2) $(EX3) $(EX4) $(EX5)

$(EX1):	$(OBJ) $(EX1).o
		$(CC) -o $(EX1) $(OBJ) $(EX1).o $(LDFLAGS) $(MPQCLIBS) $(LIBS)

$(EX2):	$(OBJ) $(EX2).o
		$(CC) -o $(EX2) $(OBJ) $(EX2).o $(MPQCLIBS) $(LDFLAGS) $(LIBS)

$(EX3):	$(OBJ) $(EX3).o
		$(CC) -o $(EX3) $(OBJ) $(EX3).o $(MPQCLIBS) $(LDFLAGS) $(LIBS)

$(EX4):	$(OBJ) $(EX4).o
		$(CC) -o $(EX4) $(OBJ) $(EX4).o $(MPQCLIBS) $(LDFLAGS) $(LIBS)

$(EX5):	$(OBJ) $(EX5).o
		$(CC) -o $(EX5) $(OBJ) $(EX5).o $(MPQCLIBS) $(LDFLAGS) $(LIBS)

%.o: %.cpp
		$(CC) $(BASEFLAGS) $(CFLAGS) $(MPQCFLAGS) $(@:.o=.cpp)

depend:
		$(CC) -c -MM $(CFLAGS) *.cpp > depend

clean:
		rm -f *.o *~ depend $(EX1) $(EX2) $(EX3) $(EX4) $(EX5)

####################################################################################################
