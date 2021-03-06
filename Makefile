UTIL=
SERVER=./Server/server.cpp
CLIENT=./Client/client.cpp
BIN_SERVER=./Server/server.bin
BIN_CLIENT=./Client/client.bin
ZIP=ProjetoFinal

all:
	g++ -Wall -g $(UTIL) $(SERVER) -o $(BIN_SERVER) -lm -pthread
	g++ -Wall -g $(UTIL) $(CLIENT) -o $(BIN_CLIENT) -lm -pthread

make_server:
	g++ -Wall -g $(UTIL) $(SERVER) -o $(BIN_SERVER) -lm -pthread

make_client:
	g++ -std=c++11 -Wall -g $(UTIL) $(CLIENT) -o $(BIN_CLIENT) -lm -pthread

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
