#include "../Socket/Socket.cpp"

using namespace std;

#define ERROR -1

int main() {   
    Socket server(54400, "127.168.0.115");
    cout << server.startServer();
}