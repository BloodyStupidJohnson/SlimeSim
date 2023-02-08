build: main.cpp
	rm -f main.o
	g++ -c main.cpp -lpthread
	rm -f app
	g++ main.o -o app -lsfml-graphics -lsfml-window -lsfml-system -lpthread

run: build
	./app

install-dependencies:
	sudo apt install libsfml-dev
