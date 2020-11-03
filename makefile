output: main.o pamHandler.o displayer.o
	g++ -lsfml-graphics -lsfml-window -lsfml-system -lpam -lpthread main.o pamHandler.o displayer.o -o output

main.o: main.cpp
	g++ -c  main.cpp

displayer.o: displayer.cpp
	g++ -c displayer.cpp

pamHandler.o: pamHandler.c
	g++ -c pamHandler.c