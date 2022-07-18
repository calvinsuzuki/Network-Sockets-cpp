#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#include <netinet/in.h>
#include <errno.h>
#include <thread>
#include <pthread.h>
#include <signal.h>
#include <atomic>

using namespace std;

#define ERROR -1
#define BUFFER_SZ 4096
#define MAX_CLIENTS 10

// Global variables
volatile atomic<int> flag(0);

// Socket variables
int sockfd = 0;
char name[32];

void catch_ctrl_c_and_exit(int sig) { 
    flag = 1;
    }

void str_overwrite_stdout() {
    printf("%s", "> ");
    fflush(stdout);
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

void sendHandler() {
    char message[BUFFER_SZ] = {};
    char buffer[BUFFER_SZ + 32] = {};

    string plain_text, frag_text;

    while (1) {
        str_overwrite_stdout();
        
        // Read the string
        getline(cin, plain_text);
        plain_text = plain_text.substr(0, BUFFER_SZ-5);

        // Transform to array of char
        char message[frag_text.length()];
        strcpy( message, frag_text.c_str() );

        if (strcmp(message, "exit") == 0) {
            break;
        } else {
            sprintf(buffer, "%s: %s\n", name, message);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(message, BUFFER_SZ);
        bzero(buffer, BUFFER_SZ + 32);
    }
    catch_ctrl_c_and_exit(2);
}

class Client{
    public:
        Client(char* _name, unsigned int _port, const char * _ipAddrs){
            strcpy(_name, name);
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

            //	Connect to the server on the socket
            socketConnect();
            
            cout << "----------------SERVER CONNECTED-----------------" << endl;

            thread recv_thrd(recvHandler);
            thread send_thrd(sendHandler);

            while (1)
            {
                // Infinite loop waiting for flag
                if(flag){
                    printf("\nBye\n");
                    break;
                }
            }    

            close(_socket);
            return 0;
        }

    protected:
    private:
        const char *ipAddrs;
        char name[32];
        unsigned int port;
        sockaddr_in hint;
        int _socket;
        int uid = 10;

        sockaddr_in socketHandler() {
            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(port);
            /* inet_pton(AF_INET, ipAddrs, &hint.sin_addr); DEPRECATED */
            hint.sin_addr.s_addr = inet_addr(ipAddrs); // This accepts IPv6
            return hint;
        }
    
        int createSocket(int __domain, int __type, int __protocol) {
            int sock = socket(__domain, __type, __protocol);
            if (sock == -1) {
                cout << "SOCKET EXCEPTION: Could not create!";
                return ERROR;
            }
            return sock;
        }

        int socketConnect() {
            int connectRes = connect(_socket, (sockaddr*)&hint, sizeof(hint));
            if (connectRes == -1) {
                cout << "CONNECT EXCEPTION: Server not reachable!";
                return ERROR;
            }
            else return 0;
        }

        };
