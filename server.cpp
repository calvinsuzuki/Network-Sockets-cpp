#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

int main() {

    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        cerr << "Could not create the socket!!";
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if( bind(AF_INET, &hint, sizeof(hint)) == -1) {
        cerr << "Can't bind to IP/Port";
        return -2;
    }

    if( listen(listening, SOMAXCONN) == -1 ) {
        cerr << "Can't listen";
        return -3;
    }

    sockaddr_in client;
    socklen_t clientSize;
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

    if( clientSocket == -1) {
        cerr << "Error with client connection!";
        return -4;
    }

    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(svc, 0, NI_MAXSERV);

    int result = getnameinfo((socketaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

    if( result ) {
        cout << host << " connected on " << service << endl;
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin_port) << endl;
    }

    char buf[4096];
    while( true ) {
        memset( buf, 0, 4096 );
        int bytesRecv = recv(clientSocket, buf, 4096, 0);
        if (bytesRec == -1) {
            cerr << "There was a connection issue" << endl;
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
    return 0;
}