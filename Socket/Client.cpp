#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

#define ERROR -1
#define BUFFER_SZ 4096
#define MAX_CLIENTS 10
#define MAX_NICK_SIZE 50

// Global variables
volatile atomic<int> flag(0);
// mutex m;

/* Cathes the CTRL+C to close server */
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void str_overwrite_stdout() {
    fflush(stdout);
    printf("> ");
    fflush(stdout);
}

void recvHandler(int mySocket) {
    char message[BUFFER_SZ];
    while ( true ) {
        int receive = recv(mySocket, message, BUFFER_SZ, 0);
        if (receive > 0) {
            printf("%s\n", message);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else {
            cout << "ERROR IN RECEIVE MESSAGE" << endl;
        }
        // Clear buffer
        memset(message, 0, sizeof(message));
    }
}

// TODO: plain_text -= frag_text
int sendFragments(string plain_text, int mySocket) {
    string frag_text;
    
    int toSend = plain_text.length();

    while( toSend > 0) {      
        // Fragments the message
        if (toSend >= BUFFER_SZ) {
            frag_text = plain_text.substr(0, BUFFER_SZ);
            plain_text = plain_text.substr(BUFFER_SZ, plain_text.length());
            send(mySocket, frag_text.c_str(), frag_text.length(), 0);
        } 
        // When the message is already small enough
        else {
            frag_text = plain_text;
        }

        send(mySocket, frag_text.c_str(), frag_text.length(), 0);
        
        //Subtracts the toSend variable
        toSend -= frag_text.length();
    }

    return 0;
} 

void sendHandler(int mySocket) {

    string plain_text;

    // Read the string until find EOF
    while ( getline(cin, plain_text) ) {
        str_overwrite_stdout();
        // Fragments the message 
        if ( plain_text == "/quit" ) break;
        sendFragments(plain_text, mySocket);
    }
 
    catch_ctrl_c_and_exit(2);
}

class Client {
    public:
    Client(const char* _nick, unsigned int _port, const char* _ipAddrs) {
        strcpy(nick, _nick);
        port = _port;
        ipAddrs = _ipAddrs;
        hint = socketHandler();
    }

    int startClient() {
        //	Create slave socket
        _socket = createSocket(AF_INET, SOCK_STREAM, 0);
        
        /* Cathes the CTRL+C to close */
        signal(SIGINT, catch_ctrl_c_and_exit);

        //	Connect to the server on the socket
        if (socketConnect() == ERROR) {
            cout << "Quiting..." << endl;
            return ERROR;
        }

        cout << "----------------SERVER CONNECTED-----------------" << endl;
        cout << "Your name on the server is: " << nick << endl;

        // Send name
        if (send(_socket, nick, MAX_NICK_SIZE, 0) == ERROR) {
            cout << "Failed to send name" << endl;
            return ERROR;
        }

        thread recv_thrd(recvHandler, _socket);
        thread send_thrd(sendHandler, _socket);
        recv_thrd.detach();
        send_thrd.detach();

        while (1) {
            // Infinite loop waiting for flag
            if (flag) {
                cout << "\nDisconnected from server!\n";
                break;
            }
        }

        close(_socket);
        return 0;
    }

    protected:
    private:
        const char* ipAddrs;
        char nick[32];
        unsigned int port;
        sockaddr_in hint;
        int _socket;

        sockaddr_in socketHandler() {
            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(port);
            hint.sin_addr.s_addr = inet_addr(ipAddrs);  // This accepts IPv6
            return hint;
        }

        int createSocket(int __domain, int __type, int __protocol) {
            _socket = socket(__domain, __type, __protocol);
            if (_socket == -1) {
                cout << "SOCKET EXCEPTION: Could not create!" << endl;
                return ERROR;
            } else
                return _socket;
        }

        int socketConnect() {
            cout << "Trying to reach server at IP: " << ipAddrs << " Port: " << port << endl;
            int connectRes = connect(_socket, (sockaddr*)&hint, sizeof(hint));
            if (connectRes == -1) {
                cout << "CONNECT EXCEPTION: Server not reachable!" << endl;
                return ERROR;
            } else
                return 0;
        }
};
