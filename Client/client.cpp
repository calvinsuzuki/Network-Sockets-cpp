#include "../Socket/Socket.cpp"

using namespace std;

#define ERROR -1

int main() {   
    Socket client(54400, "127.0.0.1");

    cout << client.startClient();
}