include=-I/opt/local/include/glib-2.0 \
	-I/opt/local/include \
	-I/opt/local/lib/glib-2.0/include \
	-I/System/Library/Frameworks/OpenGL.framework/Headers \
	-I/System/Library/Frameworks/GLUT.framework/Headers

libs=-L/opt/local/lib

all: run
run: main.cpp  LSystem.o
	g++ $(include) $(libs)  -lglib-2.0 -ljpeg -DNDEBUG -Wall LSystem.o main.cpp -o run -framework OpenGL -lXmu -framework GLUT
LSystem.o: LSystem.h LSystem.cpp Function.h Function.cpp Rule.h parse.h command.h
	g++ -c $(include) $(libs) -DNDEBUG -Wall LSystem.cpp 

clean: 
	rm *.o

