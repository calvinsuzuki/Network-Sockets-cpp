#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

#define ERROR -1

int main() {
    //	Create a socket
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == -1)
        return ERROR;    

    //	Create a hint structure for the server we're connecting with
    int port = 54400;
    string ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    //	Connect to the server on the socket
    int connectRes = connect(client, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1)
        return ERROR;
    
    // Control variables
    int bufsize = 4096;
    char buf[4096];
    string userInput;
    
    cout << endl;
    cout << "------WELCOME TO SERVERLESS, YOUNG CLIENT--------" << endl;
    cout << "-------------------------------------------------" << endl;
    cout << "---Use the character '$' to finish the message---" << endl;
    cout << endl;

    //	While loop:
    while ( true ) {
        cout << "\nClient: ";
        do {
            cin >> buf;
            send(client, buf, bufsize, 0);
        } while (*buf != '$');

        cout << "Waiting server message...";
        cout << "\nServer: ";
        do {
            recv(client, buf, bufsize, 0);
            cout << buf << " ";
        } while (*buf != '$');
    }

    close(client);
    return 0;
}