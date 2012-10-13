CFLAGS = -g -c `pkg-config --cflags libgnomeui-2.0 libgnome-2.0`
LIBS = `pkg-config --libs libgnomeui-2.0 libgnome-2.0`

container:	container.o
		g++ -o container ${LIBS} container.o

%.o: %.cpp
		g++ ${CFLAGS} $< -o $@

clean:
		rm -f *.o
		rm -f container
