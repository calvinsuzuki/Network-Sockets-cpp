#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
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

// Colors
string red = "\033[31m";
string green = "\033[32m";
string blue = "\033[34m";
string def = "\033[39m";

// Global variables
volatile atomic<int> flag(0);
// mutex m;

/* Cathes the CTRL+C to close server */
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void recvHandler(int mySocket) {
    char message[BUFFER_SZ];
    string str;
    while ( true ) {
        int receive = recv(mySocket, message, BUFFER_SZ, 0);
        if (receive > 0) {
            // Send receive 
            // if( write(mySocket, "ACK\n", 3) < 0 ) {   
            //     cout << "ERROR: write to descriptor failed" << endl;
            //     break;
            // }
            str = message;

            // Colors the messages
            if ( str.find(": ") != string::npos ) {
                cout << blue 
                    << str.substr(0, str.find(':'))
                    << def;
                cout << str.substr(str.find(':'), str.length());
                
            } else if ( (str.find("** ") != string::npos) && (str.find(" **") != string::npos) ){
                cout << str.substr(0, str.find("** ")+3) 
                    << green << str.substr(str.find("** ")+3, str.find(" **")-3) << def
                    << str.substr(str.find(" **"), str.length());
            } else if ( (str.find(">> ") != string::npos) && (str.find(" <<") != string::npos) ){
                cout << str.substr(0, str.find(">> ")+3) 
                    << red << str.substr(str.find(">> ")+3, str.find(" <<")-3) << def
                    << str.substr(str.find(" <<"), str.length());
            } else 
                cout << str;

            // Split command and command argument
            
        } else if (receive == 0) {
            break;
        } else {
            cout << "ERROR IN RECEIVE MESSAGE" << endl;
        }
        // Clear buffer
        memset(message, 0, BUFFER_SZ);
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
        // Remove final spaces
        while( true ) {
        if ( plain_text[plain_text.length()-1] == ' ' ) {
            plain_text = plain_text.substr(0, plain_text.length()-1);
        }
        else {
            break;
        }
    }

        // Fragments the message 
        sendFragments(plain_text, mySocket);
        if ( plain_text == "/quit" ) break;
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

        cout << "\n\u001b[33m**      Golden Server connected!      **\u001b[0m" << endl;
        cout << "\u001b[33mYour name on the server is: " << nick << "\u001b[0m" << endl;

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
            cout << "\u001b[33mTrying to reach server...\u001b[0m"<< endl;
            cout << "\u001b[33mIP: " << ipAddrs << " Port: " << port << "\u001b[0m" << endl;
            int connectRes = connect(_socket, (sockaddr*)&hint, sizeof(hint));
            if (connectRes == -1) {
                cout << "CONNECT EXCEPTION: Server not reachable!" << endl;
                return ERROR;
            } else
                return 0;
        }
};
