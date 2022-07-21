#include "../Socket/Server.cpp"

using namespace std;

#define ERROR -1

int main() {  
    // define the server port and ip
    int port = 53400;
    char ip[] = "127.0.0.1";
    Server server(port, ip);
    cout << server.startServer();
}