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

class Socket {
    public:
        Socket(unsigned int _port, const char * _ipAddrs) {
            port = _port;
            ipAddrs = _ipAddrs;
            hint = socketHandler();
        }
        
        int startClient() {
            //	Create client socket
            _socket = createSocket(AF_INET, SOCK_STREAM, 0);

            //	Connect to the server on the socket
            socketConnect();
            
            // Control variables
            int bufsize = BUFFER_SZ;
            char buf[BUFFER_SZ];
            string str_buf;
            string userInput;
            
            cout << "----------------SERVER CONNECTED-----------------" << endl;


            close(_socket);
            return 0;
        }

        int startServer() {

            // Start clients list
            client_t *clients[MAX_CLIENTS];

            // Create master socket
            _socket = createSocket(AF_INET, SOCK_STREAM, 0);

            signal( SIGPIPE, SIG_IGN ); // Ignore pipe signals ????
            
            multipleConnections(); // with same addrs AND port
            socketBind();
            socketListen();

            cout << endl << "--------------WELLCOME TO CHATROOM---------------\n" << endl;  

            int listenFileDescriptor = socketAccept(); // Chat p2p
            // int listenFileDescriptor;
            // while( true ) {
            //     listenFileDescriptor = socketAccept();
                
            //     if( (clientCount+1) == MAX_CLIENTS ) {
            //         cout << "Max clients reached. Rejected: ";
            //         // Print client addrs and port?
            //         close( listenFileDescriptor );
            //         continue;
            //     }

            //     // Client set
            //     client_t *client = (client_t *) malloc( sizeof(client_t) );
            //     client->address = client;
            //     client->sockfd = 
            // }

            close(_socket);            

            char buf[BUFFER_SZ];
            int bufsize = BUFFER_SZ;
            
            cout << "----------------CLIENT CONNECTED-----------------\n" << endl;

            while( true ) {
                memset( buf, 0, BUFFER_SZ );
                send(listenFileDescriptor, buf, bufsize, 0);

                while ( true ) {
                    //cout << "Waiting client message...";
                    cout << "Client: ";
                    do {
                        recv(listenFileDescriptor, buf, bufsize, 0);
                        if (*buf != '$') cout << buf;
                    } while( *buf != '$');

                    cout << "\nServer: ";
                    sendMessage(listenFileDescriptor, BUFFER_SZ);
                }
            }

            close(listenFileDescriptor);
            return 0;
        }

    protected:
    private:
        const char *ipAddrs;
        unsigned int port;
        sockaddr_in hint;
        int _socket;

        /* Server variables */
        typedef struct{ // client struct
            sockaddr_in address;
            int sockfd;
            int uid;
            char name[32];
        } client_t;

        pthread_mutex_t clients_mutex; // server mutex

        atomic<unsigned int> clientCount = atomic<unsigned int>(0); // static _Atomic unsigned int cli_count = 0;

        int sendMessage(int client, int max_message_size) {
            //max_message_size /= 2;
            string plain_text, frag_text;
            // Read the string
            getline(cin, plain_text);

            int toSend = (int) plain_text.length();

            while( toSend > 0) {      
                // Fragments the message
                if (toSend >= max_message_size) {
                    frag_text = plain_text.substr(0, max_message_size);
                    plain_text = plain_text.substr(max_message_size, plain_text.length());
                    // Send message
                } 
                // When the message is already small enought
                else {
                    frag_text = plain_text;
                }

                // Transform to array of char
                char message[frag_text.length()];
                strcpy( message, frag_text.c_str() );

                //Send message
                //cout << message;
                send(client, message, max_message_size, 0);

                //Subtracts the toSend variable
                toSend -= frag_text.length();   
            }

            send(client, "$", max_message_size, 0);
            return 0;
        }
        
        int createSocket(int __domain, int __type, int __protocol) {
            int sock = socket(__domain, __type, __protocol);
            if (sock == -1) {
                cout << "SOCKET EXCEPTION: Could not create!";
                return ERROR;
            }
            return sock;
        }

        sockaddr_in socketHandler() {
            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(port);
            /* inet_pton(AF_INET, ipAddrs, &hint.sin_addr); DEPRECATED */
            hint.sin_addr.s_addr = inet_addr(ipAddrs); // This accepts IPv6
            return hint;
        }

        int multipleConnections() {
            int opt = 1;
            // ERROR: CAN'T BIND TO PORT
            // if( setsockopt(_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&opt, sizeof(opt)) < 0 ) 
            if( setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
            {  
                perror("setsockopt");  
                return ERROR;
            }
            return 0;
        }

        int socketConnect() {
            int connectRes = connect(_socket, (sockaddr*)&hint, sizeof(hint));
            if (connectRes == -1) {
                cout << "CONNECT EXCEPTION: Server not reachable!";
                return ERROR;
            }
            else return 0;
        }

        int socketAccept() {
            sockaddr_in client;
            socklen_t clientSize = sizeof(sockaddr_in); // ??
            
            int listenFileDescriptor = accept(_socket, (sockaddr*)&client, &clientSize);

            if( listenFileDescriptor == ERROR ) {
                cout << "CONNECTION EXCEPTION: Client could not connect!";
                return ERROR;
            }
            
            /* GET CLIENT NAME? -> NOT USED */
            // char host[NI_MAXHOST];
            // char svc[NI_MAXSERV];
            // memset(host, 0, NI_MAXHOST);
            // memset(svc, 0, NI_MAXSERV);

            // int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

            // if( !result ) {
            //     inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            // }
            return listenFileDescriptor;
        }

        int socketBind() {
            if( bind(_socket, (sockaddr*)&hint, sizeof(hint)) == -1) {
                cout << "BINDING EXCEPTION: Can't bind to IP/Port";
                return ERROR;
            }
            else return 0;
        }

        int socketListen() {
        if( listen(_socket, SOMAXCONN) == -1 ) {
                cout << "MASTER EXCEPTION: Can't listen";
                return ERROR;
        }
        else return 0;
    }
};
