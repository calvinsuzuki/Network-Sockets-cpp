UTIL=
SERVER=server.cpp
CLIENT=client.cpp
BIN_SERVER=Server
BIN_CLIENT=Client
ZIP=Projeto_Parte1

all:
	g++ -Wall -g $(UTIL) $(SERVER) -o $(BIN_SERVER) -lm
	g++ -Wall -g $(UTIL) $(CLIENT) -o $(BIN_CLIENT) -lm

make_server:
	g++ -Wall -g $(UTIL) $(SERVER) -o $(BIN_SERVER) -lm

make_client:
	g++ -Wall -g $(UTIL) $(CLIENT) -o $(BIN_CLIENT) -lm

run_server:
	./$(BIN_SERVER)

run_client:
	./$(BIN_CLIENT)

clear:
	clear

sv: clear make_server run_server

client: clear make_client run_client
	
debug:
	gcc -DDEBUG -Wall $(MAIN) $(UTIL) -o $(BINARY)

valgrind_server:
	valgrind --tool=memcheck --leak-check=full  --track-origins=yes --show-leak-kinds=all --show-reachable=yes ./$(BIN_SERVER)

valgrind_client:
	valgrind --tool=memcheck --leak-check=full  --track-origins=yes --show-leak-kinds=all --show-reachable=yes ./$(BIN_CLIENT)

clean:
	@rm *.o

zip:
	@zip -r $(ZIP).zip *

delzip:
	@rm $(ZIP).zip
