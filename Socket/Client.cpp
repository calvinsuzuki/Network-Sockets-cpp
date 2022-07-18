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

// Global variables
volatile atomic<int> flag(0);
// mutex m;

// Socket variables
int sockfd;
char name[32];

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void str_overwrite_stdout() {
    printf("%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) {  // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void recvHandler() {
    char message[BUFFER_SZ] = {};
    while (1) {
        int receive = recv(sockfd, message, BUFFER_SZ, 0);
        if (receive > 0) {
            printf("%s", message);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else {
            // -1
        }
        memset(message, 0, sizeof(message));
    }
}

// TODO: plain_text -= frag_text
int sendFragments(string plain_text) {
    string frag_text;
    
    int toSend = (int) plain_text.length();

    while( toSend > 0) {      
        // Fragments the message
        if (toSend >= max_message_size) {
            frag_text = plain_text.substr(0, max_message_size);
            send(sockfd, frag_text.c_str(), strlen(frag_text.c_str()), 0);
        } 
        // When the message is already small enought
        else {
            frag_text = plain_text;
        }

        // TODO: what is this?
        // Transform to array of char
        // char message[frag_text.length()];
        // strcpy( message, frag_text.c_str() );

        //Subtracts the toSend variable
        toSend -= frag_text.length();
    }

    return 0;
} 

void sendHandler() {

    string plain_text;

    while (1) {
        str_overwrite_stdout();
        
        // Read the string
        getline(cin, plain_text);

        plain_text = name + " > " + plain_text;
        
        sendFragments(plain_text);
    }
    catch_ctrl_c_and_exit(2);
}

// void sendHandler() {
//     char message[BUFFER_SZ] = {};
//     char buffer[BUFFER_SZ + 32] = {};

//     while (1) {
//         str_overwrite_stdout();
//         fgets(message, BUFFER_SZ, stdin);
//         str_trim_lf(message, BUFFER_SZ);

//         if (strcmp(message, "exit") == 0) {
//             break;
//         } else {
//             sprintf(buffer, "%s: %s\n", name, message);
//             send(sockfd, buffer, strlen(buffer), 0);
//         }

//         bzero(message, BUFFER_SZ);
//         bzero(buffer, BUFFER_SZ + 32);
//     }
//     catch_ctrl_c_and_exit(2);
// }

class Client {
   public:
    Client(const char* __name, unsigned int _port, const char* _ipAddrs) {
        strcpy(_name, __name);
        port = _port;
        ipAddrs = _ipAddrs;
        hint = socketHandler();
    }

    int startClient() {
        // Control variables
        int bufsize = BUFFER_SZ;
        char buf[BUFFER_SZ];
        string str_buf;
        string userInput;

        //	Create client socket
        _socket = createSocket(AF_INET, SOCK_STREAM, 0);

        if (ret == ERROR) {
            cout << "Quiting..." << endl;
            return ERROR;
        }

        //	Connect to the server on the socket
        ret = socketConnect();

        if (ret == ERROR) {
            cout << "Quiting..." << endl;
            return ERROR;
        }

        cout << "----------------SERVER CONNECTED-----------------" << endl;
        cout << "Your name on the server is: " << name << endl;

        // Send name
        ret = send(_socket, _name, 32, 0);

        if (ret == -1) {
            cout << "Failed to send name" << endl;
            return ERROR;
        }

        sockfd = _socket;
        strcpy(name, _name);

        thread recv_thrd(recvHandler);
        thread send_thrd(sendHandler);

        while (1) {
            // Infinite loop waiting for flag
            if (flag) {
                printf("\nBye\n");
                break;
            }
        }

        close(sockfd);
        return 0;
    }

   protected:
   private:
    const char* ipAddrs;
    char _name[32];
    unsigned int port;
    sockaddr_in hint;
    int _socket;
    int uid = 10;
    int ret;

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
            return ERROR, sockfd;
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
