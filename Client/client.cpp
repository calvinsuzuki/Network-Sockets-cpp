#include "../Socket/Socket.cpp"

using namespace std;

#define ERROR -1

int main() {   
    Socket client(54400, "127.168.0.115");
    cout << endl << client.startClient();
}