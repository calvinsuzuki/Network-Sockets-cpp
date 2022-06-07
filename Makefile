UTIL=
MAIN=main.cpp
BINARY=Main
ZIP=Projeto_Parte1

all:
	g++ -Wall -g $(UTIL) $(MAIN) -o $(BINARY) -lm

run:
	./$(BINARY)

clear:
	clear

r: clear all run
	
debug:
	gcc -DDEBUG -Wall $(MAIN) $(UTIL) -o $(BINARY)

valgrind:
	valgrind --tool=memcheck --leak-check=full  --track-origins=yes --show-leak-kinds=all --show-reachable=yes ./$(BINARY)

clean:
	@rm *.o

zip:
	@zip -r $(ZIP).zip *

delzip:
	@rm $(ZIP).zip
