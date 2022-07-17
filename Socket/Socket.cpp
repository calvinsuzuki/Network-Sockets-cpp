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
#include <pthread.h>
#include <signal.h>

using namespace std;

#define ERROR -1

class Socket {
    public:
        Socket(unsigned int _port, const char * _ipAddrs) {
            port = _port;
            ipAddrs = _ipAddrs;
            hint = socketHandler( AF_INET );
        }
        
        int startClient() {
            //	Create client socket
            _socket = createSocket(AF_INET, SOCK_STREAM, 0);

            //	Connect to the server on the socket
            socketConnect();
            
            // Control variables
            int bufsize = 4096;
            char buf[4096];
            string str_buf;
            string userInput;
            
            cout << "----------------SERVER CONNECTED-----------------" << endl;

            //	While loop:
            while ( true ) {
                cout << "\nClient: ";
                sendMessage(_socket, 4096);
                //cout << "Waiting server message...";

                cout << "Server: ";
                do {
                    recv(_socket, buf, bufsize, 0);
                    if (*buf != '$') cout << buf;
                } while( *buf != '$');
            }

            close(_socket);
            return 0;
        }

        int startServer() {
            // Create master socket
            _socket = createSocket(AF_INET, SOCK_STREAM, 0);

            //set master socket to allow multiple connections , 
            //this is just a good habit, it will work without this 
            multipleConnections();

            socketBind();
            socketListen();

            int server = socketAccept();

            close(_socket);            

            char buf[4096];
            int bufsize = 4096;
            
            cout << endl << "----------------CLIENT CONNECTED-----------------\n" << endl;

            while( true ) {
                memset( buf, 0, 4096 );
                send(server, buf, bufsize, 0);

                while ( true ) {
                    //cout << "Waiting client message...";
                    cout << "Client: ";
                    do {
                        recv(server, buf, bufsize, 0);
                        if (*buf != '$') cout << buf;
                    } while( *buf != '$');

                    cout << "\nServer: ";
                    sendMessage(server, 4096);
                }
            }

            close(server);
            return 0;
        }

    protected:
    private:
        const char *ipAddrs;
        unsigned int port;
        sockaddr_in hint;
        int _socket;

        // /* Client structure */
        // typedef struct{
        //     struct sockaddr_in address;
        //     int sockfd;
        //     int uid;
        //     char name[32];
        // } client_t;

        // client_t *clients[MAX_CLIENTS];

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

        sockaddr_in socketHandler( int __domain ) {
            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(port);
            inet_pton(AF_INET, ipAddrs, &hint.sin_addr);
            return hint;
        }

        int multipleConnections() {
            int opt = true;
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
            socklen_t clientSize;
            char host[NI_MAXHOST];
            char svc[NI_MAXSERV];

            int server = accept(_socket, (sockaddr*)&client, &clientSize);
            if( server == ERROR ) {
                cout << "CONNECTION EXCEPTION: Client could not connect!";
                return ERROR;
            }

            memset(host, 0, NI_MAXHOST);
            memset(svc, 0, NI_MAXSERV);

            int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

            if( !result ) {
                inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            }

            return server;

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
