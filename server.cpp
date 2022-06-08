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

    int opt = true;
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        cout << "SOCKET EXCEPTION: Could not create!";
        return ERROR;
    }

    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        return ERROR;
    } 

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if( bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
        cout << "BINDING EXCEPTION: Can't bind to IP/Port";
        return ERROR;
    }

    if( listen(listening, SOMAXCONN) == -1 ) {
        cout << "LISTENING EXCEPTION: Can't listen";
        return ERROR;
    }

    sockaddr_in client;
    socklen_t clientSize;
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

    if( clientSocket == -1) {
        cout << "CONNECTION EXCEPTION: Client could not connect!";
        return ERROR;
    }

    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

    if( result ) {
        cout << host << " connected on " << svc << endl;
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin_port) << endl;
    }

    char buf[4096];
    while( true ) {
        memset( buf, 0, 4096 );
        int bytesRecv = recv(clientSocket, buf, 4096, 0);
        if (bytesRecv == -1) {
            cout << "There was a connection issue" << endl;
            break;
        }
        if ( bytesRecv == 0 ) {
            cout << "The client disconnected" << endl;
            break;
        }

        cout << "Received: " << string(buf, 0, bytesRecv) << endl;

        // Resend message
        send(clientSocket, buf, bytesRecv + 1, 0);
    }

    close(clientSocket);

    return 0;
}