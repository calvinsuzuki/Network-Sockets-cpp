#include "../Socket/Socket.cpp"

using namespace std;

#define ERROR -1

int main() {   
    Socket client(54400, "192.168.0.115");

    cout << client.startClient();
}