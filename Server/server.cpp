#include "../Socket/Server.cpp"

using namespace std;

#define ERROR -1

int main() {   
    Server server(53400, "127.0.0.1");
    cout << server.startServer();
}