#include "../Socket/Socket.cpp"

using namespace std;

#define ERROR -1

int main() {   
    Socket server(53400, "127.0.0.1");
    cout << server.startServer();
}